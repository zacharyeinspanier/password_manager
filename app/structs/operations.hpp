#ifndef OPERATIONS
#define OPERATIONS

#include "./password.hpp"

enum operationType
{
    NONE,
    ADD,
    MODIFY,
    REMOVE
};

struct operation
{
    operationType operation_type;
    password curr_password;

    operation &operator=(operation const &other);
};

#endif