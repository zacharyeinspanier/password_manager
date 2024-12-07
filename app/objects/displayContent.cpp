#include "displayContent.hpp"

// Constructor
DisplayContent::DisplayContent(){
    this->usr_acc = UserAccount::get_instance();

    this->current_operation = nullptr;
    this->operation_event_state = false;

    this->search_event_state = false;
    this->search_term = nullptr;
}

// Singelton get_instance
// Checks if the instance has been initialized
// if so, return the instance pointer
// otherwise create an instance of the object with new
DisplayContent* DisplayContent::get_instance(){
    if(DisplayContent::instance_ptr == nullptr){
        std::lock_guard<std::mutex> lock(DisplayContent::display_content_mutex);
        DisplayContent::instance_ptr = new DisplayContent();
    }

    return instance_ptr;
}

std::vector<password> DisplayContent::get_display_list(){
    std::lock_guard<std::mutex> lock(this->display_passwords_mutex);
    std::vector<password> res;
    for(int i = 0; i < this->display_passwords.size(); ++i){
        res.push_back(this->display_passwords[i]);
    }
    return res;
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
void DisplayContent::operation_event(operation &new_operation){
    std::unique_lock<std::mutex> operation_event_lock(this->operation_mutex, std::defer_lock);
    operation_event_lock.lock();
    // For now operations will be cleared by using a nullptr
    // The intended use is one operation at a time
    // Maybe in the future there could be a operation queue
    if(this->current_operation == nullptr){
        this->current_operation = &new_operation;
        this->operation_event_state = true;
    }
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
void DisplayContent::search_event(std::string &new_search_term){
    std::unique_lock<std::mutex> search_event_lock(this->search_term_mutex, std::defer_lock);
    search_event_lock.lock();
    if(this->search_term == nullptr){
        this->search_term = &new_search_term;
        this->search_event_state = true;
    }
    search_event_lock.unlock();
    this->search_term_cv.notify_all();
}