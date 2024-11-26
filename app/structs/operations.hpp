#ifndef OPERATIONS 
#define OPERATIONS

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

#endif