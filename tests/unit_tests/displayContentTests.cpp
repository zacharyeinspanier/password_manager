#include <cassert>
#include <chrono>
#include <iostream>
#include "../../app/structs/password.cpp"
#include "../../app/structs/operations.cpp"
#include "../../app/objects/userAccount.cpp"
#include "../../app/objects/displayContent.cpp"

DisplayContent *DisplayContent::instance_ptr = nullptr;
std::mutex DisplayContent::display_content_mutex;

UserAccount *UserAccount::instance_ptr = nullptr;
std::mutex UserAccount::user_acc_mutex;

int user_id = 1;
int number_of_passwords = 10;
int curr_pass_id = 0;
std::string username = "test_user";
std::vector<password> test_passwords;
DisplayContent *test_content;

void generate_passwords(int number, std::vector<password> *test_passwords)
{
    for (int i = 0; i < number; ++i)
    {
        password curr_pass;
        curr_pass.p_id = i;
        curr_pass.username = "username_" + std::to_string(i);
        curr_pass.encryped_password = "password_" + std::to_string(i);
        curr_pass.url = "https://" + std::to_string(i);
        curr_pass.description = "description_" + std::to_string(i);
        test_passwords->push_back(curr_pass);
    }
}

UserAccount *get_user_account(std::string username, int user_id, int number, std::vector<password> *test_passwords)
{
    generate_passwords(number, test_passwords);
    return UserAccount::initialize_instance(username, user_id, test_passwords);
}

void test_one()
{
    // TEST ONE: Add Operation
    // TODO: !!! operation must be queue for this to work !!!
    std::set<int> new_ids;
    for (int i = 0; i < number_of_passwords; ++i)
    {
        password new_password;
        operation new_operation;
        new_password.p_id = 101 + i;
        new_password.username = "username_" + std::to_string(101 + i);
        new_password.encryped_password = "password_" + std::to_string(101 + i);
        new_ids.insert(101 + i);
        new_operation.modify_type = modifyType::MODIFY_NONE;
        new_operation.operation_type = operationType::ADD;
        new_operation.new_password = new_password;
        test_content->operation_event(new_operation);
    }

    // Sleep to allow operation queue to empty
    std::this_thread::sleep_for(std::chrono::milliseconds(1000));

    test_content->reset_display_list();
    auto user_data = test_content->get_display_list();
    for (const auto &item : user_data)
    {
        if (new_ids.contains(item.p_id))
        {
            new_ids.erase(item.p_id);
        }
    }
    assert(new_ids.empty());

    std::cout << "Test One Pass" << std::endl;
}

void test_two()
{
    // TEST TWO: Remove operation
    std::set<int> remove_ids;
    for (int i = 0; i < number_of_passwords; ++i)
    {
        password remove_password;
        operation remove_operation;
        remove_password.p_id = 101 + i;
        remove_password.username = "username_" + std::to_string(101 + i);
        remove_password.encryped_password = "password_" + std::to_string(101 + i);
        remove_ids.insert(101 + i);
        remove_operation.modify_type = modifyType::MODIFY_NONE;
        remove_operation.operation_type = operationType::REMOVE;
        remove_operation.new_password = remove_password;
        test_content->operation_event(remove_operation);
    }

    // Sleep to allow operation queue to empty
    std::this_thread::sleep_for(std::chrono::milliseconds(1000));

    test_content->reset_display_list();
    auto user_data = test_content->get_display_list();
    // None of the p_id's in remove_ids will be in user data
    for (const auto &item : user_data)
    {
        assert(remove_ids.contains(item.p_id) == false);
    }
    std::cout << "Test Two Pass" << std::endl;
}
void test_three()
{
    // TEST THREE: Modify Desc
    test_content->reset_display_list();
    auto user_data_before = test_content->get_display_list();
    for (const auto &item : user_data_before)
    {
        password modify_password = item;
        operation modify_operation;
        modify_password.username = "test";
        modify_password.description = "new_descrioption_" + std::to_string(item.p_id);
        modify_operation.modify_type = modifyType::MODIFY_DESCRIPTION;
        modify_operation.operation_type = operationType::MODIFY;
        modify_operation.new_password = modify_password;
        test_content->operation_event(modify_operation);
    }

    // Sleep to allow operation queue to empty
    std::this_thread::sleep_for(std::chrono::milliseconds(1000));

    test_content->reset_display_list();
    auto user_data_after = test_content->get_display_list();
    for (const auto &item : user_data_after)
    {
        std::string desc_assert = "new_descrioption_" + std::to_string(item.p_id);
        assert(item.description == desc_assert);
    }

    std::cout << "Test Three Pass" << std::endl;
}
void test_four()
{
    // TEST FOUR: Modify Pass
    test_content->reset_display_list();
    auto user_data_before = test_content->get_display_list();
    for (const auto &item : user_data_before)
    {
        password modify_password = item;
        operation modify_operation;
        modify_password.username = "test";
        modify_password.encryped_password = "new_password_" + std::to_string(item.p_id);
        modify_operation.modify_type = modifyType::MODIFY_PASSWORD;
        modify_operation.operation_type = operationType::MODIFY;
        modify_operation.new_password = modify_password;
        test_content->operation_event(modify_operation);
    }

    // Sleep to allow operation queue to empty
    std::this_thread::sleep_for(std::chrono::milliseconds(1000));

    test_content->reset_display_list();
    auto user_data_after = test_content->get_display_list();
    for (const auto &item : user_data_after)
    {
        std::string pass_assert = "new_password_" + std::to_string(item.p_id);
        assert(item.encryped_password == pass_assert);
    }

    std::cout << "Test Four Pass" << std::endl;
}

void test_five()
{
    // TEST FIVE: search bar sets display_passwords
    std::set<int> passwords_same_url;
    std::string test_url = "https://testaddress.io";
    for (int i = 0; i < number_of_passwords; ++i)
    {
        password add_password;
        operation add_operation;
        add_password.p_id = 501 + i;
        add_password.username = "username_" + std::to_string(501 + i);
        add_password.encryped_password = "password_" + std::to_string(501 + i);
        add_password.url = test_url;
        passwords_same_url.insert(501 + i);
        add_operation.modify_type = modifyType::MODIFY_NONE;
        add_operation.operation_type = operationType::ADD;
        add_operation.new_password = add_password;
        test_content->operation_event(add_operation);
    }

    test_content->search_event(test_url);

    // Sleep to allow operation queue to empty
    std::this_thread::sleep_for(std::chrono::milliseconds(1000));

    auto user_data = test_content->get_display_list();

    for (const auto &item : user_data)
    {
        assert(passwords_same_url.contains(item.p_id));
        passwords_same_url.erase(item.p_id);
    }

    assert(passwords_same_url.empty());

    std::cout << "Test Five Pass" << std::endl;
}

void test_six()
{

    // TEST SIX: preivous results are erased when repeatly running search.

    // test five searched for URL and resulted in number_of_passwords
    auto user_data_before = test_content->get_display_list();
    assert(user_data_before.size() == number_of_passwords);
    std::string test_desc = "this test is to test resetting the display_passwords member";
    password add_password;
    operation add_operation;
    add_password.p_id = 6001;
    add_password.username = "username_" + std::to_string(6001);
    add_password.encryped_password = "password_" + std::to_string(6001);
    add_password.description = test_desc;
    add_operation.modify_type = modifyType::MODIFY_NONE;
    add_operation.operation_type = operationType::ADD;
    add_operation.new_password = add_password;
    test_content->operation_event(add_operation);

    test_content->search_event(test_desc);

    // Sleep to allow operation queue to empty
    std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    auto user_data_after = test_content->get_display_list();
    assert(user_data_after.size() == 1);

    std::cout << "Test Six Pass" << std::endl;
}

void test_seven()
{

    auto user_data_before = test_content->get_display_list();
    assert(user_data_before.size() == 1);
    test_content->reset_display_list();
    auto user_data_after = test_content->get_display_list();
    assert(user_data_after.size() > 1);

    std::cout << "Test Seven Pass" << std::endl;
}

int main(int argc, char *argv[])
{
    get_user_account(username, user_id, number_of_passwords, &test_passwords);
    test_content = DisplayContent::get_instance();
    test_content->start_processes();
    test_one();
    test_two();
    test_three();
    test_four();
    test_five();
    test_six();
    test_seven();
    test_content->stop_processes();
    test_content->reset_display_list();
}