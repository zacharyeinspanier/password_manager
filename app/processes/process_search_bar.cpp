#include "../objects/displayContent.hpp"

void DisplayContent::search_bar_process(){
    std::unique_lock<std::mutex> search_term_lock(this->search_term_mutex, std::defer_lock);
    std::unique_lock<std::mutex> user_account_lock(this->user_account_mutex, std::defer_lock);
   
    while(true){
        search_term_lock.lock();
        while(this->search_event_state == false || this->search_term == nullptr){
            this->search_term_cv.wait(search_term_lock);
        }
      
        user_account_lock.lock();
        // !!CRITICAL CODE!! //
        user_account_lock.unlock();

        // EXIT Loop iteration
        this->search_event_state = false;
        this->search_term = nullptr;
        search_term_lock.unlock();

        // TODO: add some an exit condition
        // thus if the app closing, break this while loop 
    }
    
}