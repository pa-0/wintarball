#pragma warning(disable : 4786)

#include <list>
#include <memory>
#include <string>
#include "CreateArchive.hpp"
#include "DirectoryState.hpp"


////////////////////////////////////////////////////////////////////////////////

static void BuildFileList(const char* folder, std::list<std::string>& files)
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
    std::string prefix;
    if (last_separator) {
        prefix = last_separator + 1;
        prefix += "/";
    } else {
        prefix = directory;
    }

    WIN32_FIND_DATA ffd;
    HANDLE find = FindFirstFile("*", &ffd);
    if (find != INVALID_HANDLE_VALUE) {

        do {

            if (ffd.dwFileAttributes == FILE_ATTRIBUTE_DIRECTORY) {
                if (strcmp(ffd.cFileName, ".") != 0 &&
                    strcmp(ffd.cFileName, "..") != 0) {

                    BuildFileList(ffd.cFileName, files);
                }
            } else {
                // add ffd.cFileName
                files.push_back(prefix + ffd.cFileName);
            }

        } while (FindNextFile(find, &ffd));

        FindClose(find);
    }
}

////////////////////////////////////////////////////////////////////////////////

bool CreateArchive(
    IUINotification* notify,
    const char* folder,
    const char* extension,
    OutputFactory output_factory)
{
    std::list<std::string> files;
    BuildFileList(folder, files);

    // figure out our archive file name
    std::string archive_name = folder;
    archive_name += extension;

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

    std::list<std::string>::iterator i = files.begin();
    for (; i != files.end(); ++i) {
        out->Write(i->c_str(), i->length() + 1);
        out->Write("\n", 1);
    }

    return true;
}

////////////////////////////////////////////////////////////////////////////////
