// userAccount Tests
#include <cassert>
#include <iostream>
#include "./testHelpers.cpp"
#include "../../app/structs/operations.hpp"

int user_id = 1;
int number_of_passwords = 10;
int curr_pass_id = 0;
std::string username = "test_user";
std::vector<password> test_passwords;
UserAccount* UserAccount::instance_ptr = nullptr;
std::mutex UserAccount::user_acc_mutex;
UserAccount *test_user = nullptr;



void test_one(){
    // TEST UserAccount Constructor adds all passwords
    for(int i = 0; i < number_of_passwords; ++i){
        assert(test_user->contains_password(i) == true);
    }
    std::cout << "Test One Pass" << std::endl;
}
void test_two(){
    // TEST UserAccount::add_password add new password
    password new_password;
    new_password.p_id = 100;
    new_password.username = "username_100";
    new_password.encryped_password =  "password_100";
    test_user->add_password(&new_password);
    assert(test_user->contains_password(100) == true);
    std::cout << "Test Two Pass" << std::endl;
}
void test_three(){
    // TEST UserAccount::remove_password
    test_user->remove_password(curr_pass_id);
    assert(test_user->contains_password(curr_pass_id) == false);
    for(int i = 1; i < number_of_passwords; ++i){
        assert(test_user->contains_password(i) == true);
    }
    std::cout << "Test Three Pass" << std::endl;
    curr_pass_id++; // increment since password is removed
}
void test_four(){
    // TEST UserAccount::modify_password
    test_user->modify_password(curr_pass_id, "new description", modifyType::MODIFY_DESCRIPTION);
    std::unordered_map<int, password> test_user_data_copy = test_user->get_data_copy();
    assert(test_user_data_copy.contains(curr_pass_id) == true);
    std::string new_desc = test_user_data_copy.extract(curr_pass_id).mapped().description;
    assert(new_desc == "new description");
    std::cout << "Test Four Pass" << std::endl;
}
void test_five(){
    // TEST UserAccount::modify_password
    test_user->modify_password(curr_pass_id, "new password", modifyType::MODIFY_PASSWORD);
    std::unordered_map<int, password> test_user_data_copy = test_user->get_data_copy();
    assert(test_user_data_copy.contains(curr_pass_id) == true);
    std::string new_password = test_user_data_copy.extract(curr_pass_id).mapped().encryped_password;
    assert(new_password == "new password");
    std::cout << "Test Five Pass" << std::endl;
}

void test_six(){
    // TEST UserAccount::get_instance()
    UserAccount *usr_acc_get_instance = UserAccount::get_instance();
    assert(usr_acc_get_instance == test_user);
    UserAccount *usr_acc_initialize =  UserAccount::initialize_instance(username, user_id, &test_passwords);
    assert(usr_acc_initialize == test_user);
    std::cout << "Test Six Pass" << std::endl;

}

int main(int argc, char *argv[]){
    test_user = get_user_account(username, user_id, number_of_passwords, &test_passwords);
    test_one();
    test_two();
    test_three();
    test_four();
    test_five();
    test_six();
}