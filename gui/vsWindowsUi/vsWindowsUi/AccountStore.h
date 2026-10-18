#pragma once

// Account lookups in the ACCOUNTS table (login and create account).
//
// Uses parameterized SQLite statements, so usernames and passwords are always treated as data
// (no SQL injection). The database file must already exist; it is never created here.

#include <string>

enum class LoginResult
{
    Success,
    InvalidCredentials, // no account with that username and password
    DatabaseError,      // see `error`
};

enum class CreateAccountResult
{
    Created,
    UsernameTaken,
    DatabaseError,      // see `error`
};

// Checks a username/password pair. On Success, sets userId and storedUsername
// (the username exactly as stored in the database).
LoginResult AuthenticateUser(const std::string& dbPathUtf8, const std::string& username,
                             const std::string& password, int& userId, std::string& storedUsername,
                             std::wstring& error);

// Adds a new account. Fails with UsernameTaken if the username already exists.
CreateAccountResult CreateAccount(const std::string& dbPathUtf8, const std::string& username,
                                  const std::string& password, std::wstring& error);
