#include "AccountStore.h"
#include "StringConvert.h"

#include <sqlite3.h>

namespace
{
    // Closes the connection when it goes out of scope.
    struct Database
    {
        sqlite3* handle = nullptr;
        ~Database()
        {
            if (handle)
            {
                sqlite3_close(handle);
            }
        }
    };

    // Finalizes the statement when it goes out of scope.
    struct Statement
    {
        sqlite3_stmt* handle = nullptr;
        ~Statement()
        {
            sqlite3_finalize(handle);
        }
    };

    std::wstring DescribeError(sqlite3* db, const wchar_t* what)
    {
        std::wstring message = what;
        if (db)
        {
            message += L"\n\n" + Utf8ToWide(sqlite3_errmsg(db));
        }
        return message;
    }

    // Opens an existing database for reading and writing (never creates a new file).
    bool OpenDatabase(const std::string& dbPathUtf8, Database& db, std::wstring& error)
    {
        int rc = sqlite3_open_v2(dbPathUtf8.c_str(), &db.handle, SQLITE_OPEN_READWRITE, nullptr);
        if (rc != SQLITE_OK)
        {
            error = DescribeError(db.handle, L"The database could not be opened.");
            return false;
        }
        sqlite3_busy_timeout(db.handle, 2000); // the core's data store thread may be writing
        return true;
    }

    bool BindText(sqlite3_stmt* statement, int index, const std::string& value)
    {
        return sqlite3_bind_text(statement, index, value.c_str(), static_cast<int>(value.size()),
                                 SQLITE_TRANSIENT) == SQLITE_OK;
    }
}

LoginResult AuthenticateUser(const std::string& dbPathUtf8, const std::string& username,
                             const std::string& password, int& userId, std::string& storedUsername,
                             std::wstring& error)
{
    Database db;
    if (!OpenDatabase(dbPathUtf8, db, error))
    {
        return LoginResult::DatabaseError;
    }

    Statement query;
    const char* sql = "SELECT USERID, USERNAME FROM ACCOUNTS WHERE USERNAME = ?1 AND PASSWORD = ?2;";
    if (sqlite3_prepare_v2(db.handle, sql, -1, &query.handle, nullptr) != SQLITE_OK ||
        !BindText(query.handle, 1, username) || !BindText(query.handle, 2, password))
    {
        error = DescribeError(db.handle, L"Could not check the login.");
        return LoginResult::DatabaseError;
    }

    int rc = sqlite3_step(query.handle);
    if (rc == SQLITE_ROW)
    {
        userId = sqlite3_column_int(query.handle, 0);
        const unsigned char* name = sqlite3_column_text(query.handle, 1);
        storedUsername = name ? reinterpret_cast<const char*>(name) : username;
        return LoginResult::Success;
    }
    if (rc == SQLITE_DONE)
    {
        return LoginResult::InvalidCredentials;
    }
    error = DescribeError(db.handle, L"Could not check the login.");
    return LoginResult::DatabaseError;
}

CreateAccountResult CreateAccount(const std::string& dbPathUtf8, const std::string& username,
                                  const std::string& password, std::wstring& error)
{
    Database db;
    if (!OpenDatabase(dbPathUtf8, db, error))
    {
        return CreateAccountResult::DatabaseError;
    }

    // Check first so we can give a clear message (USERNAME is also UNIQUE in the schema).
    {
        Statement exists;
        if (sqlite3_prepare_v2(db.handle, "SELECT 1 FROM ACCOUNTS WHERE USERNAME = ?1;", -1, &exists.handle, nullptr) != SQLITE_OK ||
            !BindText(exists.handle, 1, username))
        {
            error = DescribeError(db.handle, L"Could not check whether the username is taken.");
            return CreateAccountResult::DatabaseError;
        }
        int rc = sqlite3_step(exists.handle);
        if (rc == SQLITE_ROW)
        {
            return CreateAccountResult::UsernameTaken;
        }
        if (rc != SQLITE_DONE)
        {
            error = DescribeError(db.handle, L"Could not check whether the username is taken.");
            return CreateAccountResult::DatabaseError;
        }
    }

    Statement insert;
    if (sqlite3_prepare_v2(db.handle, "INSERT INTO ACCOUNTS (USERNAME, PASSWORD) VALUES (?1, ?2);", -1, &insert.handle, nullptr) != SQLITE_OK ||
        !BindText(insert.handle, 1, username) || !BindText(insert.handle, 2, password))
    {
        error = DescribeError(db.handle, L"Could not create the account.");
        return CreateAccountResult::DatabaseError;
    }

    int rc = sqlite3_step(insert.handle);
    if (rc == SQLITE_DONE)
    {
        return CreateAccountResult::Created;
    }
    if ((rc & 0xFF) == SQLITE_CONSTRAINT)
    {
        return CreateAccountResult::UsernameTaken; // someone took it between the check and the insert
    }
    error = DescribeError(db.handle, L"Could not create the account.");
    return CreateAccountResult::DatabaseError;
}
