#include <windows.h>
#include <shellapi.h>
#include <memory>
#include <stdio.h>
#include "ShellExtension.hpp"  // includes <windows.h>
#include "DirectoryState.hpp"
#include "ExtensionCompare.hpp"
#include "Files.hpp"
#include "IUINotification.hpp"
#include "TransformFile.hpp"
#include "../common/Configuration.hpp"
#include "resource.h"


const char* ShellExtension::s_verbs[ShellExtension::NO_COMMAND] = {
    "compress_tarball",
    "compress_bzball",
    "compress_gzball",
    "compress_bzfile",
    "compress_gzfile",
    "decompress_tarball",
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
            *p++ = "Compress Folder to .tar";
            *q++ = COMPRESS_TARBALL;
            break;
        }

        case TARBALL: {
            *p++ = "Decompress Folder";
            *q++ = DECOMPRESS_TARBALL;
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
        "Compresses directory into a .tar file.",
        "Compresses directory into a .tar.bz2 file.",
        "Compresses directory into a .tar.gz file.",
        "Compresses file into a .bz2 file.",
        "Compresses file into a .gz file.",
        "Decompresses .tar file into a directory.",
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

    WorkerThreadParameters* p = new WorkerThreadParameters;
    p->command  = m_available_commands[LOWORD(ici->lpVerb)];
    p->paths    = m_paths;

    DWORD thread_id;
    HANDLE thread = CreateThread(NULL, 0, WorkerThread, p, 0, &thread_id);
    if (thread != INVALID_HANDLE_VALUE) {
        CloseHandle(thread);
    } else {
        delete p;
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

ShellExtension::TargetType
ShellExtension::GetTargetType(const char* path)
{
    if (GetFileAttributes(path) & FILE_ATTRIBUTE_DIRECTORY) {

        return FOLDER;

    } else if (ExtensionCompare(path, ".tar")) {
    
        return TARBALL;

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

static const int BLOCKSIZE = 512;

union block {
    char buffer[BLOCKSIZE];
    posix_header header;
};

static int octal_to_int(char* o, int size) {
    int result = 0;
    while (--size) {
        char c = *o++;
        int value = (c >= '0' && c <= '7' ? c - '0' : 0);
        result = (result * 8) + value;
    }
    return result;
}

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

static void ExtractArchive(const char* archive, InputFactory input_factory)
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
    }
}

////////////////////////////////////////////////////////////////////////////////

// UN = User interface Notification
static const int UN_YESNOCANCEL = WM_APP + 0;
static const int UN_YESNO       = WM_APP + 1;
static const int UN_MESSAGE     = WM_APP + 2;
static const int UN_PROGRESS    = WM_APP + 3;
static const int UN_RESULT      = WM_APP + 4;
static const int UN_CANCEL      = WM_APP + 5;

struct UI_MESSAGE {
    DWORD calling_thread_id;
    const char* message;
    UIIcon icon;
};

class UINotification : public IUINotification
{
public:
    UINotification(DWORD thread_id) {
        m_thread_id = thread_id;
        m_should_cancel = false;

        // force creation of a message queue in this thread
        MSG msg;
        PeekMessage(&msg, NULL, WM_USER, WM_USER, PM_NOREMOVE);
    }

    virtual UIResult YesNoCancel(const char* message, UIIcon icon) {
        return (UIResult)DoPostMessage(UN_YESNOCANCEL, message, icon);
    }

    virtual UIResult YesNo(const char* message, UIIcon icon) {
        return (UIResult)DoPostMessage(UN_YESNOCANCEL, message, icon);
    }

    virtual void SetMessage(const char* message) {
        // allocate a new string and give the UI thread the responsibility of
        // deleteing it
        char* m = new char[strlen(message) + 1];
        strcpy(m, message);

        BOOL result;
        do {
            result = PostThreadMessage(
                m_thread_id,
                UN_MESSAGE,
                0,
                (LPARAM)m);
        } while (!result);
    }

    virtual void SetProgress(int progress) {
        BOOL result;
        do {
            result = PostThreadMessage(m_thread_id, UN_PROGRESS, 0, progress);
        } while (!result);
    }

    virtual bool ShouldCancel() {
        if (m_should_cancel) {
            return true;
        }

        // look at all messages in the queue for a cancel message
        MSG msg;
        while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
            if (msg.message == UN_CANCEL) {
                m_should_cancel = true;
                return true;
            }
        }

        return false;
    }

private:
    int DoPostMessage(int message, const char* str_message, UIIcon icon) {
        UI_MESSAGE m;
        m.calling_thread_id = GetCurrentThreadId();
        m.message           = str_message;
        m.icon              = icon;

        BOOL result;
        do {
            result = PostThreadMessage(m_thread_id, message, 0, (LPARAM)&m);
        } while (!result);

        // now wait for response?
        MSG msg;
        while (GetMessage(&msg, NULL, 0, 0) >= 0) {
            if (msg.message == UN_RESULT) {
                return msg.lParam;
            }
        }

        return 0;
    }

private:
    DWORD m_thread_id;
    bool  m_should_cancel;
};

DWORD WINAPI
ShellExtension::WorkerThread(LPVOID arg)
{
    std::auto_ptr<WorkerThreadParameters> p((WorkerThreadParameters*)arg);

    UIThreadParameters* parameters = new UIThreadParameters;
    parameters->parent_thread_id = GetCurrentThreadId();

    // create the UI thread
    DWORD thread_id;
    HANDLE thread = CreateThread(
        NULL,
        0,
        UIThread,
        parameters,
        0,
        &thread_id);
    if (thread == INVALID_HANDLE_VALUE) {
        delete parameters;
        MessageBox(
            NULL,
            "Error: Could not create UI thread",
            "WinTarBall",
            MB_OK);
        return 0;
    }

    std::auto_ptr<UINotification> notification(new UINotification(thread_id));
    UINotification* n = notification.get();

    std::list<std::string>::iterator i = p->paths.begin();
    for (; i != p->paths.end(); ++i) {

        switch (p->command) {
            case COMPRESS_TARBALL: {
                CreateArchive(i->c_str(), OpenANSIOutputFile);
            } break;

            case COMPRESS_BZBALL: {
                CreateArchive(i->c_str(), OpenBzip2OutputFile);
            } break;

            case COMPRESS_GZBALL: {
                CreateArchive(i->c_str(), OpenGzipOutputFile);
            } break;

            case COMPRESS_BZFILE: {
                TransformFile(n, i->c_str(), Bzip2Transform);
            } break;

            case COMPRESS_GZFILE: {
                TransformFile(n, i->c_str(), GzipTransform);
            } break;

            case DECOMPRESS_TARBALL: {
                ExtractArchive(i->c_str(), OpenANSIInputFile);
            } break;

            case DECOMPRESS_BZBALL: {
                ExtractArchive(i->c_str(), OpenBzip2InputFile);
            } break;

            case DECOMPRESS_GZBALL: {
                ExtractArchive(i->c_str(), OpenGzipInputFile);
            } break;

            case DECOMPRESS_BZFILE: {
                TransformFile(n, i->c_str(), Unbzip2Transform);
            } break;

            case DECOMPRESS_GZFILE: {
                TransformFile(n, i->c_str(), UngzipTransform);
            } break;
        }
    }

    // kill the UI thread
    bool done = false;
    while (!done) {
        int result = PostThreadMessage(thread_id, WM_QUIT, 0, 0);
        if (result) {
            done = true;
        } else {
            Sleep(50);
        }
    }

    CloseHandle(thread);
    return 0;
}

////////////////////////////////////////////////////////////////////////////////

DWORD WINAPI
ShellExtension::UIThread(LPVOID arg)
{
    std::auto_ptr<UIThreadParameters> p((UIThreadParameters*)arg);

    // create the dialog box
    HWND dialog = CreateDialog(
        _Module.m_hInst,
        MAKEINTRESOURCE(IDD_PROGRESS),
        NULL,
        UIDialogProc);
    if (!dialog) {
        return 0;
    }

    // attach the parent thread ID to the dialog
    SetWindowLong(dialog, GWL_USERDATA, p->parent_thread_id);

    // message loop
    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0) > 0) {
        if (msg.message == UN_YESNOCANCEL) {

            UI_MESSAGE* ui = (UI_MESSAGE*)msg.lParam;
            int icon = (ui->icon == UI_ERROR ? MB_ICONERROR : MB_ICONWARNING);
            int result = MessageBox(dialog, ui->message, "WinTarBall",
                MB_YESNOCANCEL | icon);

            // send the result back
            int rv = (result == IDYES ?
                        UI_YES :
                        (result == IDNO ? UI_NO : IDCANCEL));
            PostThreadMessage(ui->calling_thread_id, UN_RESULT, 0, rv);

        } else if (msg.message == UN_YESNO) {

            UI_MESSAGE* ui = (UI_MESSAGE*)msg.lParam;
            int icon = (ui->icon == UI_ERROR ? MB_ICONERROR : MB_ICONWARNING);
            int result = MessageBox(dialog, ui->message, "WinTarBall",
                MB_YESNO | icon);

            // send the result back
            int rv = (result == IDYES ?
                        UI_YES :
                        (result == IDNO ? UI_NO : UI_CANCEL));
            PostThreadMessage(ui->calling_thread_id, UN_RESULT, 0, rv);

        } else if (msg.message == UN_MESSAGE) {
            char* message = (char*)msg.lParam;
            SetDlgItemText(dialog, IDC_MESSAGE, message);
            delete[] message;

        } else if (msg.message == UN_PROGRESS) {
            // set the progress bar
            SendDlgItemMessage(
                dialog,
                IDC_PROGRESS,
                PBM_SETPOS,
                msg.lParam,
                0);

        } else if (!IsDialogMessage(dialog, &msg)) {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }

    // go home
    DestroyWindow(dialog);
    return 0;
}

////////////////////////////////////////////////////////////////////////////////

BOOL CALLBACK
ShellExtension::UIDialogProc(
    HWND dialog,
    UINT message,
    WPARAM wparam,
    LPARAM lparam)
{
    switch (message) {
        case WM_INITDIALOG: {
            SetDlgItemText(
                dialog,
                IDC_MESSAGE,
                "Processing");

            SendDlgItemMessage(
                dialog,
                IDC_PROGRESS,
                PBM_SETRANGE,
                0,
                MAKELPARAM(0, 255));
            return TRUE;
        }

        case WM_COMMAND: {
            if (LOWORD(wparam) == IDCANCEL) {
                DWORD parent_thread_id = GetWindowLong(dialog, GWL_USERDATA);
                PostThreadMessage(parent_thread_id, UN_CANCEL, 0, 0);
            }
            return TRUE;
        }

        default: {
            return FALSE;
        }
    }
}

////////////////////////////////////////////////////////////////////////////////
