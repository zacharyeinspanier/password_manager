#pragma once

// Win32 port of ui/add_password (Qt).
//
// Shows a modal "Add Password" dialog (resource IDD_ADD_PASSWORD). When the user presses Submit,
// the input boxes are read, an ADD operation is built, and it is queued on the core app's
// operation queue through DisplayContent::operation_event().

#include "framework.h"

class DisplayContent;

// Returns IDOK if a password was queued, IDCANCEL if the user cancelled.
INT_PTR ShowAddPasswordDialog(HINSTANCE hInstance, HWND hParent, DisplayContent* userContent);
