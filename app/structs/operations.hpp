#ifndef OPERATIONS 
#define OPERATIONS

enum operationType{
    ADD,
    MODIFY,
    REMOVE,
    VIEW
};

enum modifyType{
    MODIFY_PASSWORD,
    MODIFY_DESCRIPTION,
};


struct operation{
    operationType operation_type;
    int password_id; // Add operations will not need this
};

#endif