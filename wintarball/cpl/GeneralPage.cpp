#include <windows.h>
#include <commctrl.h>
#include "GeneralPage.hpp"
#include "../common/Configuration.hpp"
#include "resource.h"


////////////////////////////////////////////////////////////////////////////////

BOOL CALLBACK GeneralPageProc(HWND window, UINT message, WPARAM wparam, LPARAM lparam)
{
    switch (message) {
        
        // OnInitDialog
        // this dialog does nothing now

        case WM_INITDIALOG: {
            return FALSE;
        }

        default: {
            return FALSE;
        }
    }
}

////////////////////////////////////////////////////////////////////////////////
