// vsWindowsUi.cpp : Defines the entry point for the application.
//
// The main window has two pages:
//   Login page          - shown first. Username, Password, Login, Create Account.
//   Password manager    - shown after a successful login. Add Password, search, password grid,
//                         status line and Logout (bottom right).
//
// A user session (the core's DisplayContent: threads, state, database) only exists while the
// password manager page is shown. Logout ends the session and returns to the login page.

#include "framework.h"
#include "vsWindowsUi.h"
#include "AccountStore.h"
#include "AddPasswordDialog.h"
#include "CreateAccountDialog.h"
#include "LoginPage.h"
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

// Layout of the password manager page (pixels).
static constexpr int kMargin = 16;
static constexpr int kButtonWidth = 140;
static constexpr int kButtonHeight = 32;
static constexpr int kSearchHeight = 26;
static constexpr int kClearButtonWidth = 70;
static constexpr int kLogoutButtonWidth = 100;

// Controls that belong to the password manager page (the grid is handled separately).
static constexpr int kMainPageControlIds[] = {
    IDC_ADD_PASSWORD_BTN, IDC_SEARCH_BOX, IDC_SEARCH_CLEAR_BTN, IDC_SESSION_STATUS, IDC_LOGOUT_BTN,
};

// Global Variables:
HINSTANCE hInst;                                // current instance
WCHAR szTitle[MAX_LOADSTRING];                  // The title bar text
WCHAR szWindowClass[MAX_LOADSTRING];            // the main window class name

std::filesystem::path g_dbPath;                 // user_accounts.db (empty if not found)
std::string g_dbPathUtf8;                       // same, as UTF-8 for SQLite and the core

std::unique_ptr<DisplayContent> g_userContent;  // core app for the logged-in user (null when logged out)
std::wstring g_signedInUser;
std::wstring g_sessionStatus;                   // shown in the status line at the bottom
HFONT g_uiFont = nullptr;
HFONT g_titleFont = nullptr;
PasswordGrid g_passwordGrid;                    // grid of saved passwords
HWND g_mainWindow = nullptr;
HWND g_lastFocus = nullptr;                     // restored when the window is re-activated

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

static HFONT CreateUiFont(int heightScalePercent = 100, int weight = 0)
{
    NONCLIENTMETRICSW metrics = {};
    metrics.cbSize = sizeof(metrics);
    if (!SystemParametersInfoW(SPI_GETNONCLIENTMETRICS, sizeof(metrics), &metrics, 0))
    {
        return nullptr;
    }
    LOGFONTW font = metrics.lfMessageFont;
    font.lfHeight = font.lfHeight * heightScalePercent / 100;
    if (weight != 0)
    {
        font.lfWeight = weight;
    }
    return CreateFontIndirectW(&font);
}

// ---------------------------------------------------------------------------------------------
// Password manager page
// ---------------------------------------------------------------------------------------------

static bool SearchActive()
{
    return !g_searchTermUtf8.empty();
}

// Status line: who is signed in normally, match count while searching.
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

// Applies whatever is in the search box (called after the user pauses typing, or on Enter).
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
        if (g_userContent)
        {
            g_userContent->search_event(g_searchTermUtf8);
        }
        SetTimer(hWnd, IDT_REFRESH_GRID_SOON, kRefreshSoonMs, nullptr);
    }
    else
    {
        RefreshPasswordGrid(); // back to every password, no waiting needed
    }
}

static void ClearSearch(HWND hWnd)
{
    SetDlgItemTextW(hWnd, IDC_SEARCH_BOX, L"");
    ApplySearchText(hWnd);
}

static void CreateMainPageControls(HWND hWnd)
{
    auto make = [hWnd](DWORD exStyle, const wchar_t* className, const wchar_t* text, DWORD style, int id) {
        HWND hControl = CreateWindowExW(exStyle, className, text, WS_CHILD | style, 0, 0, 0, 0, hWnd,
                                        reinterpret_cast<HMENU>(static_cast<INT_PTR>(id)), hInst, nullptr);
        if (hControl && g_uiFont)
        {
            SendMessageW(hControl, WM_SETFONT, reinterpret_cast<WPARAM>(g_uiFont), TRUE);
        }
        return hControl;
    };

    // Created in tab order.
    make(0, L"BUTTON", L"Add Password", WS_TABSTOP | BS_PUSHBUTTON, IDC_ADD_PASSWORD_BTN);
    HWND hSearch = make(WS_EX_CLIENTEDGE, L"EDIT", L"", WS_TABSTOP | ES_AUTOHSCROLL, IDC_SEARCH_BOX);
    SendMessageW(hSearch, EM_SETCUEBANNER, TRUE, reinterpret_cast<LPARAM>(L"Search by description, URL or username"));
    SendMessageW(hSearch, EM_LIMITTEXT, 256, 0);
    make(0, L"BUTTON", L"Clear", WS_TABSTOP | BS_PUSHBUTTON | WS_DISABLED, IDC_SEARCH_CLEAR_BTN);

    g_passwordGrid.Create(hWnd, hInst, IDC_PASSWORD_GRID, g_uiFont);

    make(0, L"STATIC", L"", SS_LEFT | SS_PATHELLIPSIS | SS_CENTERIMAGE, IDC_SESSION_STATUS);
    make(0, L"BUTTON", L"Logout", WS_TABSTOP | BS_PUSHBUTTON, IDC_LOGOUT_BTN);
}

// Top row:  [Add Password]  [search box ..................]  [Clear]
// Middle:   password grid (fills the window)
// Bottom:   status line .............................. [Logout]
static void LayoutMainPage(HWND hWnd, int clientWidth, int clientHeight)
{
    int clearX = (std::max)(0, clientWidth - kMargin - kClearButtonWidth);
    MoveWindow(GetDlgItem(hWnd, IDC_ADD_PASSWORD_BTN), kMargin, kMargin, kButtonWidth, kButtonHeight, TRUE);
    MoveWindow(GetDlgItem(hWnd, IDC_SEARCH_CLEAR_BTN), clearX, kMargin, kClearButtonWidth, kButtonHeight, TRUE);

    int searchX = kMargin + kButtonWidth + kMargin;
    int searchY = kMargin + (kButtonHeight - kSearchHeight) / 2;
    MoveWindow(GetDlgItem(hWnd, IDC_SEARCH_BOX), searchX, searchY,
               (std::max)(0, clearX - 8 - searchX), kSearchHeight, TRUE);

    int bottomY = (std::max)(0, clientHeight - kMargin - kButtonHeight);
    int logoutX = (std::max)(0, clientWidth - kMargin - kLogoutButtonWidth);
    MoveWindow(GetDlgItem(hWnd, IDC_LOGOUT_BTN), logoutX, bottomY, kLogoutButtonWidth, kButtonHeight, TRUE);
    MoveWindow(GetDlgItem(hWnd, IDC_SESSION_STATUS), kMargin, bottomY,
               (std::max)(0, logoutX - 2 * kMargin), kButtonHeight, TRUE);

    int gridY = kMargin + kButtonHeight + kMargin;
    g_passwordGrid.SetBounds(kMargin, gridY,
                             (std::max)(0, clientWidth - 2 * kMargin),
                             (std::max)(0, bottomY - 12 - gridY));
}

static void ShowMainPage(HWND hWnd, bool show)
{
    for (int id : kMainPageControlIds)
    {
        ShowWindow(GetDlgItem(hWnd, id), show ? SW_SHOW : SW_HIDE);
    }
    ShowWindow(g_passwordGrid.Handle(), show ? SW_SHOW : SW_HIDE);
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

// ---------------------------------------------------------------------------------------------
// Switching pages / sessions
// ---------------------------------------------------------------------------------------------

static void ShowLoginPage(HWND hWnd, const std::wstring& username = std::wstring())
{
    ShowMainPage(hWnd, false);
    LoginPage::Show(hWnd, true);
    LoginPage::Reset(hWnd, username);
}

// Creates the core DisplayContent for the user (loads their passwords and starts the operation,
// search and data-store threads), then shows the password manager page.
static void StartSession(HWND hWnd, const std::string& username, int userId)
{
    // Opening the account loads the encryption key file next to the database (created on first
    // use); if it can't be read the core throws, so stay on the login page and say why.
    std::string dbPath = g_dbPathUtf8;
    try
    {
        g_userContent = std::make_unique<DisplayContent>(username, userId, &dbPath);
    }
    catch (const std::exception& error)
    {
        std::wstring message = L"Your passwords could not be opened:\n\n" + Utf8ToWide(error.what());
        MessageBoxW(hWnd, message.c_str(), L"Login", MB_OK | MB_ICONERROR);
        return;
    }
    g_signedInUser = Utf8ToWide(username);
    g_sessionStatus = L"Signed in as \"" + g_signedInUser + L"\"";

    // Fresh search state for every session.
    SetDlgItemTextW(hWnd, IDC_SEARCH_BOX, L"");
    KillTimer(hWnd, IDT_SEARCH_DEBOUNCE);
    g_searchText.clear();
    g_searchTermUtf8.clear();
    EnableWindow(GetDlgItem(hWnd, IDC_SEARCH_CLEAR_BTN), FALSE);

    LoginPage::Show(hWnd, false);
    LoginPage::SetMessage(hWnd, L"", false);
    ShowMainPage(hWnd, true);

    RefreshPasswordGrid();
    SetTimer(hWnd, IDT_REFRESH_GRID, kRefreshIntervalMs, nullptr);
    SetFocus(GetDlgItem(hWnd, IDC_SEARCH_BOX));
}

// Ends the session and returns to the login page.
static void Logout(HWND hWnd)
{
    KillTimer(hWnd, IDT_REFRESH_GRID);
    KillTimer(hWnd, IDT_REFRESH_GRID_SOON);
    KillTimer(hWnd, IDT_SEARCH_DEBOUNCE);

    // Stopping the core joins its threads; the data-store thread finishes its current wait
    // (up to ~2 seconds) and saves one last time, so show a wait cursor meanwhile.
    HCURSOR previousCursor = SetCursor(LoadCursor(nullptr, IDC_WAIT));
    g_userContent.reset();
    SetCursor(previousCursor);

    g_passwordGrid.Refresh({});
    g_signedInUser.clear();
    g_sessionStatus.clear();
    g_searchText.clear();
    g_searchTermUtf8.clear();
    SetDlgItemTextW(hWnd, IDC_SEARCH_BOX, L"");
    KillTimer(hWnd, IDT_SEARCH_DEBOUNCE); // clearing the box above restarts it

    ShowLoginPage(hWnd);
}

static void TryLogin(HWND hWnd)
{
    std::wstring username = LoginPage::Username(hWnd);
    std::wstring passwordText = LoginPage::Password(hWnd);

    if (username.empty() || passwordText.empty())
    {
        LoginPage::SetMessage(hWnd, L"Please enter your username and password.", true);
        SetFocus(GetDlgItem(hWnd, username.empty() ? IDC_LOGIN_USERNAME : IDC_LOGIN_PASSWORD));
        return;
    }

    int userId = -1;
    std::string storedUsername;
    std::wstring error;
    LoginResult result = AuthenticateUser(g_dbPathUtf8, WideToUtf8(username), WideToUtf8(passwordText),
                                          userId, storedUsername, error);
    SecureZeroMemory(passwordText.data(), passwordText.size() * sizeof(wchar_t));

    switch (result)
    {
    case LoginResult::Success:
        LoginPage::ClearPassword(hWnd);
        StartSession(hWnd, storedUsername, userId);
        break;
    case LoginResult::InvalidCredentials:
        LoginPage::ClearPassword(hWnd);
        LoginPage::SetMessage(hWnd, L"Incorrect username or password.", true);
        LoginPage::FocusPassword(hWnd);
        break;
    default:
        MessageBoxW(hWnd, error.c_str(), L"Login", MB_OK | MB_ICONERROR);
        break;
    }
}

static void OpenCreateAccount(HWND hWnd)
{
    std::wstring createdUsername;
    if (ShowCreateAccountDialog(hInst, hWnd, g_dbPathUtf8, createdUsername))
    {
        LoginPage::Reset(hWnd, createdUsername);
        LoginPage::SetMessage(hWnd, L"Account created. Enter your password to log in.", false);
    }
}

static void CreateMainControls(HWND hWnd)
{
    g_mainWindow = hWnd;
    g_uiFont = CreateUiFont();
    g_titleFont = CreateUiFont(220, FW_SEMIBOLD);

    LoginPage::Create(hWnd, hInst, g_uiFont, g_titleFont);
    CreateMainPageControls(hWnd);

    ShowLoginPage(hWnd);
    if (g_dbPath.empty())
    {
        LoginPage::EnableActions(hWnd, false);
        LoginPage::SetMessage(hWnd, L"Database not found. Set DB_PATH to the full path of db\\user_accounts.db.", true);
    }
}

// Enter and Esc arrive as IDOK / IDCANCEL (IsDialogMessage in the message loop), so route them
// to whatever makes sense for the page and the focused control.
static void OnEnterKey(HWND hWnd)
{
    if (LoginPage::IsShown(hWnd))
    {
        if (IsWindowEnabled(GetDlgItem(hWnd, IDC_LOGIN_BTN)))
        {
            TryLogin(hWnd);
        }
        return;
    }
    HWND hFocus = GetFocus();
    if (hFocus == GetDlgItem(hWnd, IDC_SEARCH_BOX))
    {
        ApplySearchText(hWnd);
    }
    else if (hFocus == g_passwordGrid.Handle())
    {
        OpenPasswordFromGrid(hWnd, g_passwordGrid.SelectedPassword());
    }
}

static void OnEscapeKey(HWND hWnd)
{
    if (!LoginPage::IsShown(hWnd) && GetFocus() == GetDlgItem(hWnd, IDC_SEARCH_BOX))
    {
        ClearSearch(hWnd);
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

    g_dbPath = FindDatabasePath();
    g_dbPathUtf8 = WideToUtf8(g_dbPath.wstring()); // SQLite expects UTF-8 paths

    // Initialize global strings
    LoadStringW(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
    LoadStringW(hInstance, IDC_vsWindowsUi, szWindowClass, MAX_LOADSTRING);
    MyRegisterClass(hInstance);

    // Perform application initialization:
    if (!InitInstance (hInstance, nCmdShow))
    {
        return FALSE;
    }

    HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_vsWindowsUi));

    MSG msg;

    // Main message loop. IsDialogMessage gives the main window dialog-style keyboard handling:
    // Tab / Shift+Tab move between controls, Enter sends IDOK and Esc sends IDCANCEL.
    while (GetMessage(&msg, nullptr, 0, 0))
    {
        if (!TranslateAccelerator(g_mainWindow, hAccelTable, &msg) &&
            !(g_mainWindow && IsDialogMessage(g_mainWindow, &msg)))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }

    // Stops the core threads if the window was closed while logged in. The data-store thread
    // does one final save before it exits, so recent changes are still written to the database.
    g_userContent.reset();

    if (g_uiFont)
    {
        DeleteObject(g_uiFont);
    }
    if (g_titleFont)
    {
        DeleteObject(g_titleFont);
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
      CW_USEDEFAULT, 0, 800, 560, nullptr, nullptr, hInstance, nullptr);

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
//  WM_CREATE          - create the login and password manager controls
//  WM_COMMAND         - process the application menu and buttons
//  WM_NOTIFY          - clicks on the password grid
//  WM_TIMER           - refresh the password grid / run a search
//  WM_SIZE            - lay out the controls
//  WM_CTLCOLORSTATIC  - label colours
//  WM_ACTIVATE        - remember / restore keyboard focus
//  WM_PAINT           - Paint the main window
//  WM_DESTROY         - post a quit message and return
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
            case IDOK:
                OnEnterKey(hWnd);
                break;
            case IDCANCEL:
                OnEscapeKey(hWnd);
                break;
            case IDC_LOGIN_BTN:
                TryLogin(hWnd);
                break;
            case IDC_CREATE_ACCOUNT_BTN:
                OpenCreateAccount(hWnd);
                break;
            case IDC_LOGIN_USERNAME:
            case IDC_LOGIN_PASSWORD:
                if (HIWORD(wParam) == EN_CHANGE)
                {
                    LoginPage::SetMessage(hWnd, L"", false); // typing clears the last error
                }
                break;
            case IDC_LOGOUT_BTN:
                Logout(hWnd);
                break;
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
                    if (g_userContent)
                    {
                        SetTimer(hWnd, IDT_SEARCH_DEBOUNCE, kSearchDebounceMs, nullptr);
                    }
                }
                break;
            case IDC_SEARCH_CLEAR_BTN:
                ClearSearch(hWnd);
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
        LoginPage::Layout(hWnd, LOWORD(lParam), HIWORD(lParam));
        LayoutMainPage(hWnd, LOWORD(lParam), HIWORD(lParam));
        break;
    case WM_CTLCOLORSTATIC:
        if (HBRUSH brush = LoginPage::OnCtlColorStatic(reinterpret_cast<HDC>(wParam), reinterpret_cast<HWND>(lParam)))
        {
            return reinterpret_cast<LRESULT>(brush);
        }
        return DefWindowProc(hWnd, message, wParam, lParam);
    case WM_ACTIVATE:
        // A plain window doesn't restore focus to its last control on its own (dialogs do).
        if (LOWORD(wParam) == WA_INACTIVE)
        {
            g_lastFocus = GetFocus();
        }
        else if (g_lastFocus && IsChild(hWnd, g_lastFocus) && IsWindowVisible(g_lastFocus))
        {
            SetFocus(g_lastFocus);
            return 0;
        }
        return DefWindowProc(hWnd, message, wParam, lParam);
    case WM_PAINT:
        {
            PAINTSTRUCT ps;
            BeginPaint(hWnd, &ps);
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
