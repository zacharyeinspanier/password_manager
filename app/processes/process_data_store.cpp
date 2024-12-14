#include "../objects/displayContent.hpp"

// IDEA:
    // every 2 seconds the following sequence occurs
    // lock userAccount mutex
    // copy all data in UserAccount::pass_id_map to a temporary vector
    // call cond_var.notify() to wake sql store thread
        // thread will then iterate over vector and store/update data
    // release lock
void DisplayContent::periodic_data_store(){
    std::unique_lock<std::mutex> user_account_lock(this->user_account_mutex, std::defer_lock);
    while(true){
        std::this_thread::sleep_for(std::chrono::milliseconds(2000));

        user_account_lock.lock();
        // !!CRITICAL CODE!! //
        // UserAccount::get_data_copy()
        // UserAccount::user_id()
        user_account_lock.unlock();


        // UPDATE data in sql DB
    }
}