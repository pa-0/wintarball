#include <windows.h>
#include <cpl.h>
#include <prsht.h>
#include "GeneralPage.hpp"
#include "BZip2Page.hpp"
#include "GZipPage.hpp"
#include "../common/Configuration.hpp"
#include "resource.h"


static void ExecuteControlPanel(HWND parent);


HINSTANCE g_Instance;


////////////////////////////////////////////////////////////////////////////////

LONG WINAPI CPlApplet(HWND window, UINT message, LONG param1, LONG param2)
{
    switch (message) {

        case CPL_INIT: {
            g_Instance = GetModuleHandle("wtb.cpl");
            return TRUE;
        }

        case CPL_GETCOUNT: {
            return 1;
        }

        case CPL_INQUIRE: {
            if (param1 == 0) {
                LPCPLINFO cplinfo = (LPCPLINFO)param2;
                cplinfo->lData = 0;
                cplinfo->idIcon = IDI_CPL;
                cplinfo->idName = IDS_NAME;
                cplinfo->idInfo = IDS_INFO;
            }
            return 0;
        }

        case CPL_DBLCLK: {
            ExecuteControlPanel(window);
            return 0;
        }

        default: {
            return FALSE;
        }
    }
}

////////////////////////////////////////////////////////////////////////////////

void ExecuteControlPanel(HWND parent)
{
    g_Configuration.Load();

    // pages
    PROPSHEETPAGE pages[] = {
        { sizeof(PROPSHEETPAGE), 0, g_Instance, MAKEINTRESOURCE(IDD_PAGE_GENERAL), NULL, NULL, GeneralPageProc },
        { sizeof(PROPSHEETPAGE), 0, g_Instance, MAKEINTRESOURCE(IDD_PAGE_BZIP2),   NULL, NULL, BZip2PageProc },
        { sizeof(PROPSHEETPAGE), 0, g_Instance, MAKEINTRESOURCE(IDD_PAGE_GZIP),    NULL, NULL, GZipPageProc },
    };

    // property sheet header
    PROPSHEETHEADER psh;
    memset(&psh, 0, sizeof(psh));
    psh.dwSize     = sizeof(psh);
    psh.dwFlags    = PSH_PROPSHEETPAGE;
    psh.hwndParent = parent;
    psh.hInstance  = g_Instance;
    psh.pszCaption = "WinTarBall Configuration";
    psh.nPages     = sizeof(pages) / sizeof(*pages);
    psh.nStartPage = 0;
    psh.ppsp       = pages;
    
    // execute it!
    PropertySheet(&psh);
}

////////////////////////////////////////////////////////////////////////////////
