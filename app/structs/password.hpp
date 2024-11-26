#ifndef PASSWORD 
#define PASSWORD

#include <iostream>
#include <thread>
#include <ctime>

struct password{
    std::string username;
    std::string password;
    std::string description;
    std::string url;
    time_t modify_date;
    time_t date_created;
    int p_id;
    std::mutex mutx;

    std::string encrypt_data(std::string password);
    std::string decrypt_data();
};

#endif