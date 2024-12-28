#include "displayContent.hpp"
#include "../processes/process_data_store.cpp"
#include "../processes/process_operation_event.cpp"
#include "../processes/process_search_bar.cpp"

// Constructor
DisplayContent::DisplayContent(std::string * db_path)
{
    this->usr_acc = UserAccount::get_instance();
    {
        // get initial copy of data
        std::lock_guard<std::mutex> lock_display_passwords(this->display_passwords_mutex);
        std::lock_guard<std::mutex> lock_user_acc(this->user_account_mutex);
        for (const auto &item : this->usr_acc->get_data_copy())
        {
            std::shared_ptr<password> curr_pass = std::make_shared<password>(item.second);
            this->display_passwords.insert(curr_pass);
        }
    }

    this->db_path = *db_path;
    this->data_store_exit = false;

    this->search_thread_done = false;
    this->operation_thread_done = false;

    this->operation_event_state = false;
    this->operation_loop_exit = false;

    this->search_term = nullptr;
    this->search_event_state = false;
    this->search_loop_exit = false;
}

// Singelton get_instance
// Checks if the instance has been initialized
// if so, return the instance pointer
// otherwise create an instance of the object with new
DisplayContent *DisplayContent::get_instance(std::string * db_path)
{
    if (DisplayContent::instance_ptr == nullptr)
    {
        std::lock_guard<std::mutex> lock(DisplayContent::display_content_mutex);
        DisplayContent::instance_ptr = new DisplayContent(db_path);
    }

    return instance_ptr;
}
void DisplayContent::deinitialize_instance() 
{
    instance_ptr = nullptr;
}

void DisplayContent::reset_display_list()
{
    std::lock_guard<std::mutex> lock_display_passwords(this->display_passwords_mutex);
    std::lock_guard<std::mutex> lock_user_acc(this->user_account_mutex);
    this->display_passwords.erase(this->display_passwords.begin(), this->display_passwords.end());
    for (const auto &item : this->usr_acc->get_data_copy())
    {
        std::shared_ptr<password> curr_pass = std::make_shared<password>(item.second);
        this->display_passwords.insert(curr_pass);
    }
}

std::vector<password> DisplayContent::get_display_list()
{
    std::lock_guard<std::mutex> lock(this->display_passwords_mutex);
    std::vector<password> res;
    for (std::set<std::shared_ptr<password>>::iterator curr = this->display_passwords.begin(); curr != this->display_passwords.end(); ++curr)
    {
        password temp = *((*curr).get());
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

    // Operation process exit
    {
        std::lock_guard<std::mutex> operation_lock(this->operation_mutex);
        std::lock_guard<std::mutex> operation_exit_lock(this->operation_loop_mutex);
        this->operation_loop_exit = true;
        this->operation_event_state = true;
        operation empty_operation;
        this->operation_queue.push(empty_operation);
    }
    this->operation_cv.notify_all(); // notify wakes thread -> then exit
    
    // Data Store EXIT
    {
        // Both Search and Operations threads must finish before data store can exit
        // Allowing for user data to settle before complete exit
        std::unique_lock<std::mutex> exit_threads_lock(this->exit_threads_mutex, std::defer_lock);
        exit_threads_lock.lock();
        while(this->search_thread_done == false || this->operation_thread_done == false){
            this->exit_threads_cv.wait(exit_threads_lock);
        }
        exit_threads_lock.unlock();

        std::lock_guard<std::mutex> data_store_exit_lock(this->data_store_loop_mutex);
        this->data_store_exit = true;
    }
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