#ifndef OPERATIONS 
#define OPERATIONS

#include "./password.hpp"

enum operationType{
    ADD,
    MODIFY,
    REMOVE,
    VIEW
};

enum modifyType{
    MODIFY_PASSWORD,
    MODIFY_DESCRIPTION,
    MODIFY_NONE,
};

// Step 1: create a structure 'operation' that carries all data necessary to complete the following operations
// - string for modify
// - modifyType for modify
// - password for add


struct operation{
    operationType operation_type;
    modifyType modify_type;
    password new_password;
};

#endif