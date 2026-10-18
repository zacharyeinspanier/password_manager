#include "AddPasswordDialog.h"
#include "PasswordForm.h"
#include "Resource.h"

#include "../../../app/objects/displayContent.hpp"
#include "../../../app/structs/operations.hpp"
#include "../../../app/structs/password.hpp"

namespace
{
    // Reads the input boxes, validates them, and queues an ADD operation.
    // Returns true if the operation was queued and the dialog can close.
    bool SubmitPassword(HWND hDlg, DisplayContent* userContent)
    {
        // Same values as add_password::construct_password() in the Qt UI.
        // p_id = -1 and date_created = 0 tell UserAccount::add_password() to assign the id and dates.
        password newPassword;
        newPassword.p_id = -1;
        newPassword.date_created = 0;
        newPassword.date_modified = 0;

        if (!ReadPasswordForm(hDlg, newPassword))
        {
            return false;
        }

        if (userContent == nullptr)
        {
            MessageBoxW(hDlg, L"No user session is active, so the password can't be saved.",
                        L"Add Password", MB_OK | MB_ICONERROR);
            return false;
        }

        operation newOperation;
        newOperation.operation_type = operationType::ADD;
        newOperation.curr_password = newPassword;

        // Thread-safe: pushes onto DisplayContent's operation queue and wakes the operation thread.
        userContent->operation_event(newOperation);
        return true;
    }

    INT_PTR CALLBACK AddPasswordDlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
    {
        switch (message)
        {
        case WM_INITDIALOG:
            // lParam is the DisplayContent* passed to DialogBoxParamW.
            SetWindowLongPtrW(hDlg, DWLP_USER, lParam);
            LimitPasswordFormText(hDlg);
            return TRUE; // let the dialog manager focus the first input box

        case WM_COMMAND:
            switch (LOWORD(wParam))
            {
            case IDOK: // "Submit" (also triggered by Enter)
            {
                auto* userContent = reinterpret_cast<DisplayContent*>(GetWindowLongPtrW(hDlg, DWLP_USER));
                if (SubmitPassword(hDlg, userContent))
                {
                    EndDialog(hDlg, IDOK);
                }
                return TRUE;
            }
            case IDCANCEL: // "Cancel", Esc, or the title bar X
                EndDialog(hDlg, IDCANCEL);
                return TRUE;
            }
            break;
        }
        return FALSE;
    }
}

INT_PTR ShowAddPasswordDialog(HINSTANCE hInstance, HWND hParent, DisplayContent* userContent)
{
    return DialogBoxParamW(hInstance, MAKEINTRESOURCEW(IDD_ADD_PASSWORD), hParent,
                           AddPasswordDlgProc, reinterpret_cast<LPARAM>(userContent));
}
