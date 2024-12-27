#ifndef PASSWORD
#define PASSWORD

#include <iostream>
#include <thread>
#include <mutex>
#include <ctime>

struct password
{
    std::string username;
    std::string encryped_password;
    std::string description;
    std::string url;
    long long date_modified;
    long long date_created;
    int p_id;
    mutable std::mutex password_mutex;

    password();
    password(std::string username, std::string password, std::string description, std::string url, long long date_created, long long date_modified, int p_id);
    password(const password &other);
    ~password();

    std::string encrypt_data(std::string password);
    std::string decrypt_data();
    password &operator=(password const &other);
};

#endif