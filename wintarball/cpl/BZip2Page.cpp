#include <windows.h>
#include <commctrl.h>
#include <stdio.h>
#include "BZip2Page.hpp"
#include "../common/Configuration.hpp"
#include "resource.h"


static void UpdateLabel(HWND window);


////////////////////////////////////////////////////////////////////////////////

BOOL CALLBACK BZip2PageProc(HWND window, UINT message, WPARAM wparam, LPARAM lparam)
{
    switch (message) {

        // OnInitDialog

        case WM_INITDIALOG: {
            
            // block size
            SendDlgItemMessage(window, IDC_BLOCK_SIZE, TBM_SETRANGE, FALSE, MAKELONG(1, 9));
            SendDlgItemMessage(window, IDC_BLOCK_SIZE, TBM_SETPOS, TRUE, g_Configuration.bzip2_block_size);

            UpdateLabel(window);

            // use less memory
            if (g_Configuration.bzip2_less_memory) {
                CheckDlgButton(window, IDC_USE_LESS_MEMORY, BST_CHECKED);
            }

            return FALSE;
        }

        case WM_NOTIFY: {
            PSHNOTIFY* psh = (PSHNOTIFY*)lparam;

            // OnApply
            if (psh->hdr.code == PSN_APPLY) {
                
                // block size
                g_Configuration.bzip2_block_size =
                    SendDlgItemMessage(window, IDC_BLOCK_SIZE, TBM_GETPOS, 0, 0);

                // use less memory
                g_Configuration.bzip2_less_memory =
                    IsDlgButtonChecked(window, IDC_USE_LESS_MEMORY) == BST_CHECKED;

                if (!g_Configuration.Save()) {
                    MessageBox(window, "Couldn't save configuration!", "WinTarBall", MB_OK);
                    SetWindowLong(window, DWL_MSGRESULT, PSNRET_INVALID_NOCHANGEPAGE);
                    return TRUE;
                }

                return TRUE;
            }

            return FALSE;
        }

        case WM_COMMAND: {
            if (HIWORD(wparam) == BN_CLICKED) {
                PropSheet_Changed(GetParent(window), window);
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
    int pos = SendDlgItemMessage(window, IDC_BLOCK_SIZE, TBM_GETPOS, 0, 0);
    char label[500];
    sprintf(label, "%d00k", pos);
    SetDlgItemText(window, IDC_BLOCK_SIZE_LABEL, label);
}

////////////////////////////////////////////////////////////////////////////////
