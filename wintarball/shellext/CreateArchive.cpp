#pragma warning(disable : 4786)

#include <list>
#include <memory>
#include <string>
#include "CreateArchive.hpp"
#include "DirectoryState.hpp"
#include "Tar.hpp"


#define int_to_oct(num, oct, octlen) \
    _snprintf((oct), (octlen), "%*lo ", (octlen) - 2, (unsigned long)(num))

/* integer to string-octal conversion, no NULL */
static void int_to_oct_nonull(char *oct, int num, size_t octlen)
{
    _snprintf(oct, octlen, "%*lo", octlen - 1, (unsigned long)num);
    oct[octlen - 1] = ' ';
}

////////////////////////////////////////////////////////////////////////////////

static void BuildFileList(
    const char* folder,
    const char* prefix,
    std::list<std::string>& files)
{
    // go to the new directory, storing the old
    DirectoryState ds__;

    // go to the new directory
    if (!SetCurrentDirectory(folder)) {
        return;
    }

    // what directory are we in now?
    char directory[MAX_PATH];
    GetCurrentDirectory(MAX_PATH, directory);
    int directory_length = strlen(directory);
    if (directory[directory_length - 1] == '\\') {
        directory[directory_length - 1] = 0;
    }

    // calculate the prefix to add to all files in the list
    // if the directory is 'C:\tar', we want to add 'tar/' to all files
    char* last_separator = strrchr(directory, '\\');
    std::string in_prefix = prefix;
    if (last_separator) {
        in_prefix += last_separator + 1;
        in_prefix += "/";
    } else {
        in_prefix += directory;
    }

    files.push_back(in_prefix);

    WIN32_FIND_DATA ffd;
    HANDLE find = FindFirstFile("*", &ffd);
    if (find != INVALID_HANDLE_VALUE) {

        do {

            if (ffd.dwFileAttributes == FILE_ATTRIBUTE_DIRECTORY) {
                if (strcmp(ffd.cFileName, ".") != 0 &&
                    strcmp(ffd.cFileName, "..") != 0) {

                    BuildFileList(ffd.cFileName, in_prefix.c_str(), files);
                }
            } else {
                // add ffd.cFileName
                files.push_back(in_prefix + ffd.cFileName);
            }

        } while (FindNextFile(find, &ffd));

        FindClose(find);
    }
}

////////////////////////////////////////////////////////////////////////////////

static bool fit_name(tar_posix_header& h, const char* pathname)
{
    std::string name = pathname;
    std::string prefix;

    // name must only have slashes, not backslashes
    while (name.length() > 99) {
        std::string::size_type slash_index = name.find(-1);

        // if there aren't any slashes, stop
        if (slash_index == std::string::npos) {
            return false;
        }

        prefix += name.substr(0, slash_index + 1);
        name = name.substr(slash_index + 1);
    }

    // if the prefix is too long, stop
    if (prefix.length() > 154) {
        return false;
    }

    strcpy(h.name,   name.c_str());
    strcpy(h.prefix, prefix.c_str());
    return true;
}

////////////////////////////////////////////////////////////////////////////////

inline bool is_directory(std::string& path) {
    return (path[path.length() - 1] == '/');
}

////////////////////////////////////////////////////////////////////////////////

bool CreateArchive(
    IUINotification* notify,
    const char* folder,
    const char* extension,
    OutputFactory output_factory)
{
    notify->SetMessage("Building list of files...");

    std::list<std::string> files;
    BuildFileList(folder, "", files);

    // we need to be in the parent of the folder we're archiving
    DirectoryState ds__;
    SetCurrentDirectory(folder);
    SetCurrentDirectory("..");

    // figure out our archive file name
    std::string archive_name = folder;
    archive_name += extension;

    // calculate the message prefix
    std::string prefix = "Archiving '" + basename(archive_name.c_str()) + "': ";

    // check to see if the archive already exists
    if (file_exists(archive_name.c_str())) {
        std::string message =
            "Archive '" + archive_name + "' already exists.  Overwrite?";
        UIResult r = notify->YesNoCancel(message.c_str(), UI_WARNING);
        if (r == UI_NO) {
            return true;
        } else if (r == UI_CANCEL) {
            return false;
        }
    }

    // open output file
    std::auto_ptr<IOutputFile> out(output_factory(archive_name.c_str()));
    if (!out.get()) {
        std::string message = "Can't open archive '";
        message += archive_name;
        message += "' for writing.  Continue?";
        UIResult r = notify->YesNo(message.c_str(), UI_ERROR);
        return (r == UI_YES);
    }

    // list of unprocessed files
    std::list<std::string> unprocessed_files;

    tar_block b;

    // add every file to the archive
    std::list<std::string>::iterator i = files.begin();
    bool cancelled = false;
    for (; cancelled == false && i != files.end(); ++i) {

        bool is_dir = is_directory(*i);
        int size = (is_dir ? 0 : file_size(i->c_str()));

        // open the input file
        FILE* file;
        int mode;
        if (is_dir) {
            file = NULL;
            mode = 0755;
        } else {
            file = fopen(i->c_str(), "rb");
            if (!file) {
                unprocessed_files.push_back(*i);
                continue;
            }
            mode = 0644;
        }

        memset(&b, 0, sizeof(b));
        tar_posix_header& h = b.header;

        // fill the header

        if (!fit_name(h, i->c_str())) {
            unprocessed_files.push_back(*i);
            continue;
        }

        notify->SetMessage((prefix + *i).c_str());

        int_to_oct_nonull(h.mode,  mode, 8);
        int_to_oct_nonull(h.uid,   0,    8);
        int_to_oct_nonull(h.gid,   0,    8);
        int_to_oct_nonull(h.size,  size, 12);
        int_to_oct_nonull(h.mtime, 0,    12);
        memset(h.chksum, ' ', 8);  // prefill checksum
        h.typeflag = (is_dir ? '5' : '0');
        strcpy(h.linkname, "");
        strcpy(h.magic, "ustar");
        memset(h.version, '0', 2);
        strcpy(h.uname, "user");
        strcpy(h.gname, "group");

        // calculate checksum
        unsigned chksum = 0;
        for (int j = 0; j < TAR_BLOCK_SIZE; j++) {
            chksum += b.buffer[j];
        }
        int_to_oct_nonull(h.chksum, chksum, 8);
            
        out->Write(&b, TAR_BLOCK_SIZE);

        if (!is_dir) {

            // write the file contents
            const int num_blocks = (size + TAR_BLOCK_SIZE - 1) / TAR_BLOCK_SIZE;
            int bytes_left = size;
            while (bytes_left > 0) {
                int to_write = min(bytes_left, TAR_BLOCK_SIZE);
                memset(&b, 0, TAR_BLOCK_SIZE);
                fread(&b, 1, to_write, file);
                out->Write(&b, TAR_BLOCK_SIZE);
                bytes_left -= to_write;

                // update progress bar
                notify->SetProgress((size - bytes_left) * 255 / size);

                // should we cancel?
                if (notify->ShouldCancel()) {
                    cancelled = true;
                    break;
                }
            }

            fclose(file);
        }
    }

    // write the blank block
    memset(&b, 0, sizeof(b));
    out->Write(b.buffer, 1, TAR_BLOCK_SIZE);

    // if the user cancelled, try to remove the file
    if (cancelled) {
        delete out.release();
        remove(archive_name.c_str());
    } else if (unprocessed_files.size()) {
        
        std::string message =
            "Could not add the following files\n"
            "(path too big or could not open file):\n\n";

        i = unprocessed_files.begin();
        for (; i != unprocessed_files.end(); ++i) {
            message += *i + "\n";
        }

        message += "\nContinue?";

        UIResult rv = notify->YesNo(message.c_str(), UI_WARNING);
        return (rv == UI_YES);
    }

    return true;
}

////////////////////////////////////////////////////////////////////////////////
