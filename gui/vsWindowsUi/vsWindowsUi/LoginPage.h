#pragma once

// Login page controls on the main window: title, Username, Password, Login and Create Account,
// plus a message line for errors ("Incorrect username or password") and notices.
//
// The page only owns its controls and layout; the main window decides what Login and
// Create Account do (see vsWindowsUi.cpp).

#include "framework.h"
#include <string>

namespace LoginPage
{
    // Creates the controls as children of hParent (initially hidden).
    void Create(HWND hParent, HINSTANCE hInstance, HFONT hFont, HFONT hTitleFont);

    // Centers the controls in the parent's client area.
    void Layout(HWND hParent, int clientWidth, int clientHeight);

    void Show(HWND hParent, bool show);
    bool IsShown(HWND hParent);

    // Clears both boxes (or pre-fills the username) and focuses the first empty box.
    void Reset(HWND hParent, const std::wstring& username = std::wstring());

    std::wstring Username(HWND hParent);   // trimmed
    std::wstring Password(HWND hParent);   // as typed
    void ClearPassword(HWND hParent);
    void FocusPassword(HWND hParent);

    // Message shown under the buttons. Errors are drawn in red.
    void SetMessage(HWND hParent, const std::wstring& message, bool isError);

    // Enables or disables Login / Create Account (e.g. when the database is missing).
    void EnableActions(HWND hParent, bool enabled);

    // Handles WM_CTLCOLORSTATIC for the main window's labels (white background; red for login errors).
    HBRUSH OnCtlColorStatic(HDC hdc, HWND hControl);
}
