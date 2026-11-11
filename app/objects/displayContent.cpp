#include "displayContent.hpp"
#include "../processes/process_data_store.cpp"
#include "../processes/process_operation_event.cpp"
#include "../processes/process_search_bar.cpp"

// Constructor
DisplayContent::DisplayContent(std::string username, int user_id, std::string *db_path)
{
    this->usr_acc = new UserAccount(username, user_id, db_path);
    {
        // get initial copy of data
        std::lock_guard<std::mutex> lock_display_passwords(this->display_passwords_mutex);
        std::lock_guard<std::mutex> lock_user_acc(this->user_account_mutex);
        for (const auto &item : this->usr_acc->get_data_copy())
        {
            this->display_passwords.emplace(item.first, item.second);
        }
    }

    this->db_path = *db_path;
    this->data_store_exit = false;

    this->operation_event_state = false;
    this->operation_loop_exit = false;

    this->search_term = nullptr;
    this->search_event_state = false;
    this->search_loop_exit = false;

    this->start_processes();
}

DisplayContent::~DisplayContent()
{
    this->stop_processes();
    if (this->usr_acc)
    {
        delete this->usr_acc;
        this->usr_acc = nullptr;
    }
}

void DisplayContent::reset_display_list()
{
    std::lock_guard<std::mutex> lock_display_passwords(this->display_passwords_mutex);
    std::lock_guard<std::mutex> lock_user_acc(this->user_account_mutex);

    auto user_data = this->usr_acc->get_data_copy();
    for (const auto &item : user_data)
    {
        if (this->display_passwords.contains(item.first))
        {
            this->display_passwords.at(item.first).username = item.second.username;
            this->display_passwords.at(item.first).encryped_password = item.second.encryped_password;
            this->display_passwords.at(item.first).url = item.second.url;
            this->display_passwords.at(item.first).description = item.second.description;
        }
        else
        {
            this->display_passwords.emplace(item.first, item.second);
        }
    }

    for (auto it = this->display_passwords.begin(); it != this->display_passwords.end();)
    {
        // Remove the item if user_data no longer contains p_id
        if (!user_data.contains(it->first))
        {
            it = this->display_passwords.erase(it);
        }
        else
        {
            ++it;
        }
        
    }
}

std::vector<password> DisplayContent::get_display_list()
{ 
    std::lock_guard<std::mutex> lock(this->display_passwords_mutex);
    std::vector<password> res;
    for (auto curr = this->display_passwords.begin(); curr != this->display_passwords.end(); ++curr)
    {
        password temp = curr->second;
        res.push_back(temp);
    }

    return res;
}

std::vector<password> DisplayContent::get_search_result(){
    std::lock_guard<std::mutex> lock(this->search_result_mutex);
    std::vector<password> res;
    for (auto curr = this->search_result.begin(); curr != this->search_result.end(); ++curr)
    {
        password temp = curr->second;
        res.push_back(temp);
    }

    return res;
}

void DisplayContent::start_processes()
{
    this->opeation_thread = std::thread(&DisplayContent::operation_process, this);
    this->search_thread = std::thread(&DisplayContent::search_bar_process, this);
    this->data_store_thread = std::thread(&DisplayContent::periodic_data_store, this);
}

void DisplayContent::stop_processes()
{
    // Search process exit
    {
        std::lock_guard<std::mutex> search_lock(this->search_term_mutex);
        std::lock_guard<std::mutex> search_exit_lock(this->search_loop_mutex);
        this->search_loop_exit = true;
        this->search_event_state = true;
        std::string exit_serach_term = "EXIT";
        this->search_term = &exit_serach_term;
    }
    this->search_term_cv.notify_all(); // notify wakes thread -> then exit
    this->search_thread.join();

    // Operation process exit
    {
        std::lock_guard<std::mutex> operation_lock(this->operation_mutex);
        std::lock_guard<std::mutex> operation_exit_lock(this->operation_loop_mutex);
        this->operation_loop_exit = true;
        this->operation_event_state = true;
        operation empty_operation;
        empty_operation.operation_type = operationType::NONE;
        this->operation_queue.push(empty_operation);
    }
    this->operation_cv.notify_all(); // notify wakes thread -> then exit
    this->opeation_thread.join();

    // Data Store EXIT
    {
        std::lock_guard<std::mutex> data_store_exit_lock(this->data_store_loop_mutex);
        this->data_store_exit = true;
    }
    this->data_store_thread.join();
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////
/// @brief -  This function will take an operation and set it as the current operation.
///
/// A condition variable is used to wake a sleeping thread that is waiting for an operation to process.
/// The sleeping thread is DisplayContent::operation_process().
/// It is intended to process one opeation at a time.
///
/// @param new_operation - the new operation to process
/////////////////////////////////////////////////////////////////////////////////////////////////////////
void DisplayContent::operation_event(operation &new_operation)
{
    std::unique_lock<std::mutex> operation_event_lock(this->operation_mutex, std::defer_lock);
    operation_event_lock.lock();

    this->operation_queue.push(new_operation);
    this->operation_event_state = true;

    operation_event_lock.unlock();
    this->operation_cv.notify_all();
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////
/// @brief -  This function will take an search term and set it as the current search term
///
/// A condition variable is used to wake a sleeping thread that is waiting for an search to process.
/// The sleeping thread is DisplayContent::search_bar_process().
/// It is intended to process one search at a time.
///
/// @param new_search_term - the new search term to process
/////////////////////////////////////////////////////////////////////////////////////////////////////////
void DisplayContent::search_event(std::string &new_search_term)
{
    std::unique_lock<std::mutex> search_event_lock(this->search_term_mutex, std::defer_lock);
    search_event_lock.lock();
    if (this->search_term == nullptr)
    {
        this->search_term = &new_search_term;
        this->search_event_state = true;
    }
    search_event_lock.unlock();
    this->search_term_cv.notify_all();
}