#include "ShellExtension.hpp"


const CLSID CLSID_ShellExtension =
{ 0xFA78FD00, 0x4ED3, 0x11d4, 0x83, 0xCC, 0x00, 0x60, 0x97, 0x17, 0x72, 0x7E };


CComModule _Module;


BEGIN_OBJECT_MAP(ObjectMap)
    OBJECT_ENTRY(CLSID_ShellExtension, ShellExtension)
END_OBJECT_MAP()


////////////////////////////////////////////////////////////////////////////////

BOOL WINAPI DllMain(HINSTANCE instance, DWORD reason, LPVOID)
{
    if (reason == DLL_PROCESS_ATTACH) {

        _Module.Init(ObjectMap, instance);
        DisableThreadLibraryCalls(instance);

    } else if (reason == DLL_PROCESS_DETACH) {

        _Module.Term();

    }
    return TRUE;
}

////////////////////////////////////////////////////////////////////////////////

STDAPI DllCanUnloadNow()
{
    return (_Module.GetLockCount() == 0) ? S_OK : S_FALSE;
}

////////////////////////////////////////////////////////////////////////////////

STDAPI DllGetClassObject(REFCLSID rclsid, REFIID riid, LPVOID* ppv)
{
    return _Module.GetClassObject(rclsid, riid, ppv);
}

////////////////////////////////////////////////////////////////////////////////

STDAPI DllRegisterServer()
{
    return _Module.RegisterServer();
}

////////////////////////////////////////////////////////////////////////////////

STDAPI DllUnregisterServer()
{
    return _Module.UnregisterServer();
}

////////////////////////////////////////////////////////////////////////////////
