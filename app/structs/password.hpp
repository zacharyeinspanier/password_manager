#ifndef PASSWORD 
#define PASSWORD

#include <iostream>
#include <thread>
#include <mutex>
#include <ctime>

struct password{
    std::string username;
    std::string encryped_password;
    std::string description;
    std::string url;
    time_t modify_date;
    time_t date_created;
    int p_id;
    mutable std::mutex password_mutex;

    password();
    password(std::string username, std::string password, std::string description, std::string url, int p_id);
    password(const password& other); 
    ~password();

    std::string encrypt_data(std::string password);
    std::string decrypt_data();
    password& operator=(password const& other);
};

#endif