#include "../objects/displayContent.hpp"

void DisplayContent::search_bar_process(){
    std::unique_lock<std::mutex> search_term_lock(this->search_term_mutex, std::defer_lock);
    std::unique_lock<std::mutex> search_loop_lock(this->search_loop_mutex, std::defer_lock);
    std::unique_lock<std::mutex> user_account_lock(this->user_account_mutex, std::defer_lock);
    std::unique_lock<std::mutex> display_passwords_lock(this->display_passwords_mutex, std::defer_lock);
   
    while(true){
        search_term_lock.lock();
        while(this->search_event_state == false || this->search_term == nullptr){
            this->search_term_cv.wait(search_term_lock);
        }

        // EXIT LOOP
        search_loop_lock.lock();
        if(this->search_loop_exit){
            search_loop_lock.unlock();
            break;
        }
        search_loop_lock.unlock();

        // !!CRITICAL CODE!! //
        user_account_lock.lock();
        display_passwords_lock.lock();
        this->display_passwords.erase(this->display_passwords.begin(), this->display_passwords.end());
        this->usr_acc->search(*this->search_term, &this->display_passwords);
        display_passwords_lock.unlock();
        user_account_lock.unlock();
       
        this->search_event_state = false;
        this->search_term = nullptr;
        search_term_lock.unlock();
    }  
}