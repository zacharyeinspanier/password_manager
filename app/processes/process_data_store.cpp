#include "../objects/displayContent.hpp"

void DisplayContent::periodic_data_store()
{
    std::unique_lock<std::mutex> user_account_lock(this->user_account_mutex, std::defer_lock);
    std::unique_lock<std::mutex> data_store_exit_lock(this->data_store_loop_mutex, std::defer_lock);
    std::unique_lock<std::mutex> db_path_lock(this->db_path_mutex, std::defer_lock);

    std::string db_path_copy;
    db_path_lock.lock();
    db_path_copy = this->db_path;
    db_path_lock.unlock();

    int user_id;
    user_account_lock.lock();
    user_id = this->usr_acc->get_user_id();
    user_account_lock.unlock();

    while (true)
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(2000));

        // Check if thread is commanded to exit
        // Commit one final store if exit
        bool final_store_and_exit = false;
        data_store_exit_lock.lock();
        final_store_and_exit = this->data_store_exit;
        data_store_exit_lock.unlock();

        std::unordered_map<int, password> user_data_copy;
        user_account_lock.lock();
        // !!CRITICAL CODE!! //
        user_data_copy = this->usr_acc->get_data_copy();
        user_account_lock.unlock();

        sqlite3 *db;
        int rc;
        const char *path = &db_path_copy[0];
        rc = sqlite3_open(path, &db);

        for (auto item : user_data_copy)
        {
            password curr_password = item.second;
            char sql_insert[1500];
            snprintf(
                sql_insert,
                sizeof(sql_insert),
                "INSERT OR REPLACE INTO USER_DATA (USERNAME, PASSWORD, DESCRIPTION, URL, DATE_CREATED, DATE_MODIFIED, PASSWORD_ID, USERID) VALUES('%s', '%s', '%s','%s', %s, %s, %s, %s);",
                curr_password.username.c_str(),
                curr_password.encryped_password.c_str(),
                curr_password.description.c_str(),
                curr_password.url.c_str(),
                std::to_string(curr_password.date_created).c_str(),
                std::to_string(curr_password.date_modified).c_str(),
                std::to_string(curr_password.p_id).c_str(),
                std::to_string(user_id).c_str());

            int rc_exec;
            char *messaggeError;
            rc_exec = sqlite3_exec(db, sql_insert, NULL, 0, &messaggeError);
            if (rc_exec != SQLITE_OK)
            {
                std::cerr << "Error Insert data store" << std::endl;
                std::cout << messaggeError << std::endl;
                sqlite3_free(messaggeError);
            }
        }

        if (final_store_and_exit)
        {
            break;
        }
    }
}