#include <windows.h>
#include "Configuration.hpp"


Configuration g_Configuration;


////////////////////////////////////////////////////////////////////////////////

bool
Configuration::Load()
{
    HKEY key;
    LONG result = RegOpenKeyEx(
        HKEY_CURRENT_USER,
        "Software\\aegisknight.org\\WinTarBall",
        0,
        KEY_READ,
        &key);

    if (result != ERROR_SUCCESS) {
        return false;
    }

    DWORD bz_block_size;
    DWORD bz_less_memory;
    DWORD gz_compression_level;
    DWORD size;
    RegQueryValueEx(key, "bzip2_block_size",       NULL, NULL, (BYTE*)&bz_block_size,        &(size = 4));
    RegQueryValueEx(key, "bzip2_less_memory",      NULL, NULL, (BYTE*)&bz_less_memory,       &(size = 4));
    RegQueryValueEx(key, "gzip_compression_level", NULL, NULL, (BYTE*)&gz_compression_level, &(size = 4));

    this->bzip2_block_size       = int(bz_block_size);
    this->bzip2_less_memory      = (bz_less_memory != 0);
    this->gzip_compression_level = int(gz_compression_level);

    RegCloseKey(key);
    return true;
}

////////////////////////////////////////////////////////////////////////////////

bool
Configuration::Save()
{
    HKEY key;
    DWORD disposition;
    LONG result = RegCreateKeyEx(
        HKEY_CURRENT_USER,
        "Software\\aegisknight.org\\WinTarBall",
        0,
        "",
        REG_OPTION_NON_VOLATILE,
        KEY_ALL_ACCESS,
        NULL,
        &key,
        &disposition);

    if (result != ERROR_SUCCESS) {
        return false;
    }

    DWORD bz_block_size        = bzip2_block_size;
    DWORD bz_less_memory       = bzip2_less_memory;
    DWORD gz_compression_level = gzip_compression_level;
    RegSetValueEx(key, "bzip2_block_size",       0, REG_DWORD, (BYTE*)&bz_block_size,        sizeof(DWORD));
    RegSetValueEx(key, "bzip2_less_memory",      0, REG_DWORD, (BYTE*)&bz_less_memory,       sizeof(DWORD));
    RegSetValueEx(key, "gzip_compression_level", 0, REG_DWORD, (BYTE*)&gz_compression_level, sizeof(DWORD));

    RegCloseKey(key);
    return true;
}

////////////////////////////////////////////////////////////////////////////////
