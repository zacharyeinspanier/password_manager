#include "PasswordForm.h"
#include "Resource.h"
#include "StringConvert.h"

#include <string>

namespace
{
    // Upper bound on characters accepted by each input box.
    constexpr int kMaxFieldLength = 256;
    constexpr int kMaxDescriptionLength = 1024;

    constexpr int kFieldIds[] = { IDC_FIELD_USERNAME, IDC_FIELD_PASSWORD, IDC_FIELD_URL, IDC_FIELD_DESCRIPTION };

    std::wstring GetControlText(HWND hDlg, int controlId)
    {
        HWND hControl = GetDlgItem(hDlg, controlId);
        int length = GetWindowTextLengthW(hControl);
        std::wstring text(length + 1, L'\0');
        GetWindowTextW(hControl, text.data(), length + 1);
        text.resize(length);
        return text;
    }

    // Multi-line edit controls use "\r\n"; the core stores plain "\n" (same as the Qt version).
    std::wstring ToStoredNewlines(const std::wstring& text)
    {
        std::wstring result;
        result.reserve(text.size());
        for (wchar_t c : text)
        {
            if (c != L'\r')
            {
                result.push_back(c);
            }
        }
        return result;
    }

    // Edit controls only break lines on "\r\n".
    std::wstring ToEditControlNewlines(const std::wstring& text)
    {
        std::wstring result;
        result.reserve(text.size());
        for (size_t i = 0; i < text.size(); ++i)
        {
            if (text[i] == L'\n' && (i == 0 || text[i - 1] != L'\r'))
            {
                result.push_back(L'\r');
            }
            result.push_back(text[i]);
        }
        return result;
    }

    // Shows a warning and puts keyboard focus on the field that needs fixing.
    void RejectField(HWND hDlg, int controlId, const wchar_t* message)
    {
        wchar_t title[128] = {};
        GetWindowTextW(hDlg, title, ARRAYSIZE(title));
        MessageBoxW(hDlg, message, title, MB_OK | MB_ICONWARNING);
        HWND hControl = GetDlgItem(hDlg, controlId);
        SetFocus(hControl);
        SendMessageW(hControl, EM_SETSEL, 0, -1);
    }
}

void LimitPasswordFormText(HWND hDlg)
{
    SendDlgItemMessageW(hDlg, IDC_FIELD_USERNAME, EM_LIMITTEXT, kMaxFieldLength, 0);
    SendDlgItemMessageW(hDlg, IDC_FIELD_PASSWORD, EM_LIMITTEXT, kMaxFieldLength, 0);
    SendDlgItemMessageW(hDlg, IDC_FIELD_URL, EM_LIMITTEXT, kMaxFieldLength, 0);
    SendDlgItemMessageW(hDlg, IDC_FIELD_DESCRIPTION, EM_LIMITTEXT, kMaxDescriptionLength, 0);
}

void FillPasswordForm(HWND hDlg, const password& source)
{
    SetDlgItemTextW(hDlg, IDC_FIELD_USERNAME, Utf8ToWide(source.username).c_str());
    SetDlgItemTextW(hDlg, IDC_FIELD_PASSWORD, Utf8ToWide(source.encryped_password).c_str());
    SetDlgItemTextW(hDlg, IDC_FIELD_URL, Utf8ToWide(source.url).c_str());
    SetDlgItemTextW(hDlg, IDC_FIELD_DESCRIPTION, ToEditControlNewlines(Utf8ToWide(source.description)).c_str());
}

void SetPasswordFormEnabled(HWND hDlg, bool enabled)
{
    for (int id : kFieldIds)
    {
        EnableWindow(GetDlgItem(hDlg, id), enabled ? TRUE : FALSE);
    }
}

bool ReadPasswordForm(HWND hDlg, password& result)
{
    std::wstring username = GetControlText(hDlg, IDC_FIELD_USERNAME);
    std::wstring pass = GetControlText(hDlg, IDC_FIELD_PASSWORD);
    std::wstring url = GetControlText(hDlg, IDC_FIELD_URL);
    std::wstring description = ToStoredNewlines(GetControlText(hDlg, IDC_FIELD_DESCRIPTION));

    if (username.empty())
    {
        RejectField(hDlg, IDC_FIELD_USERNAME, L"Please enter a username.");
        return false;
    }
    if (pass.empty())
    {
        RejectField(hDlg, IDC_FIELD_PASSWORD, L"Please enter a password.");
        return false;
    }

    // The core's data store thread builds its INSERT statement with string formatting
    // (see README "SQL injection prevention"), so a single quote makes the save fail silently.
    // Block it here until the core switches to parameterized queries.
    const struct { const std::wstring* text; int id; } fields[] = {
        { &username, IDC_FIELD_USERNAME },
        { &pass, IDC_FIELD_PASSWORD },
        { &url, IDC_FIELD_URL },
        { &description, IDC_FIELD_DESCRIPTION },
    };
    for (const auto& field : fields)
    {
        if (field.text->find(L'\'') != std::wstring::npos)
        {
            RejectField(hDlg, field.id,
                L"Single quotes (') can't be saved yet. Please remove them from this field.");
            return false;
        }
    }

    result.username = WideToUtf8(username);
    result.encryped_password = WideToUtf8(pass);
    result.url = WideToUtf8(url);
    result.description = WideToUtf8(description);

    // Don't leave an extra plain-text copy of the password in memory.
    SecureZeroMemory(pass.data(), pass.size() * sizeof(wchar_t));
    return true;
}
