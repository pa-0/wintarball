#include <windows.h>
#include <shellapi.h>
#include <memory>
#include <stdio.h>
#include "zlib.h"
#include "bzlib.h"
#include "ShellExtension.hpp"  // includes <windows.h>
#include "../common/Configuration.hpp"
#include "resource.h"


const char* ShellExtension::s_verbs[ShellExtension::NO_COMMAND] = {
    "compress_bzball",
    "compress_gzball",
    "compress_bzfile",
    "compress_gzfile",
    "decompress_bzball",
    "decompress_gzball",
    "decompress_bzfile",
    "decompress_gzfile",
};


////////////////////////////////////////////////////////////////////////////////

ShellExtension::ShellExtension()
{
    g_Configuration.Load();

    m_target_type = UNKNOWN;
    for (int i = 0; i < MAX_COMMANDS; i++) {
        m_available_commands[i] = NO_COMMAND;
    }
    m_menu_bitmap = LoadBitmap(_Module.m_hInst, MAKEINTRESOURCE(IDB_MENU_ICON));
}

////////////////////////////////////////////////////////////////////////////////

ShellExtension::~ShellExtension()
{
    DeleteObject(m_menu_bitmap);
}

////////////////////////////////////////////////////////////////////////////////

STDMETHODIMP
ShellExtension::Initialize(
    LPCITEMIDLIST /*folder*/,
    LPDATAOBJECT object,
    HKEY /*key*/)
{
    if (!object) {
        return E_FAIL;
    }

    // if a data object was passed in
    STGMEDIUM medium;
    FORMATETC fe = { CF_HDROP, NULL, DVASPECT_CONTENT, -1, TYMED_HGLOBAL };

    // retrieve information from it
    if (FAILED(object->GetData(&fe, &medium))) {
        return E_FAIL;
    }

    char path[MAX_PATH] = "";

    // get file names
    m_paths.clear();
    HDROP drop = (HDROP)medium.hGlobal;
    UINT count = DragQueryFile(drop, 0xFFFFFFFF, NULL, 0);
    for (unsigned i = 0; i < count; i++) {
        DragQueryFile(drop, 0, path, MAX_PATH);
        m_paths.push_back(path);
    }

    ReleaseStgMedium(&medium);
    return S_OK;
}

////////////////////////////////////////////////////////////////////////////////

STDMETHODIMP
ShellExtension::QueryContextMenu(
    HMENU menu,
    UINT index,
    UINT first,
    UINT last,
    UINT flags)
{
    // if the path is empty, no commands
    if (m_paths.empty()) {  // if (strlen(m_path) == 0) {
        return S_OK;
    }

    DetermineTargetType();

    char* commands[MAX_COMMANDS];
    char** p = commands;
    
    // initialize commands to empty
    for (int i = 0; i < MAX_COMMANDS; i++) {
        m_available_commands[i] = NO_COMMAND;
    }
    Command* q = m_available_commands;

    switch (m_target_type) {
        case FOLDER: {
            *p++ = "Compress Folder to .tar.bz2";
            *q++ = COMPRESS_BZBALL;
            *p++ = "Compress Folder to .tar.gz";
            *q++ = COMPRESS_GZBALL;
            break;
        }

        case BZ_BALL: {
            *p++ = "Decompress Folder";
            *q++ = DECOMPRESS_BZBALL;
            break;
        }

        case GZ_BALL: {
            *p++ = "Decompress Folder";
            *q++ = DECOMPRESS_GZBALL;
            break;
        }

        case BZ_FILE: {
            *p++ = "Decompress";
            *q++ = DECOMPRESS_BZFILE;
            break;
        }

        case GZ_FILE: {
            *p++ = "Decompress";
            *q++ = DECOMPRESS_GZFILE;
            break;
        }

        case NORMAL_FILE: {
            *p++ = "Compress File to .bz2";
            *q++ = COMPRESS_BZFILE;
            *p++ = "Compress File to .gz";
            *q++ = COMPRESS_GZFILE;
            break;
        }

        default: {
            return S_OK;
        }
    }

    // set common values in the menu item
    MENUITEMINFO mii;
    memset(&mii, 0, sizeof(mii));
    mii.cbSize = sizeof(mii);
    mii.fMask = MIIM_ID | MIIM_TYPE | MIIM_STATE;
    mii.fType = MFT_STRING;
    mii.fState = MFS_ENABLED;  // (def ? MFS_DEFAULT : MFS_ENABLED);

    // add each command to the menu
    UINT id = first;
    for (char** c = commands; c != p; c++) {
        mii.wID = id;
        mii.dwTypeData = *c;

        InsertMenuItem(menu, index, TRUE, &mii);
        SetMenuItemBitmaps(menu, index, MF_BYPOSITION, m_menu_bitmap, NULL);

        id++;
        index++;
    }

    return MAKE_HRESULT(SEVERITY_SUCCESS, 0, p - commands);
}

////////////////////////////////////////////////////////////////////////////////

STDMETHODIMP
ShellExtension::GetCommandString(
    UINT command,
    UINT flags,
    UINT* /*reserved*/,
    LPSTR name,
    UINT maxchars)
{
    if (command >= MAX_COMMANDS ||
        m_available_commands[command] == NO_COMMAND) {

        return E_FAIL;
    }

    int m_command = 0;

    // corresponds to m_command enum values
    static const char* help_strings[NO_COMMAND] = {
        "Compresses directory into a .tar.bz2 file.",
        "Compresses directory into a .tar.gz file.",
        "Compresses file into a .bz2 file.",
        "Compresses file into a .gz file.",
        "Decompresses .tar.bz2 file into a directory.",
        "Decompresses .tar.gz file into a directory.",
        "Decompresses bzip2-compressed file.",
        "Decompresses gzip-compressed file.",
    };

    int theCommand = m_available_commands[command];
    
    switch (flags) {
        case GCS_HELPTEXTA: {                      
            strncpy(name, help_strings[theCommand], maxchars);
            return S_OK;
        }

        case GCS_HELPTEXTW: {
            mbstowcs((wchar_t*)name, help_strings[theCommand], maxchars);
            return S_OK;
        }

        case GCS_VALIDATEA: {
            // assume if we get this far, we're valid
            return S_OK;
        }

        case GCS_VALIDATEW: {
            // assume if we get this far, we're valid
            return S_OK;
        }

        case GCS_VERBA: {
            strncpy(name, s_verbs[theCommand], maxchars);
            return S_OK;
        }

        case GCS_VERBW: {
            mbstowcs((wchar_t*)name, s_verbs[theCommand], maxchars);
            return S_OK;
        }

        default: {
            return E_FAIL;
        }
    }
}

////////////////////////////////////////////////////////////////////////////////

STDMETHODIMP
ShellExtension::InvokeCommand(LPCMINVOKECOMMANDINFO ici)
{
    // XXXX for now, don't support calling by verb
    if (!ici || HIWORD(ici->lpVerb)) {
        return E_FAIL;
    }

    ThreadParameters* p = new ThreadParameters;
    p->command = m_available_commands[LOWORD(ici->lpVerb)];
    p->paths   = m_paths;

    DWORD thread_id;
    HANDLE thread = CreateThread(NULL, 0, WorkerThread, p, 0, &thread_id);
    if (thread != INVALID_HANDLE_VALUE) {
        CloseHandle(thread);
    }

    return S_OK;
}

////////////////////////////////////////////////////////////////////////////////

void
ShellExtension::DetermineTargetType()
{
    if (!m_paths.empty()) {

        // all targets must be of the same type, or we don't know what to do
        // verify this

        std::list<std::string>::iterator i = m_paths.begin();
        m_target_type = GetTargetType(i->c_str());

        ++i;
        while (i != m_paths.end()) {

            if (m_target_type != GetTargetType(i->c_str())) {
                m_target_type = UNKNOWN;
                return;
            }

            ++i;
        }

    } else {
        m_target_type = UNKNOWN;
    }
}

////////////////////////////////////////////////////////////////////////////////

static bool ExtensionCompare(const char* str, const char* ext) {
    int str_len = strlen(str);
    int ext_len = strlen(ext);
    return (str_len >= ext_len && strcmp(str + str_len - ext_len, ext) == 0);
}

ShellExtension::TargetType
ShellExtension::GetTargetType(const char* path)
{
    if (GetFileAttributes(path) & FILE_ATTRIBUTE_DIRECTORY) {

        return FOLDER;

    } else if (ExtensionCompare(path, ".tar.bz2") ||
               ExtensionCompare(path, ".tbz")) {

        return BZ_BALL;

    } else if (ExtensionCompare(path, ".tar.gz") ||
               ExtensionCompare(path, ".tgz")) {
        
        return GZ_BALL;

    } else if (ExtensionCompare(path, ".bz2")) {

        return BZ_FILE;

    } else if (ExtensionCompare(path, ".gz")) {

        return GZ_FILE;

    } else {

        return NORMAL_FILE;

    }
}

////////////////////////////////////////////////////////////////////////////////

bool exists(const char* filename) {
    FILE* f = fopen(filename, "rb");
    if (f) {
        fclose(f);
        return true;
    } else {
        return false;
    }
}

////////////////////////////////////////////////////////////////////////////////

struct ANSIInput {
    typedef FILE* InputFile;

    static InputFile OpenInputFile(const char* filename) {
        return fopen(filename, "rb");
    }

    static unsigned Read(InputFile file, void* buffer, unsigned size) {
        return fread(buffer, 1, size, file);
    }

    static void CloseInputFile(InputFile file) {
        fclose(file);
    }
};

struct ANSIOutput {
    typedef FILE* OutputFile;

    static OutputFile OpenOutputFile(const char* filename) {
        return fopen(filename, "wb");
    }

    static unsigned Write(OutputFile file, void* buffer, unsigned size) {
        return fwrite(buffer, 1, size, file);
    }

    static void CloseOutputFile(OutputFile file) {
        fclose(file);
    }
};

struct Bzip2Transform : ANSIInput {
    static void GetOutputFilename(std::string& output_name, const char* source) {
        output_name = source;
        output_name += ".bz2";
    }


    typedef BZFILE* OutputFile;

    static OutputFile OpenOutputFile(const char* filename) {
        char mode[5] = { 'w', 'b', 0, 0, 0 };
        mode[2] = '0' + g_Configuration.bzip2_block_size;
        if (g_Configuration.bzip2_block_size) {
            mode[3] = 's';
        }

        return BZ2_bzopen(filename, mode);
    }

    static unsigned Write(OutputFile file, void* buffer, unsigned size) {
        return BZ2_bzwrite(file, buffer, size);
    }

    static void CloseOutputFile(OutputFile file) {
        BZ2_bzclose(file);
    }
};

struct Unbzip2Transform : ANSIOutput {
    static void GetOutputFilename(std::string& output_name, const char* source) {
        if (ExtensionCompare(source, ".bz2")) {
            output_name = source;
            output_name.resize(output_name.length() - 4);
        } else {
            output_name = source;
            output_name += ".unbz2";
        }
    }

    typedef BZFILE* InputFile;

    static InputFile OpenInputFile(const char* filename) {
        return BZ2_bzopen(filename, "rb");
    }

    static unsigned Read(InputFile file, void* buffer, unsigned size) {
        return BZ2_bzread(file, buffer, size);
    }

    static void CloseInputFile(InputFile file) {
        BZ2_bzclose(file);
    }
};

struct GzipTransform : ANSIInput {
    static void GetOutputFilename(std::string& output_name, const char* source) {
        output_name = source;
        output_name += ".gz";
    }


    typedef gzFile OutputFile;

    static OutputFile OpenOutputFile(const char* filename) {
        char mode[4] = { 'w', 'b', 0, 0 };
        mode[2] = '0' + g_Configuration.gzip_compression_level;
        return gzopen(filename, mode);
    }

    static unsigned Write(OutputFile file, void* buffer, unsigned size) {
        return gzwrite(file, buffer, size);
    }

    static void CloseOutputFile(OutputFile file) {
        gzclose(file);
    }
};

struct UngzipTransform : ANSIOutput {
    static void GetOutputFilename(std::string& output_name, const char* source) {
        if (ExtensionCompare(source, ".gz")) {
            output_name = source;
            output_name.resize(output_name.length() - 3);
        } else {
            output_name = source;
            output_name += ".ungz";
        }
    }

    typedef gzFile InputFile;

    static InputFile OpenInputFile(const char* filename) {
        return gzopen(filename, "rb");
    }

    static unsigned Read(InputFile file, void* buffer, unsigned size) {
        return gzread(file, buffer, size);
    }

    static void CloseInputFile(InputFile file) {
        gzclose(file);
    }
};

////////////////////////////////////////////////////////////////////////////////

// we need the second parameter because VC++ sucks :(
template<typename Transform>
void TransformFile(const char* filename, Transform = Transform())
{
    static const int BUFFER_SIZE = 4096;

    std::string output_name;
    Transform::GetOutputFilename(output_name, filename);
    if (exists(output_name.c_str())) {
        if (MessageBox(NULL, "File already exists, overwrite?", "WinTarBall", MB_YESNO | MB_ICONQUESTION) == IDNO) {
            return;
        }
    }

    Transform::InputFile in = Transform::OpenInputFile(filename);
    if (!in) {
        MessageBox(NULL, "Can't open input file.", "WinTarBall", MB_OK | MB_ICONERROR);
        return;
    }

    Transform::OutputFile out = Transform::OpenOutputFile(output_name.c_str());
    if (!out) {
        Transform::CloseInputFile(in);
        MessageBox(NULL, "Can't open output file.", "WinTarBall", MB_OK | MB_ICONERROR);
        return;
    }

    char buffer[BUFFER_SIZE];
    bool done = false;
    while (!done) {
    
        int read = Transform::Read(in, buffer, BUFFER_SIZE);
        int sent = Transform::Write(out, buffer, read);

        if (sent != read) {
            MessageBox(NULL, "Can't write to file.", "WinTarBall", MB_OK | MB_ICONERROR);
            done = true;
        } else {
            done = (read != BUFFER_SIZE);
        }
    }

    Transform::CloseOutputFile(out);
    Transform::CloseInputFile(in);
}

////////////////////////////////////////////////////////////////////////////////

DWORD WINAPI
ShellExtension::WorkerThread(LPVOID parameter)
{
    ThreadParameters* p = (ThreadParameters*)parameter;

    std::list<std::string>::iterator i = p->paths.begin();
    while (i != p->paths.end()) {

        switch (p->command) {
            case COMPRESS_BZBALL: {
                MessageBox(NULL, "COMPRESS_BZBALL", "WinTarBall", MB_OK);
                break;
            }

            case COMPRESS_GZBALL: {
                MessageBox(NULL, "COMPRESS_GZBALL", "WinTarBall", MB_OK);
                break;
            }

            case COMPRESS_BZFILE: {
                TransformFile<Bzip2Transform>(i->c_str());
                break;
            }

            case COMPRESS_GZFILE: {
                TransformFile<GzipTransform>(i->c_str());
                break;
            }

            case DECOMPRESS_BZBALL: {
                MessageBox(NULL, "DECOMPRESS_BZBALL", "WinTarBall", MB_OK);
                break;
            }

            case DECOMPRESS_GZBALL: {
                MessageBox(NULL, "DECOMPRESS_GZBALL", "WinTarBall", MB_OK);
                break;
            }

            case DECOMPRESS_BZFILE: {
                TransformFile<Unbzip2Transform>(i->c_str());
                break;
            }

            case DECOMPRESS_GZFILE: {
                TransformFile<UngzipTransform>(i->c_str());
                break;
            }
        }

        ++i;
    }
    
    delete p;
    return 0;
}

////////////////////////////////////////////////////////////////////////////////
