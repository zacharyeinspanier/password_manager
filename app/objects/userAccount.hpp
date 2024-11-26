#ifndef USERACCOUNT // USERACCOUNT include gurard: app is only included once. 
#define USERACCOUNT

#include <iostream>
#include <unordered_map>
#include <thread>
#include <vector>
#include "../structs/password.hpp"

class UserAccount{
    private:
        std::unordered_map<int, std::shared_ptr<password>> pass_id_map;
        std::unordered_map<std::string, std::shared_ptr<password>> username_map;
        std::unordered_map<std::string, std::shared_ptr<password>> descrioption_map;
        std::mutex username_map_mutex;
        std::mutex descrioption_map_mutex;
        std::string account_username;
        int user_id;

        
    public:

        UserAccount(std::string username, int user_id);
        UserAccount(std::string username, int user_id, std::vector<password> user_data);
        ~UserAccount();

        void add_password();
        void remove_password();
        void modify_password();
        std::string view_password();

        std::unordered_map<std::string, std::shared_ptr<password>> * get_read_map_username();
        std::unordered_map<std::string, std::shared_ptr<password>> * get_read_map_description();
};

#endif