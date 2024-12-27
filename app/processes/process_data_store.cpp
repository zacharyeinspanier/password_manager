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
        const char * path = &db_path_copy[0];
        rc = sqlite3_open(path, &db);

        // We are inserting into the user data base
        // There are two options
        // INSET OR REPLACE
        // 1: the password with p_id alred exists
        // 2: the password does not exist and needs to be inserted
            // a: How will I ensure that add_password will assign a unique p_id?
                // When the data is loaded I could just find the max p_id, then increment by 1



        // 3: update the existing
        // 4: INSERT WHERE user_id and password id


        if(final_store_and_exit){
            // Final data store complete
            break;
        }
    }
}