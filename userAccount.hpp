#ifndef USERACCOUNT // USERACCOUNT include gurard: app is only included once. 
#define USERACCOUNT

#include <iostream>
#include <unordered_map>
#include <thread>
#include <vector>
#include <dataTypes.cpp>


class UserAccount{
    private:
        unordered_map<string, shared_ptr<password>> username_map;
        unordered_map<string, shared_ptr<password>> descrioption_map;
        mutex password_mutex;
        string account_username;
        int user_id;

        string encrypt_data(string password);
        string decrypt_data(string password);
    public:

        UserAccount(string username, int user_id);
        UserAccount(string username, int user_id, vector<password> user_data);
        ~UserAccount();

        void add_password();
        void remove_password();
        void modify_password();

        unordered_map<string, shared_ptr<password>> * get_read_map_username();
        unordered_map<string, shared_ptr<password>> * get_read_map_description();
};

#endif