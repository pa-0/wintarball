#ifndef EXTRACT_ARCHIVE_HPP
#define EXTRACT_ARCHIVE_HPP


#include "Files.hpp"
#include "IUINotification.hpp"


extern bool ExtractArchive(
    IUINotification* notify,
    const char* archive,
    InputFactory input_factory);


#endif
