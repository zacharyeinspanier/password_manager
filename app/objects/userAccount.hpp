#ifndef USERACCOUNT // USERACCOUNT include gurard: userAccount is only included once.
#define USERACCOUNT

#include <iostream>
#include <string>
#include <unordered_map>
#include <thread>
#include <vector>
#include <mutex>
#include <set>
#include <sqlite3.h>
#include "../structs/operations.hpp"
#include "../structs/password.hpp"

class UserAccount
{
private:
    static std::mutex user_data_mutex;
    static std::vector<password> initial_user_data;
    std::unordered_map<int, std::shared_ptr<password>> pass_id_map;
    std::unordered_map<std::string, std::vector<std::shared_ptr<password>>> url_map;
    std::unordered_map<std::string, std::vector<std::shared_ptr<password>>> username_map;
    std::unordered_map<std::string, std::vector<std::shared_ptr<password>>> description_map;
    std::chrono::system_clock::duration time_since_epoch;
    std::mutex unordered_map_mutex;
    std::string account_username;
    std::string db_path;
    int user_id;
    int current_password_id;

    void get_user_data();
    static int sql_callback(void *data, int argc, char **argv, char **azColName);

public:
    UserAccount(std::string const username, const int user_id, std::string *db_path);
    ~UserAccount();

    void add_password(const password *new_password);
    void remove_password(const int p_id);
    void modify_password(const password *updated_password);
    void search(std::string search_term, std::set<std::shared_ptr<password>> *search_result);

    bool contains_password(const int p_id);
    std::unordered_map<int, password> get_data_copy();
    int get_user_id();
};

#endif