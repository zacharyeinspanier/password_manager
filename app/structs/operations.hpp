#ifndef OPERATIONS
#define OPERATIONS

#include "./password.hpp"

enum operationType
{
    ADD,
    MODIFY,
    REMOVE,
    VIEW
};

enum modifyType
{
    MODIFY_PASSWORD,
    MODIFY_DESCRIPTION,
    MODIFY_NONE,
};

struct operation
{
    operationType operation_type;
    modifyType modify_type;
    password new_password;


    operation &operator=(operation const &other);
};

#endif