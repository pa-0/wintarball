#include <string.h>
#include "ExtensionCompare.hpp"


////////////////////////////////////////////////////////////////////////////////

bool ExtensionCompare(const char* str, const char* ext)
{
    int str_len = strlen(str);
    int ext_len = strlen(ext);
    return (str_len >= ext_len && strcmp(str + str_len - ext_len, ext) == 0);
}

////////////////////////////////////////////////////////////////////////////////
