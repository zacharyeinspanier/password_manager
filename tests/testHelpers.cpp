#include "../app/structs/password.cpp"
#include "../app/objects/userAccount.cpp"

void generate_passwords(int number, std::vector<password> * test_passwords){

    for(int i = 0; i < number; ++i){
        password curr_pass;
        curr_pass.p_id = i;
        curr_pass.username = "username_" +  std::to_string(i);
        curr_pass.encryped_password =  "password_" +  std::to_string(i);
        test_passwords->push_back(curr_pass);
    }

} 


UserAccount get_user_account(std::string username, int user_id, int number, std::vector<password> * test_passwords){
    generate_passwords(number, test_passwords);
    UserAccount acc = UserAccount(username, user_id, test_passwords);
    return UserAccount(username, user_id, test_passwords); 
}