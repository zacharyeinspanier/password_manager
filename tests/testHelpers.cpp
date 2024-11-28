#include "../app/structs/password.hpp"
#include "../app/objects/userAccount.cpp"

std::vector<password> generate_passwords(int number){

    std::vector<password> res;

    for(int i = 0; i < number; ++i){
        password curr_pass;
        curr_pass.p_id = i;
        curr_pass.username = "username_" + i;
        curr_pass.password =  "password_" + i;
        res.push_back(curr_pass);
    }

    return res;
} 


UserAccount get_user_account(std::string username, int user_id, int number){
    std::vector<password> user_data = generate_passwords(number);
    return UserAccount(username, user_id, user_data);
}