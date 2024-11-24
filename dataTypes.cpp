#include <iostream>
using namespace std;

struct password{
    string username;
    string password;
    string modify_date;
    string date_created;
    string description;
    int p_id;
};

enum operationType{
    ADD,
    MODIFY,
    REMOVE,
    VIEW
};


struct Operation{
    operationType operation_type;
    int password_id; // Add operations will not need this
};
