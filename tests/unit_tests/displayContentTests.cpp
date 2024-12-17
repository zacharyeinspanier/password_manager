#include <cassert>
#include <chrono>
#include <iostream>
#include "../../app/structs/password.cpp"
#include "../../app/objects/userAccount.cpp"
#include "../../app/objects/displayContent.cpp"


DisplayContent* DisplayContent::instance_ptr = nullptr;
std::mutex DisplayContent::display_content_mutex;

UserAccount* UserAccount::instance_ptr = nullptr;
std::mutex UserAccount::user_acc_mutex;

int user_id = 1;
int number_of_passwords = 10;
int curr_pass_id = 0;
std::string username = "test_user";
std::vector<password> test_passwords;
DisplayContent *test_content;
UserAccount *test_user;


void generate_passwords(int number, std::vector<password> * test_passwords){
    for(int i = 0; i < number; ++i){
        password curr_pass;
        curr_pass.p_id = i;
        curr_pass.username = "username_" +  std::to_string(i);
        curr_pass.encryped_password =  "password_" +  std::to_string(i);
        curr_pass.url = "https://" +  std::to_string(i);
        curr_pass.description = "description_" +  std::to_string(i);
        test_passwords->push_back(curr_pass);
    }
} 


UserAccount * get_user_account(std::string username, int user_id, int number, std::vector<password> * test_passwords){
    generate_passwords(number, test_passwords);
    return UserAccount::initialize_instance(username, user_id, test_passwords);
}



// Initialize Display Content
    // Initialize user account
// Start processes
// Create an operation
// assert that operation succeeded

// Test 1: operation_event

void test_one(){
    // Test operation event ADD
    // 1: create the add operation
    std::cout << "test_one start" << std::endl;
    operation add_operation;
    password new_password;
    new_password.p_id = 19385;
    new_password.username = "username_1000";
    new_password.encryped_password = "password_1000";
    new_password.description = "description_1000";

    add_operation.modify_type = modifyType::MODIFY_NONE;
    add_operation.operation_type = operationType::ADD;
    add_operation.new_password = new_password;

    test_content->operation_event(add_operation);

    operation add_operation_one;
    password new_password_one;
    new_password_one.p_id = 11111111;
    new_password_one.username = "username_11111";
    new_password_one.encryped_password = "password_11111";
    new_password_one.description = "description_11110";

    add_operation_one.modify_type = modifyType::MODIFY_NONE;
    add_operation_one.operation_type = operationType::ADD;
    add_operation_one.new_password = new_password_one;

    
    std::this_thread::sleep_for(std::chrono::milliseconds(2000));
    test_content->operation_event(add_operation_one);

    // test_content->reset_display_list();
    // auto data = test_content->get_display_list();
    // for (const auto& item :data) {
    //     std::cout << "ID: " << item.p_id << std::endl;
    // }
    
    //assert(test_user->contains_password(999) == true);
    //td::cout << "Test One Pass: "<< test_user->contains_password(999) << std::endl;
}
    
// Test 2: search_event

int main(int argc, char *argv[]){
    get_user_account(username, user_id, number_of_passwords, &test_passwords);
    //test_user = UserAccount::get_instance();
    test_content = DisplayContent::get_instance();
    std::thread opeation_thread(&DisplayContent::operation_process, test_content);
    //test_content->start_processes();
    test_one();
    test_content->stop_processes();
    opeation_thread.join();
    test_content->reset_display_list();
    // std::cout<< "+++++++"<<std::endl;
    // auto data = test_content->get_display_list();
    // for (const auto& item :data) {
    //     std::cout << "ID: " << item.p_id << std::endl;
    // }
}