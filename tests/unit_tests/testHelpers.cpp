#include <cassert>
#include <iostream>
#include <sqlite3.h>
#include <chrono>
#include "../../app/objects/userAccount.cpp"
#include "../../app/structs/password.cpp"
#include "../../app/structs/operations.cpp"

std::string db_path;
int user_id = 1;
std::chrono::time_point<std::chrono::system_clock> now = std::chrono::system_clock::now();

void generate_passwords(int number)
{
    sqlite3 *db;
    int rc;
    const char *path = &db_path[0];
    rc = sqlite3_open(path, &db);
    if (rc != SQLITE_OK)
    {
        std::cerr << "Error opening sql db" << std::endl;
    }
    auto duration = now.time_since_epoch();
    auto millis = std::chrono::duration_cast<std::chrono::milliseconds>(duration).count();
    // Here we will need to insert into the data base.
    for (int i = 0; i < number; ++i)
    {
        std::string username = "username_" + std::to_string(i);
        std::string password = "password_" + std::to_string(i);
        std::string description = "description_" + std::to_string(i);
        std::string url = "https://" + std::to_string(i);
        std::string date_created = std::to_string(millis);
        std::string date_modifed = std::to_string(millis);
        std::string password_id = std::to_string(i);

        char sql_insert[500];
        snprintf(
            sql_insert,
            sizeof(sql_insert),
            "INSERT OR REPLACE INTO USER_DATA (USERNAME, PASSWORD, DESCRIPTION, URL, DATE_CREATED, DATE_MODIFIED, PASSWORD_ID, USERID) VALUES('%s', '%s', '%s','%s', %s, %s, %s, %s);",
            username.c_str(),
            password.c_str(),
            description.c_str(),
            url.c_str(),
            date_created.c_str(),
            date_modifed.c_str(),
            password_id.c_str(),
            std::to_string(user_id).c_str());
        int rc_exec;
        char *messaggeError;
        rc_exec = sqlite3_exec(db, sql_insert, NULL, 0, &messaggeError);
        if (rc_exec != SQLITE_OK)
        {
            std::cerr << "Error Insert generate passwords" << std::endl;
            std::cout << messaggeError << std::endl;
            sqlite3_free(messaggeError);
        }
    }
}

UserAccount *get_user_account(std::string username, int user_id, int number)
{
    generate_passwords(number);
    return UserAccount::initialize_instance(username, user_id, &db_path);
}

void clean_up_database()
{
    sqlite3 *db;
    int rc;
    const char *path = &db_path[0];
    rc = sqlite3_open(path, &db);
    if (rc != SQLITE_OK)
    {
        std::cerr << "Error opening sql db" << std::endl;
    }

    char sql_insert[500];
    snprintf(sql_insert, sizeof(sql_insert), "DELETE FROM USER_DATA WHERE USERID = %s", std::to_string(user_id).c_str());
    int rc_exec;
    char *messaggeError;
    rc_exec = sqlite3_exec(db, sql_insert, NULL, 0, &messaggeError);
    if (rc_exec != SQLITE_OK)
    {
        std::cerr << "Error delete" << std::endl;
        std::cout << messaggeError << std::endl;
        sqlite3_free(messaggeError);
    }
}