#include "operations.hpp"

operation &operation::operator=(operation const &other)
{

    if (&other != this)
    {
        operation_type = other.operation_type;
        curr_password = other.curr_password;
    }

    return *this;
}