#include "CreateAccountDialog.h"
#include "AccountStore.h"
#include "Resource.h"
#include "StringConvert.h"

namespace
{
    constexpr int kMaxFieldLength = 256;

    struct CreateAccountState
    {
        const std::string* dbPathUtf8 = nullptr;
        std::wstring createdUsername;
    };

    std::wstring GetControlText(HWND hDlg, int controlId)
    {
        HWND hControl = GetDlgItem(hDlg, controlId);
        int length = GetWindowTextLengthW(hControl);
        std::wstring text(length + 1, L'\0');
        GetWindowTextW(hControl, text.data(), length + 1);
        text.resize(length);
        return text;
    }

    std::wstring Trim(const std::wstring& text)
    {
        size_t first = text.find_first_not_of(L" \t");
        if (first == std::wstring::npos)
        {
            return std::wstring();
        }
        size_t last = text.find_last_not_of(L" \t");
        return text.substr(first, last - first + 1);
    }

    void RejectField(HWND hDlg, int controlId, const std::wstring& message)
    {
        MessageBoxW(hDlg, message.c_str(), L"Create Account", MB_OK | MB_ICONWARNING);
        HWND hControl = GetDlgItem(hDlg, controlId);
        SetFocus(hControl);
        SendMessageW(hControl, EM_SETSEL, 0, -1);
    }

    // Validates the input and creates the account. Returns true if the dialog can close.
    bool SubmitAccount(HWND hDlg, CreateAccountState* state)
    {
        std::wstring username = Trim(GetControlText(hDlg, IDC_ACCOUNT_USERNAME));
        std::wstring password = GetControlText(hDlg, IDC_ACCOUNT_PASSWORD);
        std::wstring confirm = GetControlText(hDlg, IDC_ACCOUNT_CONFIRM);

        if (username.empty())
        {
            RejectField(hDlg, IDC_ACCOUNT_USERNAME, L"Please enter a username.");
            return false;
        }
        if (password.empty())
        {
            RejectField(hDlg, IDC_ACCOUNT_PASSWORD, L"Please enter a password.");
            return false;
        }
        if (password != confirm)
        {
            SetDlgItemTextW(hDlg, IDC_ACCOUNT_CONFIRM, L"");
            RejectField(hDlg, IDC_ACCOUNT_CONFIRM, L"The passwords don't match. Please confirm your password again.");
            return false;
        }

        std::wstring error;
        CreateAccountResult result = CreateAccount(*state->dbPathUtf8, WideToUtf8(username), WideToUtf8(password), error);
        SecureZeroMemory(password.data(), password.size() * sizeof(wchar_t));
        SecureZeroMemory(confirm.data(), confirm.size() * sizeof(wchar_t));

        switch (result)
        {
        case CreateAccountResult::Created:
            state->createdUsername = username;
            MessageBoxW(hDlg, (L"Account \"" + username + L"\" was created. You can log in now.").c_str(),
                        L"Create Account", MB_OK | MB_ICONINFORMATION);
            return true;
        case CreateAccountResult::UsernameTaken:
            RejectField(hDlg, IDC_ACCOUNT_USERNAME, L"The username \"" + username + L"\" already exists. Please pick another one.");
            return false;
        default:
            MessageBoxW(hDlg, error.c_str(), L"Create Account", MB_OK | MB_ICONERROR);
            return false;
        }
    }

    INT_PTR CALLBACK CreateAccountDlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
    {
        switch (message)
        {
        case WM_INITDIALOG:
            SetWindowLongPtrW(hDlg, DWLP_USER, lParam);
            SendDlgItemMessageW(hDlg, IDC_ACCOUNT_USERNAME, EM_LIMITTEXT, kMaxFieldLength, 0);
            SendDlgItemMessageW(hDlg, IDC_ACCOUNT_PASSWORD, EM_LIMITTEXT, kMaxFieldLength, 0);
            SendDlgItemMessageW(hDlg, IDC_ACCOUNT_CONFIRM, EM_LIMITTEXT, kMaxFieldLength, 0);
            return TRUE; // focus goes to the first input box

        case WM_COMMAND:
        {
            auto* state = reinterpret_cast<CreateAccountState*>(GetWindowLongPtrW(hDlg, DWLP_USER));
            if (state == nullptr)
            {
                break;
            }
            switch (LOWORD(wParam))
            {
            case IDOK: // "Create Account" (also Enter)
                if (SubmitAccount(hDlg, state))
                {
                    EndDialog(hDlg, IDOK);
                }
                return TRUE;
            case IDCANCEL: // "Cancel", Esc, or the title bar X
                EndDialog(hDlg, IDCANCEL);
                return TRUE;
            }
            break;
        }
        }
        return FALSE;
    }
}

bool ShowCreateAccountDialog(HINSTANCE hInstance, HWND hParent, const std::string& dbPathUtf8,
                             std::wstring& createdUsername)
{
    CreateAccountState state;
    state.dbPathUtf8 = &dbPathUtf8;
    INT_PTR result = DialogBoxParamW(hInstance, MAKEINTRESOURCEW(IDD_CREATE_ACCOUNT), hParent,
                                     CreateAccountDlgProc, reinterpret_cast<LPARAM>(&state));
    if (result == IDOK)
    {
        createdUsername = state.createdUsername;
        return true;
    }
    return false;
}
