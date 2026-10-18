#pragma once

// Win32 port of ui/view_password (Qt).
//
// Shows a modal "View Password" dialog (resource IDD_VIEW_PASSWORD) for one saved password.
// The input boxes start disabled. "Edit" enables them; "Save" reads them, builds a MODIFY
// operation and queues it through DisplayContent::operation_event(), then goes back to view mode.
// "Delete" asks for confirmation, queues a REMOVE operation, and closes the dialog.

#include "framework.h"
#include "../../app/structs/password.hpp"

class DisplayContent;

// Returns IDOK if at least one change (edit or delete) was queued, IDCANCEL otherwise.
INT_PTR ShowViewPasswordDialog(HINSTANCE hInstance, HWND hParent, DisplayContent* userContent,
                               const password& selected);
