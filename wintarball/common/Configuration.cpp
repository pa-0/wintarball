#include <windows.h>
#include "Configuration.hpp"


static const char* CONFIG_PATH = "Software\\aegisknight.org\\WinTarBall";


Configuration g_Configuration;


////////////////////////////////////////////////////////////////////////////////

inline DWORD ReadValue(HKEY key, const char* name)
{
  DWORD value = 0;
  DWORD size = 4;
    RegQueryValueEx(key, name, NULL, NULL, (BYTE*)&value, &size);
    return value;
}

bool
Configuration::Load()
{
    HKEY key;
    LONG result = RegOpenKeyEx(
        HKEY_CURRENT_USER,
        CONFIG_PATH,
        0,
        KEY_READ,
        &key);

    if (result != ERROR_SUCCESS) {
        return false;
    }

    DWORD bz_block_size        = ReadValue(key, "bzip2_block_size");      
    DWORD bz_less_memory       = ReadValue(key, "bzip2_less_memory");     
    DWORD gz_compression_level = ReadValue(key, "gzip_compression_level");

    this->bzip2_block_size       = int(bz_block_size);
    this->bzip2_less_memory      = (bz_less_memory != 0);
    this->gzip_compression_level = int(gz_compression_level);

    RegCloseKey(key);
    return true;
}

////////////////////////////////////////////////////////////////////////////////

inline void WriteValue(HKEY key, const char* name, DWORD value)
{
    RegSetValueEx(key, name, 0, REG_DWORD, (BYTE*)&value, sizeof(DWORD));
}

bool
Configuration::Save()
{
    HKEY key;
    DWORD disposition;
    LONG result = RegCreateKeyEx(
        HKEY_CURRENT_USER,
        CONFIG_PATH,
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

    WriteValue(key, "bzip2_block_size",       bzip2_block_size);
    WriteValue(key, "bzip2_less_memory",      bzip2_less_memory);
    WriteValue(key, "gzip_compression_level", gzip_compression_level);

    RegCloseKey(key);
    return true;
}

////////////////////////////////////////////////////////////////////////////////
