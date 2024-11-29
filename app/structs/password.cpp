#include "password.hpp"

password& password::operator=(password const& other){
    // password const& other
    // meaning pass by reference. Thus it is the same password not a copy of password

    // &other != this: the address of other does not equal the address of this
    // keep in mind that this is a pointer
    if(&other != this){
        std::lock_guard<std::mutex> other_lock_guard(other.password_mutex);
        std::lock_guard<std::mutex> this_lock_guard(password_mutex);

        username = other.username;
        encryped_password = other.encryped_password;
        description = other.description;
        url = other.url;
        modify_date = other.modify_date;
        date_created = other.date_created;
        p_id = other.p_id;
    }

    // return dereference of this
    // thus the return type is the address of this
    // password & (the address of password)
    return *this;
}