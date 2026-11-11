#pragma once

// Shared helpers for dialogs that show the four password fields
// (IDC_FIELD_USERNAME, IDC_FIELD_PASSWORD, IDC_FIELD_URL, IDC_FIELD_DESCRIPTION).
// Used by the Add Password and View Password dialogs so both behave the same way.

#include "framework.h"
#include "../../app/structs/password.hpp"

// Sets the maximum length of each input box. Call from WM_INITDIALOG.
void LimitPasswordFormText(HWND hDlg);

// Fills the input boxes from a password (the core stores UTF-8 with "\n" line breaks).
void FillPasswordForm(HWND hDlg, const password& source);

// Enables or disables all four input boxes.
void SetPasswordFormEnabled(HWND hDlg, bool enabled);

// Reads and validates the input boxes. On a validation problem it shows a warning,
// focuses the offending box and returns false. On success it fills `result`'s
// username, encryped_password, url and description and returns true; the id and
// dates are left for the caller to set.
bool ReadPasswordForm(HWND hDlg, password& result);
