#include "PasswordGrid.h"
#include "Resource.h"
#include "StringConvert.h"

#include <string>

namespace
{
    constexpr int kIconSize = 32;
    constexpr int kTileWidth = 140;   // horizontal spacing between tiles (pixels)
    constexpr int kTileHeight = 90;   // vertical spacing between tiles (pixels)

    // Tile label: the description on one line, or a placeholder when it's empty.
    std::wstring TileLabel(const password& item)
    {
        std::wstring label = Utf8ToWide(item.description);
        for (wchar_t& c : label)
        {
            if (c == L'\r' || c == L'\n' || c == L'\t')
            {
                c = L' ';
            }
        }
        if (label.find_first_not_of(L' ') == std::wstring::npos)
        {
            label = L"(no description)";
        }
        return label;
    }

    // True when the two lists would produce different tiles (different ids or labels).
    bool TilesDiffer(const std::vector<password>& a, const std::vector<password>& b)
    {
        if (a.size() != b.size())
        {
            return true;
        }
        for (size_t i = 0; i < a.size(); ++i)
        {
            if (a[i].p_id != b[i].p_id || a[i].description != b[i].description)
            {
                return true;
            }
        }
        return false;
    }
}

PasswordGrid::~PasswordGrid()
{
    // The ListView destroys hImages_ itself unless LVS_SHAREIMAGELISTS is set; it is set below,
    // so the grid owns the image list.
    if (hImages_)
    {
        ImageList_Destroy(hImages_);
    }
}

bool PasswordGrid::Create(HWND hParent, HINSTANCE hInstance, int controlId, HFONT hFont)
{
    hList_ = CreateWindowExW(WS_EX_CLIENTEDGE, WC_LISTVIEWW, L"",
        WS_CHILD | WS_VISIBLE | WS_TABSTOP | LVS_ICON | LVS_SINGLESEL | LVS_AUTOARRANGE |
        LVS_SORTASCENDING | LVS_SHOWSELALWAYS | LVS_SHAREIMAGELISTS,
        0, 0, 0, 0, hParent, reinterpret_cast<HMENU>(static_cast<INT_PTR>(controlId)), hInstance, nullptr);
    if (!hList_)
    {
        return false;
    }

    ListView_SetExtendedListViewStyle(hList_, LVS_EX_DOUBLEBUFFER);
    if (hFont)
    {
        SendMessageW(hList_, WM_SETFONT, reinterpret_cast<WPARAM>(hFont), TRUE);
    }

    // Every tile uses the app icon.
    hImages_ = ImageList_Create(kIconSize, kIconSize, ILC_COLOR32 | ILC_MASK, 1, 0);
    HICON hIcon = static_cast<HICON>(LoadImageW(hInstance, MAKEINTRESOURCEW(IDI_PASSWORDMANAGERWINDOWSUI),
                                                IMAGE_ICON, kIconSize, kIconSize, LR_DEFAULTCOLOR));
    if (hIcon)
    {
        ImageList_AddIcon(hImages_, hIcon);
        DestroyIcon(hIcon);
    }
    ListView_SetImageList(hList_, hImages_, LVSIL_NORMAL);
    ListView_SetIconSpacing(hList_, kTileWidth, kTileHeight);
    return true;
}

void PasswordGrid::SetBounds(int x, int y, int width, int height)
{
    if (hList_)
    {
        MoveWindow(hList_, x, y, width, height, TRUE);
    }
}

void PasswordGrid::Refresh(const std::vector<password>& passwords)
{
    bool rebuild = TilesDiffer(passwords_, passwords);

    // Always keep the latest copy so the dialog opens with current username/password/url,
    // even when the tiles themselves don't need to change.
    passwords_ = passwords;

    if (rebuild)
    {
        RebuildItems();
    }
}

void PasswordGrid::RebuildItems()
{
    // Remember the selection by password id so it survives the rebuild.
    int selectedId = -1;
    if (const password* selected = SelectedPassword())
    {
        selectedId = selected->p_id;
    }

    SendMessageW(hList_, WM_SETREDRAW, FALSE, 0);
    ListView_DeleteAllItems(hList_);

    for (const password& item : passwords_)
    {
        std::wstring label = TileLabel(item);
        LVITEMW lvItem = {};
        lvItem.mask = LVIF_TEXT | LVIF_IMAGE | LVIF_PARAM;
        lvItem.iItem = ListView_GetItemCount(hList_);
        lvItem.iImage = 0;
        lvItem.pszText = label.data();
        lvItem.lParam = item.p_id; // look the password up by id, since the ListView sorts items
        int index = ListView_InsertItem(hList_, &lvItem);

        if (item.p_id == selectedId && index >= 0)
        {
            ListView_SetItemState(hList_, index, LVIS_SELECTED | LVIS_FOCUSED, LVIS_SELECTED | LVIS_FOCUSED);
        }
    }

    SendMessageW(hList_, WM_SETREDRAW, TRUE, 0);
    InvalidateRect(hList_, nullptr, TRUE);
}

const password* PasswordGrid::FindById(int p_id) const
{
    for (const password& item : passwords_)
    {
        if (item.p_id == p_id)
        {
            return &item;
        }
    }
    return nullptr;
}

const password* PasswordGrid::PasswordAtItem(int itemIndex) const
{
    if (!hList_ || itemIndex < 0)
    {
        return nullptr;
    }
    LVITEMW lvItem = {};
    lvItem.mask = LVIF_PARAM;
    lvItem.iItem = itemIndex;
    if (!ListView_GetItem(hList_, &lvItem))
    {
        return nullptr;
    }
    return FindById(static_cast<int>(lvItem.lParam));
}

const password* PasswordGrid::SelectedPassword() const
{
    if (!hList_)
    {
        return nullptr;
    }
    return PasswordAtItem(ListView_GetNextItem(hList_, -1, LVNI_SELECTED));
}
