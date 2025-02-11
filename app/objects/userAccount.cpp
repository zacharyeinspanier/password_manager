#include "userAccount.hpp"

std::string DEFAULT_STRING_VAL = "";

// userAccount implementation
UserAccount::UserAccount(const std::string username, const int user_id, std::string *db_path)
{
    this->account_username = username;
    this->user_id = user_id;
    this->db_path = *db_path;
    this->current_password_id = 0;
    std::chrono::time_point<std::chrono::system_clock> now = std::chrono::system_clock::now();
    this->time_since_epoch = now.time_since_epoch();
    this->get_password_id();
    this->current_password_id = std::max(10, UserAccount::max_password_id);
    this->get_user_data();

    for (int i = 0; i < UserAccount::initial_user_data.size(); ++i)
    {
        this->current_password_id = std::max(this->current_password_id, UserAccount::initial_user_data[i].p_id);
        this->add_password(&UserAccount::initial_user_data[i]);
    }
}

UserAccount::~UserAccount() {
    // TODO: could this cause problems? 
    UserAccount::initial_user_data.clear();
}

int UserAccount::sql_callback_password_id(void *data, int argc, char **argv, char **azColName){

    std::lock_guard<std::mutex> user_data_lock(UserAccount::user_data_mutex);
    std::string p_id_string = argv[6];
    int curr_p_id = stoi(p_id_string);
    UserAccount::max_password_id = std::max(UserAccount::max_password_id, curr_p_id);
    return 0;
}

void UserAccount::get_password_id(){
    sqlite3 *db;
    int rc;
    const char *path = &this->db_path[0];
    rc = sqlite3_open(path, &db);

    if (rc != SQLITE_OK)
    {
        // TODO: error handeling
        std::cerr << "Error Insert get user data" << std::endl;
    }
    else
    {
        std::string sql = "SELECT * FROM USER_DATA;";
        std::string data = "CALLBACK FUNCTION";
        int rc_exec = sqlite3_exec(db, sql.c_str(), UserAccount::sql_callback_password_id, (void *)data.c_str(), NULL);

        if (rc_exec != SQLITE_OK)
        {
            std::cerr << "Error Select" << std::endl;
        }
    }
}

int UserAccount::sql_callback_user_data(void *data, int argc, char **argv, char **azColName)
{
    // This callback is invoked each time the SELECT query finds a matching row.
    // This callback is used to store user data as a password, and insert the password into initial_user_data
    std::lock_guard<std::mutex> user_data_lock(UserAccount::user_data_mutex);
    password curr_password;
    curr_password.username = argv[0];
    curr_password.encryped_password = argv[1];
    curr_password.description = argv[2];
    curr_password.url = argv[3];
    std::string date_created_string = argv[4];
    curr_password.date_created = std::stoll(date_created_string);
    std::string date_modified_string = argv[5];
    curr_password.date_modified = std::stoll(date_modified_string);
    std::string p_id_string = argv[6];
    curr_password.p_id = stoi(p_id_string);

    UserAccount::initial_user_data.push_back(curr_password);

    return 0;
}

void UserAccount::get_user_data()
{
    sqlite3 *db;
    int rc;
    const char *path = &this->db_path[0];
    rc = sqlite3_open(path, &db);

    if (rc != SQLITE_OK)
    {
        // TODO: error handeling
        std::cerr << "Error Insert get user data" << std::endl;
    }
    else
    {
        std::string sql = "SELECT * FROM USER_DATA WHERE USERID =" + std::to_string(this->user_id) + ";";
        std::string data = "CALLBACK FUNCTION";
        int rc_exec = sqlite3_exec(db, sql.c_str(), UserAccount::sql_callback_user_data, (void *)data.c_str(), NULL);

        if (rc_exec != SQLITE_OK)
        {
            std::cerr << "Error Select" << std::endl;
        }
    }
}

void UserAccount::add_password(const password *new_password)
{

    std::lock_guard<std::mutex> unordered_map_lock(this->unordered_map_mutex);
    // Copy password
    password cpy_new_password = *new_password;

    // Set password id
    if (cpy_new_password.p_id == -1)
    {
        cpy_new_password.p_id = this->current_password_id + 1;
        this->current_password_id += 1;
    }

    // Set the dates in milliseconds
    if (cpy_new_password.date_created == 0)
    {
        cpy_new_password.date_created = std::chrono::duration_cast<std::chrono::milliseconds>(this->time_since_epoch).count();
        cpy_new_password.date_modified = std::chrono::duration_cast<std::chrono::milliseconds>(this->time_since_epoch).count();
    }

    // shared pointers
    std::shared_ptr<password> pass_id_map = std::make_shared<password>(cpy_new_password);
    std::shared_ptr<password> pass_url_map_ptr(pass_id_map);
    std::shared_ptr<password> pass_desc_map(pass_id_map);
    std::shared_ptr<password> pass_usr_map(pass_id_map);

    try
    {
        // description map
        if (this->description_map.contains(cpy_new_password.description))
        {
            this->description_map.at(cpy_new_password.description).push_back(pass_desc_map);
        }
        else
        {
            std::vector<std::shared_ptr<password>> map_vec;
            map_vec.push_back(pass_desc_map);
            this->description_map.emplace(cpy_new_password.description, map_vec);
        }

        // url map
        if (this->url_map.contains(cpy_new_password.url))
        {
            this->url_map.at(cpy_new_password.url).push_back(pass_url_map_ptr);
        }
        else
        {
            std::vector<std::shared_ptr<password>> map_vec;
            map_vec.push_back(pass_url_map_ptr);
            this->url_map.emplace(cpy_new_password.url, map_vec);
        }

        // username map
        if (this->username_map.contains(cpy_new_password.username))
        {
            this->username_map.at(cpy_new_password.username).push_back(pass_usr_map);
        }
        else
        {
            std::vector<std::shared_ptr<password>> map_vec;
            map_vec.push_back(pass_usr_map);
            this->username_map.emplace(cpy_new_password.username, map_vec);
        }
        this->pass_id_map.emplace(cpy_new_password.p_id, pass_id_map);
    }
    catch (...)
    {
        throw;
    }
}

void UserAccount::remove_password(int p_id)
{

    std::lock_guard<std::mutex> unordered_map_lock(this->unordered_map_mutex);

    if (this->pass_id_map.contains(p_id))
    {
        {
            std::lock_guard<std::mutex> removed_ids_lock(this->removed_ids_mutex);
            this->removed_ids.push_back(p_id);
        }

        // extract key/value pair from map
        auto node_handle = this->pass_id_map.extract(p_id);
        std::shared_ptr<password> removed_password = node_handle.mapped();
        if (this->url_map.contains(removed_password->url))
        {
            auto url_vec = this->url_map.at(removed_password->url);
            int remove_index = -1;
            for (int i = 0; i < url_vec.size(); ++i)
            {
                if (url_vec[i]->p_id == p_id)
                {
                    remove_index = i;
                }
            }
            if (remove_index != -1)
            {
                (this->url_map.at(removed_password->url)).erase(this->url_map.at(removed_password->url).begin() + remove_index);
            }
        }
        if (this->description_map.contains(removed_password->description))
        {
            std::vector<std::shared_ptr<password>> descrioption_vec = this->description_map.at(removed_password->description);
            int remove_index = -1;
            for (int i = 0; i < descrioption_vec.size(); ++i)
            {
                if (descrioption_vec[i]->p_id == p_id)
                {
                    remove_index = i;
                }
            }
            if (remove_index != -1)
            {
                this->description_map.at(removed_password->description).erase(this->description_map.at(removed_password->description).begin() + remove_index);
            }
        }
        if (this->username_map.contains(removed_password->username))
        {
            std::vector<std::shared_ptr<password>> username_vec = this->username_map.at(removed_password->username);
            int remove_index = -1;
            for (int i = 0; i < username_vec.size(); ++i)
            {
                if (username_vec[i]->p_id == p_id)
                {
                    remove_index = i;
                }
            }
            if (remove_index != -1)
            {
                this->username_map.at(removed_password->username).erase(this->username_map.at(removed_password->username).begin() + remove_index);
            }
        }
    }
}

void UserAccount::modify_password(const password *updated_password)
{
    std::lock_guard<std::mutex> unordered_map_lock(this->unordered_map_mutex);
    if (this->pass_id_map.contains(updated_password->p_id))
    {
        std::shared_ptr<password> password_ptr = this->pass_id_map[updated_password->p_id];
        std::lock_guard<std::mutex> password_ptr_mutex(password_ptr->password_mutex);
        if (updated_password->username != DEFAULT_STRING_VAL)
        {
            password_ptr->username = updated_password->username;
        }
        if (updated_password->encryped_password != DEFAULT_STRING_VAL)
        {
            password_ptr->encryped_password = updated_password->encryped_password;
        }
        if (updated_password->url != DEFAULT_STRING_VAL)
        {
            password_ptr->url = updated_password->url;
        }
        if (updated_password->description != DEFAULT_STRING_VAL)
        {
            password_ptr->description = updated_password->description;
        }

        password_ptr->date_modified = std::chrono::duration_cast<std::chrono::milliseconds>(this->time_since_epoch).count();
    }
}

std::unordered_map<int, password> UserAccount::get_data_copy()
{
    std::lock_guard<std::mutex> unordered_map_lock(this->unordered_map_mutex);
    std::unordered_map<int, password> user_data;
    for (auto it = this->pass_id_map.begin(); it != this->pass_id_map.end(); ++it)
    {
        password cpy_password = *(it->second.get());
        user_data.emplace(std::make_pair(it->second->p_id, cpy_password));
    }
    return user_data;
}

bool UserAccount::contains_password(const int p_id)
{
    std::lock_guard<std::mutex> unordered_map_lock(this->unordered_map_mutex);
    return this->pass_id_map.contains(p_id);
}

int UserAccount::get_user_id()
{
    return this->user_id;
}

std::unordered_map<int, password> UserAccount::search(std::string search_term)
{
    std::lock_guard<std::mutex> unordered_map_lock(this->unordered_map_mutex);
    std::unordered_map<int, password> search_result;
    if (auto search = this->url_map.find(search_term); search != this->url_map.end())
    {
        std::vector<std::shared_ptr<password>> url_search = (*search).second;
        for (int i = 0; i < url_search.size(); ++i)
        {
            search_result.emplace(url_search[i].get()->p_id, *url_search[i].get());
        }
    }
    if (auto search = this->username_map.find(search_term); search != this->username_map.end())
    {
        std::vector<std::shared_ptr<password>> username_search = (*search).second;
        for (int i = 0; i < username_search.size(); ++i)
        {
            search_result.emplace(username_search[i].get()->p_id, *username_search[i].get());
        }
    }
    if (auto search = this->description_map.find(search_term); search != this->description_map.end())
    {
        std::vector<std::shared_ptr<password>> descrioption_search = (*search).second;
        for (int i = 0; i < descrioption_search.size(); ++i)
        {
            search_result.emplace(descrioption_search[i].get()->p_id, *descrioption_search[i].get());
        }
    }
    return search_result;
}

std::vector<int> UserAccount::get_removed_ids(){
    std::lock_guard<std::mutex> removed_ids_lock(this->removed_ids_mutex);
    std::vector<int> removed_ids_copy = this->removed_ids;
    this->removed_ids.clear();
    return removed_ids_copy;
}
