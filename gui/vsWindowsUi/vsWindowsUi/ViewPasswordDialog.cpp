#include "ViewPasswordDialog.h"
#include "PasswordForm.h"
#include "Resource.h"

#include "../../../app/objects/displayContent.hpp"
#include "../../../app/structs/operations.hpp"

namespace
{
    // Per-dialog state, stored in DWLP_USER.
    struct ViewPasswordState
    {
        DisplayContent* userContent = nullptr;
        password current;          // what the dialog shows in view mode (updated after Save)
        bool editing = false;
        bool changesQueued = false;
    };

    ViewPasswordState* GetState(HWND hDlg)
    {
        return reinterpret_cast<ViewPasswordState*>(GetWindowLongPtrW(hDlg, DWLP_USER));
    }

    // View mode: inputs disabled, Save disabled, Delete enabled, button reads "Edit".
    // Edit mode: inputs enabled, Save enabled, Delete disabled, button reads "Cancel Edit".
    void SetEditMode(HWND hDlg, ViewPasswordState* state, bool editing)
    {
        state->editing = editing;
        SetPasswordFormEnabled(hDlg, editing);
        EnableWindow(GetDlgItem(hDlg, IDC_VIEW_SAVE_BTN), editing ? TRUE : FALSE);
        EnableWindow(GetDlgItem(hDlg, IDC_VIEW_DELETE_BTN), editing ? FALSE : TRUE);
        SetDlgItemTextW(hDlg, IDC_VIEW_EDIT_BTN, editing ? L"Cancel Edit" : L"Edit");

        // Default button (Enter) is Save while editing, Close otherwise.
        SendMessageW(hDlg, DM_SETDEFID, editing ? IDC_VIEW_SAVE_BTN : IDCANCEL, 0);

        if (editing)
        {
            HWND hUsername = GetDlgItem(hDlg, IDC_FIELD_USERNAME);
            SendMessageW(hDlg, WM_NEXTDLGCTL, reinterpret_cast<WPARAM>(hUsername), TRUE);
        }
        else
        {
            SendMessageW(hDlg, WM_NEXTDLGCTL, reinterpret_cast<WPARAM>(GetDlgItem(hDlg, IDC_VIEW_EDIT_BTN)), TRUE);
        }
    }

    // Reads the input boxes and queues a MODIFY operation (same as view_password::on_save_password_btn_clicked).
    bool SaveChanges(HWND hDlg, ViewPasswordState* state)
    {
        password updated = state->current; // keeps p_id and dates
        if (!ReadPasswordForm(hDlg, updated))
        {
            return false;
        }

        if (state->userContent == nullptr)
        {
            MessageBoxW(hDlg, L"No user session is active, so the changes can't be saved.",
                        L"View Password", MB_OK | MB_ICONERROR);
            return false;
        }

        operation modifyOperation;
        modifyOperation.operation_type = operationType::MODIFY;
        modifyOperation.curr_password = updated;
        state->userContent->operation_event(modifyOperation);

        state->current = updated;
        state->changesQueued = true;
        return true;
    }

    // Asks for confirmation, then queues a REMOVE operation
    // (same as view_password::on_password_delete_btn_clicked). Returns true if it was queued.
    bool DeletePassword(HWND hDlg, ViewPasswordState* state)
    {
        int answer = MessageBoxW(hDlg, L"About to remove password. Are you sure?", L"Delete Password",
                                 MB_YESNO | MB_ICONWARNING | MB_DEFBUTTON2); // default to "No"
        if (answer != IDYES)
        {
            return false;
        }

        if (state->userContent == nullptr)
        {
            MessageBoxW(hDlg, L"No user session is active, so the password can't be removed.",
                        L"Delete Password", MB_OK | MB_ICONERROR);
            return false;
        }

        // UserAccount::remove_password() only needs the id.
        operation removeOperation;
        removeOperation.operation_type = operationType::REMOVE;
        removeOperation.curr_password = state->current;
        state->userContent->operation_event(removeOperation);

        state->changesQueued = true;
        return true;
    }

    INT_PTR CALLBACK ViewPasswordDlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
    {
        switch (message)
        {
        case WM_INITDIALOG:
        {
            auto* state = reinterpret_cast<ViewPasswordState*>(lParam);
            SetWindowLongPtrW(hDlg, DWLP_USER, lParam);
            LimitPasswordFormText(hDlg);
            FillPasswordForm(hDlg, state->current);
            SetEditMode(hDlg, state, false);
            return FALSE; // focus was set in SetEditMode
        }

        case WM_COMMAND:
        {
            ViewPasswordState* state = GetState(hDlg);
            if (state == nullptr)
            {
                break; // message arrived before WM_INITDIALOG
            }
            switch (LOWORD(wParam))
            {
            case IDC_VIEW_EDIT_BTN:
                if (state->editing)
                {
                    // Cancel Edit: throw away unsaved typing.
                    FillPasswordForm(hDlg, state->current);
                    SetEditMode(hDlg, state, false);
                }
                else
                {
                    SetEditMode(hDlg, state, true);
                }
                return TRUE;

            case IDC_VIEW_SAVE_BTN:
                if (state->editing && SaveChanges(hDlg, state))
                {
                    FillPasswordForm(hDlg, state->current);
                    SetEditMode(hDlg, state, false);
                }
                return TRUE;

            case IDC_VIEW_DELETE_BTN:
                if (!state->editing && DeletePassword(hDlg, state))
                {
                    EndDialog(hDlg, IDOK); // the password is gone, nothing left to show
                }
                return TRUE;

            case IDOK:     // Enter with no default button handled above
            case IDCANCEL: // "Close", Esc, or the title bar X
                EndDialog(hDlg, state->changesQueued ? IDOK : IDCANCEL);
                return TRUE;
            }
            break;
        }
        }
        return FALSE;
    }
}

INT_PTR ShowViewPasswordDialog(HINSTANCE hInstance, HWND hParent, DisplayContent* userContent,
                               const password& selected)
{
    ViewPasswordState state;
    state.userContent = userContent;
    state.current = selected;
    return DialogBoxParamW(hInstance, MAKEINTRESOURCEW(IDD_VIEW_PASSWORD), hParent,
                           ViewPasswordDlgProc, reinterpret_cast<LPARAM>(&state));
}
