#include <windows.h>
#include <commctrl.h>
#include <stdio.h>
#include "GZipPage.hpp"
#include "../common/Configuration.hpp"
#include "resource.h"


static void UpdateLabel(HWND window);


////////////////////////////////////////////////////////////////////////////////

BOOL CALLBACK GZipPageProc(HWND window, UINT message, WPARAM wparam, LPARAM lparam)
{
    switch (message) {

        // OnInitDialog
        
        case WM_INITDIALOG: {
            
            // compression level
            SendDlgItemMessage(window, IDC_COMPRESSION_LEVEL, TBM_SETRANGE, FALSE, MAKELONG(1, 9));
            SendDlgItemMessage(window, IDC_COMPRESSION_LEVEL, TBM_SETPOS, TRUE, g_Configuration.gzip_compression_level);

            UpdateLabel(window);

            return FALSE;
        }

        case WM_NOTIFY: {
            PSHNOTIFY* psh = (PSHNOTIFY*)lparam;

            // OnApply
            if (psh->hdr.code == PSN_APPLY) {
                
                // block size
                g_Configuration.gzip_compression_level = 
                    SendDlgItemMessage(window, IDC_COMPRESSION_LEVEL, TBM_GETPOS, 0, 0);

                if (!g_Configuration.Save()) {
                    MessageBox(window, "Couldn't save configuration!", "WinTarBall", MB_OK);
                    SetWindowLong(window, DWL_MSGRESULT, PSNRET_INVALID_NOCHANGEPAGE);
                    return TRUE;
                }

                return TRUE;
            }

            return FALSE;
        }

        case WM_HSCROLL: {
            UpdateLabel(window);
            PropSheet_Changed(GetParent(window), window);
            return TRUE;
        }

        default: {
            return FALSE;
        }
    }
}

////////////////////////////////////////////////////////////////////////////////

void UpdateLabel(HWND window)
{
    int pos = SendDlgItemMessage(window, IDC_COMPRESSION_LEVEL, TBM_GETPOS, 0, 0);
    char label[500];
    char* help = "";
    switch (pos) {
        case 1: help = "(fastest)"; break;
        case 5: help = "(average)"; break;
        case 9: help = "(best)"; break;
    }
    sprintf(label, "%d %s", pos, help);
    SetDlgItemText(window, IDC_COMPRESSION_LEVEL_LABEL, label);
}

////////////////////////////////////////////////////////////////////////////////
