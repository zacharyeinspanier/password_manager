# password_manager

A desktop password manager written in C++. It stores a user's passwords (username, password, URL and description) in a local SQLite database. The app can add, view, edit, delete and search entries. The core logic in `app/` is multi-threaded: separate threads handle user operations, search and writing changes back to the database.

## Dependencies

- **C++20 compiler.** The project is compiled with `-std=c++20` (`/std:c++20` in Visual Studio).
- **SQLite.** Bundled in `sqlite/` and compiled into both GUI builds, so you don't need to install it separately.
- **For the Windows UI:** Visual Studio 2026 with the **Desktop development with C++** workload (MSVC v145 toolset and a Windows 10/11 SDK).
- **For the Qt UI:** Qt 6 (built with Qt 6.12, MinGW 64-bit kit) with the **Widgets**, **Sql** and **Linguist Tools** modules, plus Qt Creator and CMake 3.16 or newer.
- **For the unit tests:** `g++` and a system SQLite library (`-lsqlite3`), run from a bash shell (macOS/Linux, or MSYS2 on Windows).

## Setting `DB_PATH`

`DB_PATH` must be set to the **absolute path** of `db/user_accounts.db`. The Qt app and the unit tests both require it. The `DB_PATH` should be set as an enviroment variable.

## Running the Tests

The unit tests live in `tests/unit_tests/`:

To run them:

```
cd tests
./run_tests.sh
```