#pragma once

// "Create Account" dialog (resource IDD_CREATE_ACCOUNT): username, password, confirm password,
// with Create Account and Cancel buttons. The passwords must match. On success the account is
// added to the ACCOUNTS table and the dialog closes.

#include "framework.h"
#include <string>

// Returns true if an account was created; createdUsername is then set so the login page can
// be filled in for the user.
bool ShowCreateAccountDialog(HINSTANCE hInstance, HWND hParent, const std::string& dbPathUtf8,
                             std::wstring& createdUsername);
