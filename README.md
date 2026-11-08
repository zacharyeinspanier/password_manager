# password_manager

A desktop password manager written in C++. It stores a user's passwords (username, password, URL and description) in a local SQLite database. The app can add, view, edit, delete and search entries. The core logic in `app/` is multi-threaded: separate threads handle user operations, search and writing changes back to the database.

## Dependencies

- **C++20 compiler.** Compiled with `-std=c++20`
- **SQLite.** SQLite Download Page
- **Windows UI:** Visual Studio 2026 
- **Qt UI:** Qt 6

## Setting `DB_PATH`

`DB_PATH` must be set to the **absolute path** of `db/user_accounts.db`. The Qt app and the unit tests both require it. The `DB_PATH` should be set as an enviroment variable.

## Running the Tests

The unit tests live in `tests/unit_tests/`:

To run them:

```
cd tests
./run_tests.sh
```