#include <windows.h>
#include <shellapi.h>
#include <memory>
#include <stdio.h>
#include "ShellExtension.hpp"  // includes <windows.h>
#include "Files.hpp"
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

static bool exists(const char* filename) {
    FILE* f = fopen(filename, "rb");
    if (f) {
        fclose(f);
        return true;
    } else {
        return false;
    }
}

////////////////////////////////////////////////////////////////////////////////

typedef void (*FilenameTransform)(std::string& output_name, const char* source);

static void Bzip2FilenameTransform(
    std::string& output_name,
    const char* source)
{
    output_name = source;
    output_name += ".bz2";
}

static void Unbzip2FilenameTransform(
    std::string& output_name,
    const char* source)
{
    if (ExtensionCompare(source, ".bz2")) {
        output_name = source;
        output_name.resize(output_name.length() - 4);
    } else {
        output_name = source;
        output_name += ".unbz2";
    }
}

static void GzipFilenameTransform(
    std::string& output_name,
    const char* source)
{
    output_name = source;
    output_name += ".gz";
}

static void UngzipFilenameTransform(
    std::string& output_name,
    const char* source)
{
    if (ExtensionCompare(source, ".gz")) {
        output_name = source;
        output_name.resize(output_name.length() - 3);
    } else {
        output_name = source;
        output_name += ".ungz";
    }
}

////////////////////////////////////////////////////////////////////////////////

void DoTransformFile(
    const char* filename,
    FilenameTransform fn_transform,
    InputFactory input_factory,
    OutputFactory output_factory)
{
    static const int BUFFER_SIZE = 4096;

    std::string output_name;
    fn_transform(output_name, filename);
    if (exists(output_name.c_str())) {
        if (MessageBox(NULL, "File already exists, overwrite?", "WinTarBall", MB_YESNO | MB_ICONQUESTION) == IDNO) {
            return;
        }
    }

    std::auto_ptr<IInputFile> in(input_factory(filename));
    if (!in.get()) {
        MessageBox(NULL, "Can't open input file.", "WinTarBall", MB_OK | MB_ICONERROR);
        return;
    }

    std::auto_ptr<IOutputFile> out(output_factory(output_name.c_str()));
    if (!out.get()) {
        MessageBox(NULL, "Can't open output file.", "WinTarBall", MB_OK | MB_ICONERROR);
        return;
    }

    char buffer[BUFFER_SIZE];
    bool done = false;
    while (!done) {
    
        int read = in->Read(buffer, BUFFER_SIZE);
        int sent = out->Write(buffer, read);

        if (sent != read) {
            MessageBox(NULL, "Can't write to file.", "WinTarBall", MB_OK | MB_ICONERROR);
            done = true;
        } else {
            done = (read != BUFFER_SIZE);
        }
    }
}

////////////////////////////////////////////////////////////////////////////////

struct Transform {
    FilenameTransform filename_transform;
    InputFactory      input_factory;
    OutputFactory     output_factory;
};

struct Bzip2Transform : Transform {
    Bzip2Transform() {
        filename_transform = Bzip2FilenameTransform;
        input_factory      = OpenANSIInputFile;
        output_factory     = OpenBzip2OutputFile;
    }
};

struct Unbzip2Transform : Transform {
    Unbzip2Transform() {
        filename_transform = Unbzip2FilenameTransform;
        input_factory      = OpenBzip2InputFile;
        output_factory     = OpenANSIOutputFile;
    }
};

struct GzipTransform : Transform {
    GzipTransform() {
        filename_transform = GzipFilenameTransform;
        input_factory      = OpenANSIInputFile;
        output_factory     = OpenGzipOutputFile;
    }
};

struct UngzipTransform : Transform {
    UngzipTransform() {
        filename_transform = UngzipFilenameTransform;
        input_factory      = OpenGzipInputFile;
        output_factory     = OpenANSIOutputFile;
    }
};

// we need the second parameter because VC++ sucks :(
template<typename transform>
void TransformFile(const char* filename, transform t = transform())
{
    DoTransformFile(
        filename,
        t.filename_transform,
        t.input_factory,
        t.output_factory);
}

////////////////////////////////////////////////////////////////////////////////

void BuildFileList(const char* folder, std::list<std::string>& files)
{
    // go to the new directory, storing the old
    char old_directory[MAX_PATH];
    GetCurrentDirectory(MAX_PATH, old_directory);
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

    SetCurrentDirectory(old_directory);
}

void CreateArchive(const char* folder, OutputFactory output_factory)
{
    std::list<std::string> files;
    BuildFileList(folder, files);

    MessageBox(NULL, "Here, we'd be done making the archive.", "WinTarBall", MB_OK);
}

////////////////////////////////////////////////////////////////////////////////

struct posix_header
{                               /* byte offset */
    char name[100];             /*   0 */
    char mode[8];               /* 100 */
    char uid[8];                /* 108 */
    char gid[8];                /* 116 */
    char size[12];              /* 124 */
    char mtime[12];             /* 136 */
    char chksum[8];             /* 148 */
    char typeflag;              /* 156 */
    char linkname[100];         /* 157 */
    char magic[6];              /* 257 */
    char version[2];            /* 263 */
    char uname[32];             /* 265 */
    char gname[32];             /* 297 */
    char devmajor[8];           /* 329 */
    char devminor[8];           /* 337 */
    char prefix[155];           /* 345 */
                                /* 500 */
};

#define BLOCKSIZE 512

union block {
    char buffer[BLOCKSIZE];
    posix_header header;
};

int octal_to_int(char* o, int size) {
    int result = 0;
    while (--size) {
        char c = *o++;
        int value = (c >= '0' && c <= '7' ? c - '0' : 0);
        result = (result * 8) + value;
    }
    return result;
}

struct DirectoryState {
    char directory[MAX_PATH];

    DirectoryState() {
        GetCurrentDirectory(MAX_PATH, directory);
    }

    ~DirectoryState() {
        SetCurrentDirectory(directory);
    }
};

FILE* OpenOutputFile(const char* filename) {
    {
        DirectoryState ds__;

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
    }

    return fopen(filename, "wb");
}

void ExtractArchive(const char* archive, InputFactory input_factory)
{
    DirectoryState ds__;

    char* archive_buffer = new char[strlen(archive) + 1];
    strcpy(archive_buffer, archive);

    char* last_slash     = strrchr(archive_buffer, '/');
    char* last_backslash = strrchr(archive_buffer, '\\');
    char* last = NULL;
    if (last_slash && last_backslash) {
        last = (last_slash > last_backslash ? last_slash : last_backslash);
    } else if (last_slash) {
        last = last_slash;
    } else if (last_backslash) {
        last = last_backslash;
    }
    if (last) {
        *last = 0;
        SetCurrentDirectory(archive_buffer);
    }
    delete[] archive_buffer;

    std::auto_ptr<IInputFile> file(input_factory(archive));
    if (!file.get()) {
        MessageBox(NULL, "Error", "WinTarBall", MB_OK);
        return;
    }

//    __asm int 3

    block b;
    while (true) {
        int read = file->Read(&b, BLOCKSIZE);
        if (read != BLOCKSIZE) {
            break;
        }
        bool all_zeroes = true;
        for (int i = 0; i < BLOCKSIZE; i++) {
            if (b.buffer[i]) {
                all_zeroes = false;
                break;
            }
        }
        if (all_zeroes) {
            break;
        }


        std::string filename;

        if (strcmp(b.header.name, "././@LongLink") == 0) {
            // GNU long filename
            int size = octal_to_int(b.header.size, 12);
            int num_blocks = (size + 511) / 512;

            while (num_blocks--) {
                char name[BLOCKSIZE + 1];
                name[BLOCKSIZE] = 0;
                int read = file->Read(name, BLOCKSIZE);
                if (read != BLOCKSIZE) {
                    break;
                }

                filename += name;
            }

            // now that we've read the filename, read the real thing
            int read = file->Read(&b, BLOCKSIZE);
            if (read != BLOCKSIZE) {
                break;
            }

        } else {
            // build filename from prefix + name
            filename = b.header.prefix;
            if (filename.length()) {
                filename += "/";
            }
            filename += b.header.name;
        }

        // now try to open output file
        if (b.header.typeflag == '0' ||
            b.header.typeflag == '\0') {

            FILE* ofile = OpenOutputFile(filename.c_str());
            if (!ofile) {
                MessageBox(NULL, filename.c_str(), "Couldn't open file", MB_OK);
            }

            int size = octal_to_int(b.header.size, 12);
            int num_blocks = (size + 511) / 512;
            while (num_blocks--) {
                char data[BLOCKSIZE];
                int read = file->Read(data, BLOCKSIZE);
                if (read != BLOCKSIZE) {
                    return;
                }

                if (ofile) {
                    if (size >= BLOCKSIZE) {
                        fwrite(data, 1, BLOCKSIZE, ofile);
                    } else {
                        fwrite(data, 1, size, ofile);
                    }
                }

                size -= BLOCKSIZE;
            }

            if (ofile) {
                fclose(ofile);
            }
        }

//        char str[1024];
//        sprintf(str, "%s\n%c\n%s", b.header.name, b.header.typeflag, b.header.linkname);
//        MessageBox(NULL, filename.c_str(), "WinTarBall", MB_OK);
    }

    MessageBox(NULL, "Here, we'd be done extracting the archive.", "WinTarBall", MB_OK);
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
                CreateArchive(i->c_str(), OpenBzip2OutputFile);
                break;
            }

            case COMPRESS_GZBALL: {
                CreateArchive(i->c_str(), OpenGzipOutputFile);
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
                ExtractArchive(i->c_str(), OpenBzip2InputFile);
                break;
            }

            case DECOMPRESS_GZBALL: {
                ExtractArchive(i->c_str(), OpenGzipInputFile);
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
