#include <iostream>
#include <thread>
#include <ctime>
using namespace std;

struct password{
    string username;
    string password;
    string description;
    string url;
    time_t modify_date;
    time_t date_created;
    int p_id;
    mutex mutx;

    string encrypt_data(string password);
    string decrypt_data();
};

enum operationType{
    ADD,
    MODIFY,
    REMOVE,
    VIEW
};


struct operation{
    operationType operation_type;
    int password_id; // Add operations will not need this
};
