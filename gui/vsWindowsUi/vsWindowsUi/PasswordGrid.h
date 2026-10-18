#pragma once

// Grid of saved passwords on the main page.
//
// Wraps a Win32 ListView in icon view: each password is one tile, labelled with its description,
// sorted alphabetically. The owner forwards WM_NOTIFY and calls Refresh() with the core's
// display list; the grid only rebuilds its tiles when the list of passwords actually changes,
// so the periodic refresh doesn't flicker or lose the selection.

#include "framework.h"
#include <commctrl.h>
#include <vector>

#include "../../../app/structs/password.hpp"

class PasswordGrid
{
public:
    PasswordGrid() = default;
    ~PasswordGrid();
    PasswordGrid(const PasswordGrid&) = delete;
    PasswordGrid& operator=(const PasswordGrid&) = delete;

    bool Create(HWND hParent, HINSTANCE hInstance, int controlId, HFONT hFont);
    void SetBounds(int x, int y, int width, int height);

    // Replaces the grid's data with the latest display list from DisplayContent.
    void Refresh(const std::vector<password>& passwords);

    // Returns the password for a ListView item index (e.g. from NM_CLICK), or nullptr.
    const password* PasswordAtItem(int itemIndex) const;

    // Returns the selected password, or nullptr.
    const password* SelectedPassword() const;

    // Number of passwords currently shown.
    size_t Count() const { return passwords_.size(); }

    HWND Handle() const { return hList_; }

private:
    const password* FindById(int p_id) const;
    void RebuildItems();

    HWND hList_ = nullptr;
    HIMAGELIST hImages_ = nullptr;
    std::vector<password> passwords_;
};
