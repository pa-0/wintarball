#ifndef CREATE_ARCHIVE_HPP
#define CREATE_ARCHIVE_HPP


#include "Files.hpp"
#include "IUINotification.hpp"


extern bool CreateArchive(
    IUINotification* notify,
    const char* folder,
    const char* extension,
    OutputFactory output_factory);


#endif
