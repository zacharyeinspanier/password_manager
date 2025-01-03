#include "./testHelpers.cpp"
#include "../../app/objects/displayContent.cpp"

std::mutex UserAccount::user_data_mutex;
std::vector<password> UserAccount::initial_user_data;

int number_of_passwords = 10;
int curr_pass_id = 0;
std::string username = "test_user";
std::set<int> passwords_in_db_sql_query;
std::set<int> passwords_in_db;
DisplayContent *test_content = nullptr;
std::string test;

void test_one()
{
    // TEST ONE: Add Operation
    std::set<int> new_ids;
    for (int i = 0; i < number_of_passwords; ++i)
    {
        password new_password;
        operation new_operation;
        new_password.p_id = 101 + i;
        new_password.username = "username_" + std::to_string(101 + i);
        new_password.encryped_password = "password_" + std::to_string(101 + i);
        new_ids.insert(new_password.p_id);
        passwords_in_db.insert(new_password.p_id);
        new_operation.operation_type = operationType::ADD;
        new_operation.curr_password = new_password;
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
        passwords_in_db.erase(remove_password.p_id);
        remove_operation.operation_type = operationType::REMOVE;
        remove_operation.curr_password = remove_password;
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
        modify_operation.operation_type = operationType::MODIFY;
        modify_operation.curr_password = modify_password;
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
        modify_operation.operation_type = operationType::MODIFY;
        modify_operation.curr_password = modify_password;
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
        add_password.description = "description temp";
        add_password.date_created = time(0);
        add_password.date_modified = time(0);
        passwords_same_url.insert(add_password.p_id);
        passwords_in_db.insert(add_password.p_id);
        add_operation.operation_type = operationType::ADD;
        add_operation.curr_password = add_password;
        test_content->operation_event(add_operation);
    }

    test_content->search_event(test_url);

    // Sleep to allow operation queue to empty
    std::this_thread::sleep_for(std::chrono::milliseconds(2000));

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
    passwords_in_db.insert(add_password.p_id);
    add_password.username = "username_" + std::to_string(6001);
    add_password.encryped_password = "password_" + std::to_string(6001);
    add_password.description = test_desc;
    add_operation.operation_type = operationType::ADD;
    add_operation.curr_password = add_password;
    test_content->operation_event(add_operation);

    test_content->search_event(test_desc);

    // Sleep to allow operation queue to empty
    std::this_thread::sleep_for(std::chrono::milliseconds(2000));
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

int sql_callback(void *data, int argc, char **argv, char **azColName)
{
    // NOTE if the password is changed, this could be an error
    // PASSWORD_ID == column 6
    int p_id = std::stoi(argv[6]);
    passwords_in_db_sql_query.insert(p_id);

    return 0;
}

void test_eight()
{
    sqlite3 *db;
    int rc;
    const char *path = &db_path[0];
    rc = sqlite3_open(path, &db);

    assert(rc == SQLITE_OK);

    std::string sql = "SELECT * FROM USER_DATA WHERE USERID='" + std::to_string(user_id) + "';";
    std::string data = "CALLBACK FUNCTION";
    char *messaggeError;
    int rc_exec;
    rc_exec = sqlite3_exec(db, sql.c_str(), sql_callback, (void *)data.c_str(), &messaggeError);
    assert(rc_exec == SQLITE_OK);

    for (auto item : passwords_in_db_sql_query)
    {
        assert(passwords_in_db.contains(item));
    }

    std::cout << "Test Eight Pass" << std::endl;
}

int main(int argc, char *argv[])
{
    char *env_db_path_raw = std::getenv("DB_PATH");
    db_path = std::string(env_db_path_raw);
    clean_up_database();

    generate_passwords(number_of_passwords);
    for (int i = 0; i < number_of_passwords; ++i)
    {
        passwords_in_db.insert(i);
    }

    test_content = new DisplayContent(username, user_id, &db_path);
    test_one();
    test_two();
    test_three();
    test_four();
    test_five();
    test_six();
    test_seven();
    std::this_thread::sleep_for(std::chrono::milliseconds(2000));
    test_eight();
    delete test_content;
    test_content = nullptr;
    clean_up_database();
    return 0;
}