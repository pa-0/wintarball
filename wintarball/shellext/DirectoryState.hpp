#ifndef DIRECTORY_STATE_HPP
#define DIRECTORY_STATE_HPP


#include <windows.h>


struct DirectoryState {
    char directory[MAX_PATH];

    DirectoryState() {
        GetCurrentDirectory(MAX_PATH, directory);
    }

    ~DirectoryState() {
        SetCurrentDirectory(directory);
    }
};


#endif
