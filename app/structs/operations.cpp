#include "operations.hpp"

operation &operation::operator=(operation const &other){

    if (&other != this){
        operation_type = other.operation_type;
        modify_type = other.modify_type;
        new_password = other.new_password;
    }

    return *this;
}