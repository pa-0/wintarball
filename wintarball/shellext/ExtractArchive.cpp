#pragma warning(disable : 4786)

#include <list>
#include <memory>
#include <string>
#include "ExtractArchive.hpp"
#include "DirectoryState.hpp"
#include "Tar.hpp"


////////////////////////////////////////////////////////////////////////////////

static FILE* OpenOutputFile(const char* filename)
{
    DirectoryState* ds__ = new DirectoryState;

    char* buffer = new char[strlen(filename) + 1];
    strcpy(buffer, filename);

    char* f = buffer;
    while (*f) {
        char* g = f;
        while (*g != 0 && *g != '/' && *g != '\\') {
            g++;
        }

        if (*g == 0) {
            break;
        } else {
            *g = 0;

            CreateDirectory(f, NULL);
            if (!SetCurrentDirectory(f)) {
                delete[] buffer;
                return NULL;
            }

            f = g + 1;
        }
    }

    delete[] buffer;
    delete ds__;
    return fopen(filename, "wb");
}

////////////////////////////////////////////////////////////////////////////////

static bool all_zeroes(const void* b, int size)
{
    const char* c = (const char*)b;
    while (size--) {
        if (*c++) {
            return false;
        }
    }
    return true;
}

////////////////////////////////////////////////////////////////////////////////

bool ExtractArchive(
    IUINotification* notify,
    const char* archive,
    InputFactory input_factory)
{
    DirectoryState ds__;

    // set the current directory to the one the archive is in
    // otherwise, we might end up in C:\ or somewhere equally nasty
    char* archive_buffer = new char[strlen(archive) + 1];
    strcpy(archive_buffer, archive);

    char* last_slash     = strrchr(archive_buffer, '/');
    char* last_backslash = strrchr(archive_buffer, '\\');
    char* last = NULL;
    if (last_slash && last_backslash) {
        last = max(last_slash, last_backslash);
    } else if (last_slash) {
        last = last_slash;
    } else if (last_backslash) {
        last = last_backslash;
    }
    if (last) {
        *last = 0;
    }
    SetCurrentDirectory(archive_buffer);
    delete[] archive_buffer;

    // open archive
    std::auto_ptr<IInputFile> file(input_factory(archive));
    if (!file.get()) {
        std::string message = "Can't open archive '";
        message += archive;
        message += "'.  Continue?";
        UIResult r = notify->YesNo(message.c_str(), UI_ERROR);
        return (r == UI_YES);
    }

    std::string message_prefix = "Extracting '" + basename(archive) + "': ";

    // files that weren't written successfully
    std::list<std::string> unwritten_files;

#define READ_BLOCK(b)                                  \
    {                                                  \
        int read = file->Read((b), TAR_BLOCK_SIZE);    \
        if (read != TAR_BLOCK_SIZE) {                  \
            done = true;                               \
            break;                                     \
        }                                              \
    }

    tar_block b;
    bool done = false;
    while (!done) {
        READ_BLOCK(&b);
        if (all_zeroes(&b, sizeof(b))) {
            done = true;
            break;
        }

        std::string filename;

        if (strcmp(b.header.name, "././@LongLink") == 0) {
            // GNU long filename
            int size = octal_to_int(b.header.size, 12);
            int num_blocks = (size + 511) / 512;

            while (num_blocks--) {
                char name[TAR_BLOCK_SIZE + 1];
                name[TAR_BLOCK_SIZE] = 0;
                READ_BLOCK(name);

                filename += name;
            }

            // now that we've read the filename, read the real information block
            READ_BLOCK(&b);

        } else {
            // POSIX filename
            // build filename from prefix + name
            filename = b.header.prefix;
            if (filename.length()) {
                filename += "/";
            }
            filename += b.header.name;
        }

        notify->SetMessage((message_prefix + filename).c_str());

        // now try to open output file
        if (b.header.typeflag == '0' ||
            b.header.typeflag == '\0') {

            FILE* ofile = OpenOutputFile(filename.c_str());
            if (!ofile) {
                unwritten_files.push_back(filename);
            }

            const int size = octal_to_int(b.header.size, 12);
            const int num_blocks = (size + 511) / 512;
            int bytes_left = size;
            int nb = num_blocks;
            while (nb--) {
                char data[TAR_BLOCK_SIZE];
                READ_BLOCK(data);
                
                int to_write = min(bytes_left, TAR_BLOCK_SIZE);
                if (ofile) {
                    fwrite(data, 1, to_write, ofile);
                }
                bytes_left -= to_write;

                // update the progress bar
                notify->SetProgress((size - bytes_left) * 255 / size);

                // check to see if we should cancel
                if (notify->ShouldCancel()) {
                    if (ofile) {
                        fclose(ofile);
                        remove(filename.c_str());
                        done = true;
                        break;
                    }
                }
            }

            if (ofile) {
                fclose(ofile);
            }
        }
    }

    // if we couldn't write some files, show a warning dialog
    if (unwritten_files.size()) {
        std::string message = "Could not create the following files:\n";

        std::list<std::string>::iterator i = unwritten_files.begin();
        for (; i != unwritten_files.end(); ++i) {
            message += (*i + "\n");
        }

        message += "\nContinue?";
        UIResult r = notify->YesNo(message.c_str(), UI_WARNING);
        return (r == UI_YES);
    }

    return true;
}

////////////////////////////////////////////////////////////////////////////////
