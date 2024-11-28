// userAccount Tests
#include <cassert>
#include <iostream>
#include "./testHelpers.cpp"

int user_id = 1;
int number_of_passwords = 10;
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
}
void test_four(){
    // TEST UserAccount::modify_password
    UserAccount usr = get_user_account(username, user_id, number_of_passwords);
}
void test_five(){
    // TEST UserAccount::modify_password
    UserAccount usr = get_user_account(username, user_id, number_of_passwords);
}

int main(int argc, char *argv[]){
    test_one();
    test_two();
    test_three();
    test_four();
    test_five();
}