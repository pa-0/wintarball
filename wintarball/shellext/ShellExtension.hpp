#ifndef SHELL_EXTENSION_HPP
#define SHELL_EXTENSION_HPP


// disable __uuidof
#define _ATL_NO_UUIDOF


#include <atlbase.h>
extern CComModule _Module;
#include <atlcom.h>
#include <shlobj.h>
#include <list>
#include <string>
#include "resource.h"


// {FA78FD00-4ED3-11d4-83CC-00609717727E}
extern const CLSID CLSID_ShellExtension;


class ATL_NO_VTABLE ShellExtension
: public CComObjectRootEx<CComSingleThreadModel>
, public CComCoClass<ShellExtension, &CLSID_ShellExtension>
, public IShellExtInit
, public IContextMenu
{
public:
    ShellExtension();
    ~ShellExtension();

    DECLARE_REGISTRY_RESOURCEID(IDR_SHELLEXTENSION)

    BEGIN_COM_MAP(ShellExtension)
        COM_INTERFACE_ENTRY(IShellExtInit)
        COM_INTERFACE_ENTRY(IContextMenu)
    END_COM_MAP()

    // IShellExtInit
    STDMETHOD(Initialize)(LPCITEMIDLIST folder, LPDATAOBJECT object, HKEY key);

    // IContextMenu (in order of execution)
    STDMETHOD(QueryContextMenu)(HMENU menu, UINT index, UINT first, UINT last, UINT flags);
    STDMETHOD(GetCommandString)(UINT command, UINT flags, UINT* reserved, LPSTR name, UINT maxchars);
    STDMETHOD(InvokeCommand)(LPCMINVOKECOMMANDINFO ici);

private:
    enum { MAX_COMMANDS = 2 };

    enum TargetType {
        FOLDER,
        BZ_BALL,
        GZ_BALL,
        BZ_FILE,
        GZ_FILE,
        NORMAL_FILE,
        UNKNOWN,  // maybe a mix of files
    };

    enum Command {
        COMPRESS_BZBALL,
        COMPRESS_GZBALL,
        COMPRESS_BZFILE,
        COMPRESS_GZFILE,
        DECOMPRESS_BZBALL,
        DECOMPRESS_GZBALL,
        DECOMPRESS_BZFILE,
        DECOMPRESS_GZFILE,
        NO_COMMAND,
    };

    struct ThreadParameters {
        Command                command;
        std::list<std::string> paths;
    };

private:
    void DetermineTargetType();
    TargetType GetTargetType(const char* path);
    static DWORD WINAPI WorkerThread(LPVOID parameter);

private:
    static const char* s_verbs[NO_COMMAND];

    std::list<std::string> m_paths;
    TargetType             m_target_type;
    Command                m_available_commands[MAX_COMMANDS];
    HBITMAP                m_menu_bitmap;
};


#endif
