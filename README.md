# password_manager

A desktop password manager written in C++. It stores each user's saved logins (username, password, URL and description) in a local SQLite database. The app can add, view, edit, delete and search entries. The core logic in `app/` is multi-threaded: separate threads handle user operations, search and writing changes back to the database.

The core is shared by two desktop front ends:

- **`gui/vsWindowsUi`**: a native Windows (Win32) app built with Visual Studio.
- **`gui/qtCreatorUi`**: a cross-platform Qt Widgets app built with Qt Creator and CMake.

## Project Layout

```
app/        Core logic: user accounts, display content, password/operation structs, worker threads
db/         SQLite database (user_accounts.db)
gui/        Front ends: vsWindowsUi (Visual Studio) and qtCreatorUi (Qt Creator)
sqlite/     Bundled SQLite amalgamation (sqlite3.c / sqlite3.h)
tests/      Unit tests and the run_tests.sh script
```

## Dependencies

- **C++20 compiler.** The project is compiled with `-std=c++20` (`/std:c++20` in Visual Studio).
- **SQLite.** Bundled in `sqlite/` and compiled into both GUI builds, so you don't need to install it separately.
- **For the Windows UI:** Visual Studio 2026 with the **Desktop development with C++** workload (MSVC v145 toolset and a Windows 10/11 SDK).
- **For the Qt UI:** Qt 6 (built with Qt 6.12, MinGW 64-bit kit) with the **Widgets**, **Sql** and **Linguist Tools** modules, plus Qt Creator and CMake 3.16 or newer.
- **For the unit tests:** `g++` and a system SQLite library (`-lsqlite3`), run from a bash shell (macOS/Linux, or MSYS2 on Windows).

## Setting `DB_PATH`

`DB_PATH` must be set to the **absolute path** of `db/user_accounts.db`. The Qt app and the unit tests both require it. The Windows UI uses it if it's set; otherwise it looks for `db\user_accounts.db` in the folders above the `.exe`.

Use the full path. Variables such as `%USERPROFILE%` are not expanded.

- **Windows (permanent):**
  ```
  setx DB_PATH "C:\path\to\password_manager\db\user_accounts.db"
  ```
  Restart Visual Studio or Qt Creator afterwards so they pick up the new variable.
- **macOS/Linux, or bash on Windows:** rename `.env.sample` to `.env`, set the path inside it, and then run `source .env` in the terminal you build or run from.

## Building the Windows UI (Visual Studio)

1. Open `gui/vsWindowsUi/vsWindowsUi.slnx` in Visual Studio.
2. Choose a configuration (`Debug` or `Release`) and platform (`x64`).
3. Build with **Build > Build Solution** (`Ctrl+Shift+B`).
4. Run with **Debug > Start Debugging** (`F5`). If the database isn't found, set `DB_PATH` (see above). You can also set it for this project only under **Project Properties > Debugging > Environment**.

## Building the Qt UI (Qt Creator)

1. Open `gui/qtCreatorUi/CMakeLists.txt` in Qt Creator.
2. Select a Qt 6 kit (for example, *Desktop Qt 6.12.0 MinGW 64-bit*) and click **Configure Project**.
3. Under **Projects > Build & Run > Build Environment**, add `DB_PATH` with the absolute path to `db/user_accounts.db`.
4. Build (`Ctrl+B`) and run (`Ctrl+R`).

If the project folder was moved or renamed, delete `gui/qtCreatorUi/build/` before configuring so CMake starts fresh.

## Encryption

Saved passwords are encrypted with **ChaCha20-Poly1305** (RFC 8439), implemented in `app/encryption/` with no external libraries. `UserAccount` encrypts a password when it is added or edited, keeps it encrypted in memory and in the database, and decrypts it only in the copies it hands to the GUI (`get_data_copy`, `search`).

- **Stored format:** `ENC1:` followed by base64 text. Each value has its own random nonce, so the same password never encrypts to the same text twice. The Poly1305 tag means a corrupted or modified value fails to decrypt instead of returning garbage.
- **Key file:** the 32-byte key lives in `<database>.key`, for example `db/user_accounts.db.key`. It is created automatically the first time an account is opened, and never overwritten after that.
- **Back up the key file.** Without it the saved passwords cannot be recovered. Anyone who has both the database and the key file can decrypt the passwords, so the key file is in `.gitignore`.
- **Existing databases:** plain-text passwords from before encryption are encrypted automatically the next time their account is opened, and the encrypted values are written back to the database.

## Running the Tests

The unit tests live in `tests/unit_tests/`:

- **`userAccountTests`** covers adding, removing, editing and searching passwords on a user account.
- **`displayContentTests`** covers the display layer that sits on top of the account.
- **`encryptionTests`** checks ChaCha20, Poly1305 and the combined cipher against the RFC 8439 test vectors, plus tamper detection and the key file.

To run them:

```
cd tests
./run_tests.sh
```

The script sources `../.env` to get `DB_PATH`, so create `.env` first (see **Setting `DB_PATH`**). It then compiles each test into `tests/output/` and runs it. Every test prints a `Pass` line, and the first failure stops the run with an `assert`.

> **Warning:** The tests delete and regenerate every password belonging to user ID 1 in the database that `DB_PATH` points to. Point `DB_PATH` at a copy of the database if you want to keep that data.

## Future Work

1. **Password encryption:** Passwords are now encrypted (see **Encryption**). Possible next steps: derive the key from each user's login password instead of a key file, and decrypt a password only when the user opens it rather than for the whole list.
2. **SQL injection prevention:** Login and save operations currently build SQL queries from user input, which leaves them open to SQL injection. Switch to parameterized queries so input is always treated strictly as data.
3. **Search improvements:** Make search faster and more responsive, and improve match accuracy across name, URL, and description fields.
