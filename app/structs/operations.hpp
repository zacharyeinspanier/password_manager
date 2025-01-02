#ifndef OPERATIONS
#define OPERATIONS

#include "./password.hpp"

enum operationType
{
    NONE,
    ADD,
    MODIFY,
    REMOVE,
    VIEW // TODO: Remove this; no longer needed. the front end has a copy of the data 
                // if we encrypt the password data, the password struct will decrypt and encrypt. 
};

// TODO:
// Remove modifyType
// Modify will just re-write all data from password struct to the matching p_id

enum modifyType
{
    MODIFY_NONE,
    MODIFY_PASSWORD,
    MODIFY_DESCRIPTION
};

struct operation
{
    operationType operation_type;
    modifyType modify_type;
    password new_password; // TOOD: it would be nice to have this just be named current_password


    operation &operator=(operation const &other);
};

#endif