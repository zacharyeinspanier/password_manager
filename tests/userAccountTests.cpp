// userAccount Tests
#include <cassert>
#include <iostream>
#include "./testHelpers.cpp"
#include "../app/structs/operations.hpp"

int user_id = 1;
int number_of_passwords = 10;
int curr_pass_id = 0;
std::string username = "test_user";


void test_one(){
    // TEST UserAccount Constructor adds all passwords
    UserAccount usr = get_user_account(username, user_id, number_of_passwords);

    for(int i = 0; i < number_of_passwords; ++i){
        assert(usr.contains_password(i) == true);
    }
    std::cout << "Test One Pass" << std::endl;
}
void test_two(){
    // TEST UserAccount::add_password add new password
    UserAccount usr = get_user_account(username, user_id, number_of_passwords);
    password new_password;
    new_password.p_id = 100;
    new_password.username = "username_100";
    new_password.password =  "password_100";
    usr.add_password(&new_password);
    assert(usr.contains_password(100) == true);
    std::cout << "Test Two Pass" << std::endl;
}
void test_three(){
    // TEST UserAccount::remove_password
    UserAccount usr = get_user_account(username, user_id, number_of_passwords);
    usr.remove_password(curr_pass_id);
    assert(usr.contains_password(curr_pass_id) == false);
    for(int i = 1; i < number_of_passwords; ++i){
        assert(usr.contains_password(i) == true);
    }
}
void test_four(){
    // TEST UserAccount::modify_password
    UserAccount usr = get_user_account(username, user_id, number_of_passwords);
    usr.modify_password(curr_pass_id, "new description", modifyType::MODIFY_DESCRIPTION);
    std::unordered_map<int, password> usr_data_copy = usr.get_data_copy();
    assert(usr_data_copy.contains(curr_pass_id) == true);
    std::string new_desc = usr_data_copy.extract(curr_pass_id).mapped().description;
    assert(new_desc == "new description");
}
void test_five(){
    // TEST UserAccount::modify_password
    UserAccount usr = get_user_account(username, user_id, number_of_passwords);
    usr.modify_password(curr_pass_id, "new password", modifyType::MODIFY_PASSWORD);
    std::unordered_map<int, password> usr_data_copy = usr.get_data_copy();
    assert(usr_data_copy.contains(curr_pass_id) == true);
    std::string new_password = usr_data_copy.extract(curr_pass_id).mapped().password;
    assert(new_password == "new password");
}

int main(int argc, char *argv[]){
    test_one();
    test_two();
    test_three();
    test_four();
    test_five();
}