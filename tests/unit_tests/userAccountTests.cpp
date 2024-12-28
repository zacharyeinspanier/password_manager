// userAccount Tests
#include "./testHelpers.cpp"

int number_of_passwords = 10;
int curr_pass_id = 0;
std::string username = "test_user";

std::mutex UserAccount::user_data_mutex;
std::vector<password> UserAccount::initial_user_data;
UserAccount *test_user = nullptr;
std::set<std::shared_ptr<password>> search_result;

void test_one()
{
    // TEST UserAccount Constructor adds all passwords
    for (int i = 0; i < number_of_passwords; ++i)
    {
        assert(test_user->contains_password(i) == true);
    }
    std::cout << "Test One Pass" << std::endl;
}
void test_two()
{
    // TEST UserAccount::add_password add new password
    password new_password;
    new_password.p_id = 100;
    new_password.username = "username_100";
    new_password.encryped_password = "password_100";
    test_user->add_password(&new_password);
    assert(test_user->contains_password(100) == true);
    std::cout << "Test Two Pass" << std::endl;
}
void test_three()
{
    // TEST UserAccount::remove_password
    test_user->remove_password(curr_pass_id);
    assert(test_user->contains_password(curr_pass_id) == false);
    for (int i = 1; i < number_of_passwords; ++i)
    {
        assert(test_user->contains_password(i) == true);
    }
    std::cout << "Test Three Pass" << std::endl;
    curr_pass_id++; // increment since password is removed
}
void test_four()
{
    // TEST UserAccount::modify_password
    test_user->modify_password(curr_pass_id, "new description", modifyType::MODIFY_DESCRIPTION);
    std::unordered_map<int, password> test_user_data_copy = test_user->get_data_copy();
    assert(test_user_data_copy.contains(curr_pass_id) == true);
    std::string new_desc = test_user_data_copy.extract(curr_pass_id).mapped().description;
    assert(new_desc == "new description");
    std::cout << "Test Four Pass" << std::endl;
}
void test_five()
{
    // TEST UserAccount::modify_password
    test_user->modify_password(curr_pass_id, "new password", modifyType::MODIFY_PASSWORD);
    std::unordered_map<int, password> test_user_data_copy = test_user->get_data_copy();
    assert(test_user_data_copy.contains(curr_pass_id) == true);
    std::string new_password = test_user_data_copy.extract(curr_pass_id).mapped().encryped_password;
    assert(new_password == "new password");
    std::cout << "Test Five Pass" << std::endl;
}


void test_seven()
{
    // When there are mutiple passwords with the same username they are grouped together
    // TEST that all usernames with the same value are grouped together
    std::set<int> static_username_ids;
    for (int i = 0; i < number_of_passwords; ++i)
    {
        password new_password;
        new_password.p_id = 101 + i;
        new_password.username = "static_username";
        new_password.encryped_password = "password_" + std::to_string(101 + i);
        static_username_ids.insert(101 + i);
        test_user->add_password(&new_password);
        assert(test_user->contains_password(101 + i) == true);
    }
    std::string search_term = "static_username";
    search_result.erase(search_result.begin(), search_result.end());
    test_user->search(search_term, &search_result);
    for (const auto &item : search_result)
    {
        assert(static_username_ids.contains((*item.get()).p_id));
    }
    std::cout << "Test Seven Pass" << std::endl;
}

void test_eight()
{
    // TEST that all descriptions with the same value are grouped together
    std::set<int> static_description_ids;
    for (int i = 0; i < number_of_passwords; ++i)
    {
        password new_password;
        new_password.p_id = 201 + i;
        new_password.username = "username_" + std::to_string(101 + i);
        new_password.encryped_password = "password_" + std::to_string(101 + i);
        new_password.description = "static_description";
        static_description_ids.insert(201 + i);
        test_user->add_password(&new_password);
        assert(test_user->contains_password(101 + i) == true);
    }
    std::string search_term = "static_description";
    search_result.erase(search_result.begin(), search_result.end());
    test_user->search(search_term, &search_result);
    for (const auto &item : search_result)
    {
        assert(static_description_ids.contains((*item.get()).p_id));
    }
    std::cout << "Test Eight Pass" << std::endl;
}

void test_nine()
{
    // TEST that all urls with the same value are grouped together
    std::set<int> static_url_ids;
    for (int i = 0; i < number_of_passwords; ++i)
    {
        password new_password;
        new_password.p_id = 301 + i;
        new_password.username = "username_" + std::to_string(101 + i);
        new_password.encryped_password = "password_" + std::to_string(101 + i);
        new_password.description = "descr_" + std::to_string(101 + i);
        new_password.url = "https//:static_url";
        static_url_ids.insert(301 + i);
        test_user->add_password(&new_password);
        assert(test_user->contains_password(101 + i) == true);
    }
    std::string search_term = "https//:static_url";
    search_result.erase(search_result.begin(), search_result.end());
    test_user->search(search_term, &search_result);
    for (const auto &item : search_result)
    {
        assert(static_url_ids.contains((*item.get()).p_id));
    }
    std::cout << "Test Nine Pass" << std::endl;
}

void test_ten()
{
    std::set<int> static_username_ids;
    for (int i = 0; i < number_of_passwords; ++i)
    {
        static_username_ids.insert(101 + i);
    }

    for (int i = 0; i < number_of_passwords; ++i)
    {
        test_user->remove_password(101 + i);
        static_username_ids.erase(101 + i);
        assert(test_user->contains_password(101 + i) == false);

        // Ensure the value is erased from the username unordered map
        search_result.erase(search_result.begin(), search_result.end());
        std::string search_term = "static_username";
        test_user->search(search_term, &search_result);
        for (const auto &item : search_result)
        {
            assert(static_username_ids.contains((*item.get()).p_id));
        }
    }
    std::cout << "Test Ten Pass" << std::endl;
}

void test_eleven()
{
    std::set<int> static_description_ids;
    for (int i = 0; i < number_of_passwords; ++i)
    {
        static_description_ids.insert(201 + i);
    }

    for (int i = 0; i < number_of_passwords; ++i)
    {
        test_user->remove_password(201 + i);
        static_description_ids.erase(201 + i);
        assert(test_user->contains_password(201 + i) == false);

        // Ensure the value is erased from the username unordered map
        search_result.erase(search_result.begin(), search_result.end());
        std::string search_term = "static_description";
        test_user->search(search_term, &search_result);
        for (const auto &item : search_result)
        {
            assert(static_description_ids.contains((*item.get()).p_id));
        }
    }
    std::cout << "Test Eleven Pass" << std::endl;
}

void test_twelve()
{
    std::set<int> static_url_ids;
    for (int i = 0; i < number_of_passwords; ++i)
    {
        static_url_ids.insert(301 + i);
    }

    for (int i = 0; i < number_of_passwords; ++i)
    {
        test_user->remove_password(301 + i);
        static_url_ids.erase(301 + i);
        assert(test_user->contains_password(301 + i) == false);

        // Ensure the value is erased from the username unordered map
        search_result.erase(search_result.begin(), search_result.end());
        std::string search_term = "https//:static_url";
        test_user->search(search_term, &search_result);
        for (const auto &item : search_result)
        {
            assert(static_url_ids.contains((*item.get()).p_id));
        }
    }
    std::cout << "Test Twelve Pass" << std::endl;
}

int main(int argc, char *argv[])
{
    // NOTE: must set enviroment var DB_PATH.
    char *env_db_path_raw = std::getenv("DB_PATH");
    db_path = std::string(env_db_path_raw);
    clean_up_database();
    test_user = get_user_account(username, user_id, number_of_passwords);
    test_one();
    test_two();
    test_three();
    test_four();
    test_five();
    //test_six();
    test_seven();
    test_eight();
    test_nine();
    test_ten();
    test_twelve();
    test_eleven();
    clean_up_database();
}