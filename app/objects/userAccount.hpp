#ifndef USERACCOUNT // USERACCOUNT include gurard: app is only included once. 
#define USERACCOUNT

#include <iostream>
#include <unordered_map>
#include <thread>
#include <vector>
#include <mutex>
#include "../structs/operations.hpp"
#include "../structs/password.hpp"

class UserAccount{
    private:
        std::unordered_map<int, std::shared_ptr<password>> pass_id_map;
        std::unordered_map<std::string, std::vector<std::shared_ptr<password>>> url_map;
        std::unordered_map<std::string, std::vector<std::shared_ptr<password>>> username_map;
        std::unordered_map<std::string, std::vector<std::shared_ptr<password>>> descrioption_map;
        std::mutex unordered_map_mutex;
        std::string account_username;
        int user_id;
        
    public:
        UserAccount(std::string username, int user_id);
        UserAccount(std::string username, int user_id, std::vector<password> user_data);
        ~UserAccount();

        void add_password(password * new_password);
        void remove_password(int p_id);
        void modify_password(int p_id, std::string new_value, modifyType modify_type);
        std::string view_password(int p_id);
        bool contains_password(int p_id);

        std::unordered_map<int, password> get_data_copy();
};

#endif