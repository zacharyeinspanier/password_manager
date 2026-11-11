// vsWindowsUi.cpp : Defines the entry point for the application.
//

#include "framework.h"
#include "vsWindowsUi.h"
#include "AddPasswordDialog.h"
#include "PasswordGrid.h"
#include "StringConvert.h"
#include "ViewPasswordDialog.h"

#include "../../../app/objects/displayContent.hpp"

#include <algorithm>
#include <filesystem>
#include <memory>
#include <string>

#include <commctrl.h>
#pragma comment(lib, "comctl32.lib")

// Use the modern (v6) look for buttons and edit boxes.
#pragma comment(linker, "\"/manifestdependency:type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")

#define MAX_LOADSTRING 100

// Timers on the main window.
#define IDT_REFRESH_GRID        1   // periodic: pick up changes from the core's threads
#define IDT_REFRESH_GRID_SOON   2   // one-shot: right after a dialog queues an operation or a search
#define IDT_SEARCH_DEBOUNCE     3   // one-shot: user stopped typing in the search box
static constexpr UINT kRefreshIntervalMs = 1000;
static constexpr UINT kRefreshSoonMs = 150;
static constexpr UINT kSearchDebounceMs = 250;

// Layout (pixels).
static constexpr int kMargin = 16;
static constexpr int kButtonWidth = 140;
static constexpr int kButtonHeight = 32;
static constexpr int kSearchHeight = 26;
static constexpr int kClearButtonWidth = 70;
static constexpr int kStatusHeight = 20;

// ---------------------------------------------------------------------------------------------
// Temporary development session.
// TODO: replace with a login screen (port of MainWindow::on_login_btn_clicked) that looks the
// user up in the ACCOUNTS table. Until then the app opens this existing account.
// ---------------------------------------------------------------------------------------------
static const std::string kDevUsername = "temp";
static constexpr int kDevUserId = 14;

// Global Variables:
HINSTANCE hInst;                                // current instance
WCHAR szTitle[MAX_LOADSTRING];                  // The title bar text
WCHAR szWindowClass[MAX_LOADSTRING];            // the main window class name

std::unique_ptr<DisplayContent> g_userContent;  // core app: threads, state, database
std::wstring g_sessionStatus;                   // shown in the status line at the bottom
HFONT g_uiFont = nullptr;
PasswordGrid g_passwordGrid;                    // grid of saved passwords
HWND g_mainWindow = nullptr;

// Search state. Empty term = show every password.
std::wstring g_searchText;                      // what the grid is currently filtered by
std::string g_searchTermUtf8;                   // same, as UTF-8 for the core

// Forward declarations of functions included in this code module:
ATOM                MyRegisterClass(HINSTANCE hInstance);
BOOL                InitInstance(HINSTANCE, int);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    About(HWND, UINT, WPARAM, LPARAM);

//
//  FUNCTION: FindDatabasePath()
//
//  PURPOSE: Locates user_accounts.db.
//
//  1. The DB_PATH environment variable, if set (same as the Qt version / .env file).
//  2. Otherwise walk up from the .exe's folder looking for db\user_accounts.db, which finds
//     the repo's database for both x64\Debug and x64\Release builds.
//
//  Returns an empty path if the file can't be found. (sqlite3_open would silently create an
//  empty database at a wrong path, so we check first.)
//
static std::filesystem::path FindDatabasePath()
{
    namespace fs = std::filesystem;
    std::error_code ec;

    wchar_t envValue[MAX_PATH * 2] = {};
    DWORD envLength = GetEnvironmentVariableW(L"DB_PATH", envValue, ARRAYSIZE(envValue));
    if (envLength > 0 && envLength < ARRAYSIZE(envValue))
    {
        fs::path envPath(envValue);
        if (fs::is_regular_file(envPath, ec))
        {
            return envPath;
        }
    }

    wchar_t exePath[MAX_PATH] = {};
    GetModuleFileNameW(nullptr, exePath, ARRAYSIZE(exePath));
    fs::path dir = fs::path(exePath).parent_path();
    for (int depth = 0; depth < 6 && !dir.empty(); ++depth)
    {
        fs::path candidate = dir / L"db" / L"user_accounts.db";
        if (fs::is_regular_file(candidate, ec))
        {
            return candidate;
        }
        if (dir == dir.parent_path())
        {
            break;
        }
        dir = dir.parent_path();
    }
    return fs::path();
}

//
//  FUNCTION: StartUserSession()
//
//  PURPOSE: Creates the core DisplayContent object, which loads the user's passwords and starts
//           the operation, search, and data-store threads.
//
static void StartUserSession()
{
    std::filesystem::path dbPath = FindDatabasePath();
    if (dbPath.empty())
    {
        g_sessionStatus = L"Database not found. Set DB_PATH or keep db\\user_accounts.db in the repo.";
        return;
    }

    std::string dbPathUtf8 = WideToUtf8(dbPath.wstring()); // sqlite3_open expects UTF-8
    g_userContent = std::make_unique<DisplayContent>(kDevUsername, kDevUserId, &dbPathUtf8);
    g_sessionStatus = L"Signed in as \"" + Utf8ToWide(kDevUsername) + L"\"   |   " + dbPath.wstring();
}

static HFONT CreateUiFont()
{
    NONCLIENTMETRICSW metrics = {};
    metrics.cbSize = sizeof(metrics);
    if (SystemParametersInfoW(SPI_GETNONCLIENTMETRICS, sizeof(metrics), &metrics, 0))
    {
        return CreateFontIndirectW(&metrics.lfMessageFont);
    }
    return nullptr;
}

static bool SearchActive()
{
    return !g_searchTermUtf8.empty();
}

// Status line: session info normally, match count while searching.
static void UpdateStatusText()
{
    std::wstring text = g_sessionStatus;
    if (SearchActive())
    {
        size_t count = g_passwordGrid.Count();
        text = (count == 0 ? std::wstring(L"No passwords match")
                           : std::to_wstring(count) + (count == 1 ? L" password matches" : L" passwords match"))
             + L" \"" + g_searchText + L"\"";
    }
    SetDlgItemTextW(g_mainWindow, IDC_SESSION_STATUS, text.c_str());
}

// Pulls the latest passwords from the core and updates the grid.
//
// While searching, this asks the core's search thread to (re)run the current term and shows the
// results of the last completed search. Re-running each tick keeps results current after adds,
// edits and deletes; the IDT_REFRESH_GRID_SOON timer picks up a new term's results quickly.
static void RefreshPasswordGrid()
{
    if (!g_userContent)
    {
        return;
    }
    if (SearchActive())
    {
        g_userContent->search_event(g_searchTermUtf8);
        g_passwordGrid.Refresh(g_userContent->get_search_result());
    }
    else
    {
        g_userContent->reset_display_list();
        g_passwordGrid.Refresh(g_userContent->get_display_list());
    }
    UpdateStatusText();
}

// Applies whatever is in the search box (called after the user pauses typing).
static void ApplySearchText(HWND hWnd)
{
    KillTimer(hWnd, IDT_SEARCH_DEBOUNCE);

    HWND hSearch = GetDlgItem(hWnd, IDC_SEARCH_BOX);
    int length = GetWindowTextLengthW(hSearch);
    std::wstring text(length + 1, L'\0');
    GetWindowTextW(hSearch, text.data(), length + 1);
    text.resize(length);

    // Ignore leading/trailing spaces.
    size_t first = text.find_first_not_of(L' ');
    size_t last = text.find_last_not_of(L' ');
    text = (first == std::wstring::npos) ? std::wstring() : text.substr(first, last - first + 1);

    if (text == g_searchText)
    {
        return;
    }
    g_searchText = text;
    g_searchTermUtf8 = WideToUtf8(text);

    if (SearchActive())
    {
        // Ask the search thread now; show its results once it has finished.
        g_userContent->search_event(g_searchTermUtf8);
        SetTimer(hWnd, IDT_REFRESH_GRID_SOON, kRefreshSoonMs, nullptr);
    }
    else
    {
        RefreshPasswordGrid(); // back to every password, no waiting needed
    }
}

// Search box keyboard shortcuts: Enter searches immediately, Esc clears.
static LRESULT CALLBACK SearchBoxProc(HWND hEdit, UINT message, WPARAM wParam, LPARAM lParam,
                                      UINT_PTR subclassId, DWORD_PTR refData)
{
    UNREFERENCED_PARAMETER(refData);
    switch (message)
    {
    case WM_KEYDOWN:
        if (wParam == VK_RETURN)
        {
            ApplySearchText(GetParent(hEdit));
            return 0;
        }
        if (wParam == VK_ESCAPE)
        {
            SetWindowTextW(hEdit, L"");
            ApplySearchText(GetParent(hEdit));
            return 0;
        }
        break;
    case WM_CHAR:
        if (wParam == L'\r' || wParam == 0x1B)
        {
            return 0; // swallow so the edit control doesn't beep
        }
        break;
    case WM_NCDESTROY:
        RemoveWindowSubclass(hEdit, SearchBoxProc, subclassId);
        break;
    }
    return DefSubclassProc(hEdit, message, wParam, lParam);
}

static void CreateMainControls(HWND hWnd)
{
    g_mainWindow = hWnd;
    g_uiFont = CreateUiFont();

    HWND hAddButton = CreateWindowW(L"BUTTON", L"Add Password",
        WS_CHILD | WS_VISIBLE | WS_TABSTOP | BS_PUSHBUTTON,
        kMargin, kMargin, kButtonWidth, kButtonHeight,
        hWnd, reinterpret_cast<HMENU>(static_cast<INT_PTR>(IDC_ADD_PASSWORD_BTN)), hInst, nullptr);

    HWND hSearch = CreateWindowExW(WS_EX_CLIENTEDGE, L"EDIT", L"",
        WS_CHILD | WS_VISIBLE | WS_TABSTOP | ES_AUTOHSCROLL,
        0, 0, 0, 0,
        hWnd, reinterpret_cast<HMENU>(static_cast<INT_PTR>(IDC_SEARCH_BOX)), hInst, nullptr);
    SendMessageW(hSearch, EM_SETCUEBANNER, TRUE, reinterpret_cast<LPARAM>(L"Search by description, URL or username"));
    SendMessageW(hSearch, EM_LIMITTEXT, 256, 0);
    SetWindowSubclass(hSearch, SearchBoxProc, 1, 0);

    HWND hClear = CreateWindowW(L"BUTTON", L"Clear",
        WS_CHILD | WS_VISIBLE | WS_TABSTOP | BS_PUSHBUTTON | WS_DISABLED,
        0, 0, 0, 0,
        hWnd, reinterpret_cast<HMENU>(static_cast<INT_PTR>(IDC_SEARCH_CLEAR_BTN)), hInst, nullptr);

    HWND hStatus = CreateWindowW(L"STATIC", g_sessionStatus.c_str(),
        WS_CHILD | WS_VISIBLE | SS_LEFT | SS_PATHELLIPSIS | SS_CENTERIMAGE,
        0, 0, 0, 0,
        hWnd, reinterpret_cast<HMENU>(static_cast<INT_PTR>(IDC_SESSION_STATUS)), hInst, nullptr);

    if (g_uiFont)
    {
        SendMessageW(hAddButton, WM_SETFONT, reinterpret_cast<WPARAM>(g_uiFont), TRUE);
        SendMessageW(hSearch, WM_SETFONT, reinterpret_cast<WPARAM>(g_uiFont), TRUE);
        SendMessageW(hClear, WM_SETFONT, reinterpret_cast<WPARAM>(g_uiFont), TRUE);
        SendMessageW(hStatus, WM_SETFONT, reinterpret_cast<WPARAM>(g_uiFont), TRUE);
    }

    g_passwordGrid.Create(hWnd, hInst, IDC_PASSWORD_GRID, g_uiFont);

    // No session means nowhere to queue operations or search.
    EnableWindow(hAddButton, g_userContent != nullptr);
    EnableWindow(hSearch, g_userContent != nullptr);

    if (g_userContent)
    {
        RefreshPasswordGrid();
        SetTimer(hWnd, IDT_REFRESH_GRID, kRefreshIntervalMs, nullptr);
    }
}

// Top row:  [Add Password]  [search box ..................]  [Clear]
// Middle:   password grid (fills the window)
// Bottom:   status line
static void LayoutMainControls(HWND hWnd, int clientWidth, int clientHeight)
{
    int clearX = (std::max)(0, clientWidth - kMargin - kClearButtonWidth);
    MoveWindow(GetDlgItem(hWnd, IDC_SEARCH_CLEAR_BTN), clearX, kMargin, kClearButtonWidth, kButtonHeight, TRUE);

    int searchX = kMargin + kButtonWidth + kMargin;
    int searchY = kMargin + (kButtonHeight - kSearchHeight) / 2;
    MoveWindow(GetDlgItem(hWnd, IDC_SEARCH_BOX), searchX, searchY,
               (std::max)(0, clearX - 8 - searchX), kSearchHeight, TRUE);

    int statusY = (std::max)(0, clientHeight - kMargin / 2 - kStatusHeight);
    MoveWindow(GetDlgItem(hWnd, IDC_SESSION_STATUS), kMargin, statusY,
               (std::max)(0, clientWidth - 2 * kMargin), kStatusHeight, TRUE);

    int gridY = kMargin + kButtonHeight + kMargin;
    g_passwordGrid.SetBounds(kMargin, gridY,
                             (std::max)(0, clientWidth - 2 * kMargin),
                             (std::max)(0, statusY - 8 - gridY));
}

// Opens the View Password dialog for the password behind a grid item.
static void OpenPasswordFromGrid(HWND hWnd, const password* selected)
{
    if (selected == nullptr)
    {
        return;
    }
    password snapshot = *selected; // the grid may refresh while the dialog is open
    if (ShowViewPasswordDialog(hInst, hWnd, g_userContent.get(), snapshot) == IDOK)
    {
        SetTimer(hWnd, IDT_REFRESH_GRID_SOON, kRefreshSoonMs, nullptr);
    }
}

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
                     _In_opt_ HINSTANCE hPrevInstance,
                     _In_ LPWSTR    lpCmdLine,
                     _In_ int       nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

    INITCOMMONCONTROLSEX commonControls = { sizeof(commonControls), ICC_LISTVIEW_CLASSES | ICC_STANDARD_CLASSES };
    InitCommonControlsEx(&commonControls);

    // Start the core app before the window is created so the UI can reflect the session state.
    StartUserSession();

    // Initialize global strings
    LoadStringW(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
    LoadStringW(hInstance, IDC_vsWindowsUi, szWindowClass, MAX_LOADSTRING);
    MyRegisterClass(hInstance);

    // Perform application initialization:
    if (!InitInstance (hInstance, nCmdShow))
    {
        g_userContent.reset();
        return FALSE;
    }

    HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_vsWindowsUi));

    MSG msg;

    // Main message loop:
    while (GetMessage(&msg, nullptr, 0, 0))
    {
        if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }

    // Stops the core threads. The data-store thread does one final save before it exits,
    // so passwords added just before closing are still written to the database.
    g_userContent.reset();

    if (g_uiFont)
    {
        DeleteObject(g_uiFont);
    }

    return (int) msg.wParam;
}



//
//  FUNCTION: MyRegisterClass()
//
//  PURPOSE: Registers the window class.
//
ATOM MyRegisterClass(HINSTANCE hInstance)
{
    WNDCLASSEXW wcex;

    wcex.cbSize = sizeof(WNDCLASSEX);

    wcex.style          = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc    = WndProc;
    wcex.cbClsExtra     = 0;
    wcex.cbWndExtra     = 0;
    wcex.hInstance      = hInstance;
    wcex.hIcon          = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_vsWindowsUi));
    wcex.hCursor        = LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground  = (HBRUSH)(COLOR_WINDOW+1);
    wcex.lpszMenuName   = MAKEINTRESOURCEW(IDC_vsWindowsUi);
    wcex.lpszClassName  = szWindowClass;
    wcex.hIconSm        = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

    return RegisterClassExW(&wcex);
}

//
//   FUNCTION: InitInstance(HINSTANCE, int)
//
//   PURPOSE: Saves instance handle and creates main window
//
//   COMMENTS:
//
//        In this function, we save the instance handle in a global variable and
//        create and display the main program window.
//
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
   hInst = hInstance; // Store instance handle in our global variable

   HWND hWnd = CreateWindowW(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,
      CW_USEDEFAULT, 0, 800, 500, nullptr, nullptr, hInstance, nullptr);

   if (!hWnd)
   {
      return FALSE;
   }

   ShowWindow(hWnd, nCmdShow);
   UpdateWindow(hWnd);

   return TRUE;
}

//
//  FUNCTION: WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  PURPOSE: Processes messages for the main window.
//
//  WM_CREATE   - create the child controls
//  WM_COMMAND  - process the application menu and buttons
//  WM_NOTIFY   - clicks on the password grid
//  WM_TIMER    - refresh the password grid
//  WM_SIZE     - lay out the controls
//  WM_PAINT    - Paint the main window
//  WM_DESTROY  - post a quit message and return
//
//
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
    case WM_CREATE:
        CreateMainControls(hWnd);
        break;
    case WM_COMMAND:
        {
            int wmId = LOWORD(wParam);
            // Parse the menu selections:
            switch (wmId)
            {
            case IDC_ADD_PASSWORD_BTN:
                if (ShowAddPasswordDialog(hInst, hWnd, g_userContent.get()) == IDOK)
                {
                    SetTimer(hWnd, IDT_REFRESH_GRID_SOON, kRefreshSoonMs, nullptr);
                }
                break;
            case IDC_SEARCH_BOX:
                if (HIWORD(wParam) == EN_CHANGE)
                {
                    // Wait for a pause in typing before searching.
                    EnableWindow(GetDlgItem(hWnd, IDC_SEARCH_CLEAR_BTN),
                                 GetWindowTextLengthW(GetDlgItem(hWnd, IDC_SEARCH_BOX)) > 0);
                    SetTimer(hWnd, IDT_SEARCH_DEBOUNCE, kSearchDebounceMs, nullptr);
                }
                break;
            case IDC_SEARCH_CLEAR_BTN:
                SetDlgItemTextW(hWnd, IDC_SEARCH_BOX, L"");
                ApplySearchText(hWnd);
                SetFocus(GetDlgItem(hWnd, IDC_SEARCH_BOX));
                break;
            case IDM_ABOUT:
                DialogBox(hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);
                break;
            case IDM_EXIT:
                DestroyWindow(hWnd);
                break;
            default:
                return DefWindowProc(hWnd, message, wParam, lParam);
            }
        }
        break;
    case WM_NOTIFY:
        {
            auto* header = reinterpret_cast<LPNMHDR>(lParam);
            if (header->idFrom == IDC_PASSWORD_GRID)
            {
                switch (header->code)
                {
                case NM_CLICK: // single click on a tile
                    {
                        auto* activate = reinterpret_cast<LPNMITEMACTIVATE>(lParam);
                        OpenPasswordFromGrid(hWnd, g_passwordGrid.PasswordAtItem(activate->iItem));
                    }
                    break;
                case NM_RETURN: // Enter on the selected tile (keyboard users)
                    OpenPasswordFromGrid(hWnd, g_passwordGrid.SelectedPassword());
                    break;
                }
            }
        }
        break;
    case WM_TIMER:
        if (wParam == IDT_SEARCH_DEBOUNCE)
        {
            ApplySearchText(hWnd);
            break;
        }
        if (wParam == IDT_REFRESH_GRID_SOON)
        {
            KillTimer(hWnd, IDT_REFRESH_GRID_SOON);
        }
        if (wParam == IDT_REFRESH_GRID || wParam == IDT_REFRESH_GRID_SOON)
        {
            RefreshPasswordGrid();
        }
        break;
    case WM_SIZE:
        LayoutMainControls(hWnd, LOWORD(lParam), HIWORD(lParam));
        break;
    case WM_PAINT:
        {
            PAINTSTRUCT ps;
            BeginPaint(hWnd, &ps);
            // TODO: Add any drawing code here...
            EndPaint(hWnd, &ps);
        }
        break;
    case WM_DESTROY:
        KillTimer(hWnd, IDT_REFRESH_GRID);
        KillTimer(hWnd, IDT_REFRESH_GRID_SOON);
        KillTimer(hWnd, IDT_SEARCH_DEBOUNCE);
        PostQuitMessage(0);
        break;
    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}

// Message handler for about box.
INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    UNREFERENCED_PARAMETER(lParam);
    switch (message)
    {
    case WM_INITDIALOG:
        return (INT_PTR)TRUE;

    case WM_COMMAND:
        if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
        {
            EndDialog(hDlg, LOWORD(wParam));
            return (INT_PTR)TRUE;
        }
        break;
    }
    return (INT_PTR)FALSE;
}
