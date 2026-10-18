#ifndef USERACCOUNT // USERACCOUNT include gurard: userAccount is only included once.
#define USERACCOUNT

#include <iostream>
#include <string>
#include <unordered_map>
#include <thread>
#include <vector>
#include <mutex>
#include <memory>
#include <chrono>
#include <set>
#include <sqlite3.h>
#include "../structs/operations.hpp"
#include "../structs/password.hpp"
#include "../encryption/encryption.hpp"

class UserAccount
{
private:
    static std::mutex user_data_mutex;
    static std::vector<password> initial_user_data;
    static int max_password_id;
    std::unordered_map<int, std::shared_ptr<password>> pass_id_map;
    std::unordered_map<std::string, std::vector<std::shared_ptr<password>>> url_map;
    std::unordered_map<std::string, std::vector<std::shared_ptr<password>>> username_map;
    std::unordered_map<std::string, std::vector<std::shared_ptr<password>>> description_map;
    std::vector<int> removed_ids;
    std::chrono::system_clock::duration time_since_epoch;
    std::mutex unordered_map_mutex;
    std::mutex removed_ids_mutex;
    std::string account_username;
    std::string db_path;
    int user_id;
    int current_password_id;

    // Encrypts passwords before they are stored (in memory and in the database) and decrypts
    // them before they are handed to the GUI. Key: "<db_path>.key" (see Encryption::from_key_file).
    Encryption cipher;

    void get_user_data();
    void get_password_id();
    static int sql_callback_user_data(void *data, int argc, char **argv, char **azColName);
    static int sql_callback_password_id(void *data, int argc, char **argv, char **azColName);

    // Returns the stored form of a password: values that are already encrypted with this key
    // (loaded from the database) are kept as-is; anything else (new or legacy plain-text
    // passwords) is encrypted.
    std::string encrypt_for_storage(const std::string &value) const;
    // Copy of a stored password with encryped_password decrypted for display.
    password decrypted_copy(const password &stored) const;

public:
    UserAccount(std::string const username, const int user_id, std::string *db_path);
    ~UserAccount();

    void add_password(const password *new_password);
    void remove_password(const int p_id);
    void modify_password(const password *updated_password);
    // Results have their passwords decrypted (for the GUI).
    std::unordered_map<int, password> search(std::string search_term);
    // Passwords decrypted (for the GUI).
    std::unordered_map<int, password> get_data_copy();
    // Passwords still encrypted (for writing to the database).
    std::unordered_map<int, password> get_encrypted_data_copy();
    std::vector<int> get_removed_ids();
    bool contains_password(const int p_id);
    int get_user_id();
};

#endif