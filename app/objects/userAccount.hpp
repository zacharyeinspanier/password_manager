#ifndef USERACCOUNT // USERACCOUNT include gurard: userAccount is only included once. 
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
        std::unordered_map<int, std::shared_ptr<password> > pass_id_map;
        std::unordered_map<std::string, std::vector<std::shared_ptr<password> > > url_map;
        std::unordered_map<std::string, std::vector<std::shared_ptr<password> > > username_map;
        std::unordered_map<std::string, std::vector<std::shared_ptr<password> > > descrioption_map;
        std::mutex unordered_map_mutex;
        std::string account_username;
        int user_id;
        
    public:
        UserAccount(std::string const username, const int user_id);
        UserAccount(std::string const username, const int user_id, const std::vector<password> * user_data);
        ~UserAccount();

        void add_password(const password * new_password);
        void remove_password(const int p_id);
        void modify_password(const int p_id, const std::string new_value, const modifyType modify_type);
        std::string view_password(const int p_id);
        bool contains_password(const int p_id);

        std::unordered_map<int, password> get_data_copy();
};

#endif