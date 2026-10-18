#include "LoginPage.h"
#include "Resource.h"

namespace
{
    constexpr int kFormWidth = 300;
    constexpr int kTitleHeight = 40;
    constexpr int kLabelHeight = 20;
    constexpr int kEditHeight = 26;
    constexpr int kButtonHeight = 32;
    constexpr int kMessageHeight = 40;
    constexpr int kFormHeight = kTitleHeight + 16 + 2 * (kLabelHeight + 4 + kEditHeight + 12) + 8 +
                                2 * kButtonHeight + 8 + 12 + kMessageHeight;

    constexpr COLORREF kErrorColor = RGB(196, 43, 28);

    bool g_messageIsError = false;

    constexpr int kControlIds[] = {
        IDC_LOGIN_TITLE, IDC_LOGIN_USERNAME_LABEL, IDC_LOGIN_USERNAME, IDC_LOGIN_PASSWORD_LABEL,
        IDC_LOGIN_PASSWORD, IDC_LOGIN_BTN, IDC_CREATE_ACCOUNT_BTN, IDC_LOGIN_MESSAGE,
    };

    HWND MakeControl(HWND hParent, HINSTANCE hInstance, DWORD exStyle, const wchar_t* className,
                     const wchar_t* text, DWORD style, int id, HFONT hFont)
    {
        HWND hControl = CreateWindowExW(exStyle, className, text, WS_CHILD | style, 0, 0, 0, 0, hParent,
                                        reinterpret_cast<HMENU>(static_cast<INT_PTR>(id)), hInstance, nullptr);
        if (hControl && hFont)
        {
            SendMessageW(hControl, WM_SETFONT, reinterpret_cast<WPARAM>(hFont), TRUE);
        }
        return hControl;
    }

    std::wstring GetText(HWND hParent, int id)
    {
        HWND hControl = GetDlgItem(hParent, id);
        int length = GetWindowTextLengthW(hControl);
        std::wstring text(length + 1, L'\0');
        GetWindowTextW(hControl, text.data(), length + 1);
        text.resize(length);
        return text;
    }
}

namespace LoginPage
{
    void Create(HWND hParent, HINSTANCE hInstance, HFONT hFont, HFONT hTitleFont)
    {
        // Created in tab order: Username, Password, Login, Create Account.
        MakeControl(hParent, hInstance, 0, L"STATIC", L"Password Manager", SS_CENTER, IDC_LOGIN_TITLE, hTitleFont);
        MakeControl(hParent, hInstance, 0, L"STATIC", L"Username", SS_LEFT, IDC_LOGIN_USERNAME_LABEL, hFont);
        HWND hUser = MakeControl(hParent, hInstance, WS_EX_CLIENTEDGE, L"EDIT", L"",
                                 WS_TABSTOP | ES_AUTOHSCROLL, IDC_LOGIN_USERNAME, hFont);
        MakeControl(hParent, hInstance, 0, L"STATIC", L"Password", SS_LEFT, IDC_LOGIN_PASSWORD_LABEL, hFont);
        HWND hPass = MakeControl(hParent, hInstance, WS_EX_CLIENTEDGE, L"EDIT", L"",
                                 WS_TABSTOP | ES_AUTOHSCROLL | ES_PASSWORD, IDC_LOGIN_PASSWORD, hFont);
        MakeControl(hParent, hInstance, 0, L"BUTTON", L"Login", WS_TABSTOP | BS_DEFPUSHBUTTON, IDC_LOGIN_BTN, hFont);
        MakeControl(hParent, hInstance, 0, L"BUTTON", L"Create Account", WS_TABSTOP | BS_PUSHBUTTON, IDC_CREATE_ACCOUNT_BTN, hFont);
        MakeControl(hParent, hInstance, 0, L"STATIC", L"", SS_CENTER, IDC_LOGIN_MESSAGE, hFont);

        SendMessageW(hUser, EM_LIMITTEXT, 256, 0);
        SendMessageW(hPass, EM_LIMITTEXT, 256, 0);
    }

    void Layout(HWND hParent, int clientWidth, int clientHeight)
    {
        int x = (clientWidth - kFormWidth) / 2;
        int y = (clientHeight - kFormHeight) / 2;
        if (x < 16) x = 16;
        if (y < 16) y = 16;

        auto place = [&](int id, int height, int gapAfter) {
            MoveWindow(GetDlgItem(hParent, id), x, y, kFormWidth, height, TRUE);
            y += height + gapAfter;
        };
        place(IDC_LOGIN_TITLE, kTitleHeight, 16);
        place(IDC_LOGIN_USERNAME_LABEL, kLabelHeight, 4);
        place(IDC_LOGIN_USERNAME, kEditHeight, 12);
        place(IDC_LOGIN_PASSWORD_LABEL, kLabelHeight, 4);
        place(IDC_LOGIN_PASSWORD, kEditHeight, 12 + 8);
        place(IDC_LOGIN_BTN, kButtonHeight, 8);
        place(IDC_CREATE_ACCOUNT_BTN, kButtonHeight, 12);
        place(IDC_LOGIN_MESSAGE, kMessageHeight, 0);
    }

    void Show(HWND hParent, bool show)
    {
        for (int id : kControlIds)
        {
            ShowWindow(GetDlgItem(hParent, id), show ? SW_SHOW : SW_HIDE);
        }
    }

    bool IsShown(HWND hParent)
    {
        return IsWindowVisible(GetDlgItem(hParent, IDC_LOGIN_USERNAME)) != FALSE;
    }

    void Reset(HWND hParent, const std::wstring& username)
    {
        SetDlgItemTextW(hParent, IDC_LOGIN_USERNAME, username.c_str());
        SetDlgItemTextW(hParent, IDC_LOGIN_PASSWORD, L"");
        SetFocus(GetDlgItem(hParent, username.empty() ? IDC_LOGIN_USERNAME : IDC_LOGIN_PASSWORD));
    }

    std::wstring Username(HWND hParent)
    {
        std::wstring text = GetText(hParent, IDC_LOGIN_USERNAME);
        size_t first = text.find_first_not_of(L" \t");
        if (first == std::wstring::npos)
        {
            return std::wstring();
        }
        size_t last = text.find_last_not_of(L" \t");
        return text.substr(first, last - first + 1);
    }

    std::wstring Password(HWND hParent)
    {
        return GetText(hParent, IDC_LOGIN_PASSWORD);
    }

    void ClearPassword(HWND hParent)
    {
        SetDlgItemTextW(hParent, IDC_LOGIN_PASSWORD, L"");
    }

    void FocusPassword(HWND hParent)
    {
        SetFocus(GetDlgItem(hParent, IDC_LOGIN_PASSWORD));
    }

    void SetMessage(HWND hParent, const std::wstring& message, bool isError)
    {
        g_messageIsError = isError;
        HWND hMessage = GetDlgItem(hParent, IDC_LOGIN_MESSAGE);
        SetWindowTextW(hMessage, message.c_str());
        InvalidateRect(hMessage, nullptr, TRUE);
    }

    void EnableActions(HWND hParent, bool enabled)
    {
        EnableWindow(GetDlgItem(hParent, IDC_LOGIN_BTN), enabled ? TRUE : FALSE);
        EnableWindow(GetDlgItem(hParent, IDC_CREATE_ACCOUNT_BTN), enabled ? TRUE : FALSE);
    }

    HBRUSH OnCtlColorStatic(HDC hdc, HWND hControl)
    {
        // All labels on the main window sit on the white window background.
        bool error = GetDlgCtrlID(hControl) == IDC_LOGIN_MESSAGE && g_messageIsError;
        SetTextColor(hdc, error ? kErrorColor : GetSysColor(COLOR_WINDOWTEXT));
        SetBkColor(hdc, GetSysColor(COLOR_WINDOW));
        return GetSysColorBrush(COLOR_WINDOW);
    }
}
