#ifndef USERACCOUNT // USERACCOUNT include gurard: userAccount is only included once. 
#define USERACCOUNT

#include <iostream>
#include <unordered_map>
#include <thread>
#include <vector>
#include <mutex>
#include <set>
#include "../structs/operations.hpp"
#include "../structs/password.hpp"

////////////////////////////////////////////////////////////////////////////////////////////////
/// @brief This class holds user account passwords and member functions for mutating data
///
/// This class is a singleton. There is only one user account active at a time.
/// 
/// Useage:
/// It is intended to call the static function  UserAccount::initialize_instance() before
/// any calls to UserAccount::get_instance()
////////////////////////////////////////////////////////////////////////////////////////////////
class UserAccount{
    private:

        static UserAccount * instance_ptr;
        static std::mutex user_acc_mutex;
        std::unordered_map<int, std::shared_ptr<password> > pass_id_map;
        std::unordered_map<std::string, std::vector<std::shared_ptr<password> > > url_map;
        std::unordered_map<std::string, std::vector<std::shared_ptr<password> > > username_map;
        std::unordered_map<std::string, std::vector<std::shared_ptr<password> > > descrioption_map;
        std::mutex unordered_map_mutex;
        std::string account_username;
        int user_id;

        UserAccount(std::string const username, const int user_id);
        UserAccount(std::string const username, const int user_id, const std::vector<password> * user_data);
        ~UserAccount();
        
    public:
        
        // Singleton
        UserAccount(const UserAccount& obj) = delete;
        static UserAccount* initialize_instance(std::string const username, const int user_id, const std::vector<password> * user_data);
        static UserAccount* get_instance();

        // Password Operations
        void add_password(const password * new_password);
        void remove_password(const int p_id);
        void modify_password(const int p_id, const std::string new_value, const modifyType modify_type);
        std::string view_password(const int p_id);
        bool contains_password(const int p_id);
        void search(std::string search_term, std::set<std::shared_ptr<password>> * search_result);

        std::unordered_map<int, password> get_data_copy();
        int get_user_id();
};

#endif