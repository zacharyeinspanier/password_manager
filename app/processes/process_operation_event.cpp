#include "../objects/displayContent.hpp"


// enum operationType{
//     ADD,
//     MODIFY,
//     REMOVE,
//     VIEW
// };

void DisplayContent::operation_process(){
    std::unique_lock<std::mutex> operation_event_lock(this->operation_mutex, std::defer_lock);
    std::unique_lock<std::mutex> user_account_lock(this->user_account_mutex, std::defer_lock);
   
    while(true){
        operation_event_lock.lock();
        while(this->operation_event_state == false || this->current_operation == nullptr){
            this->operation_cv.wait(operation_event_lock);
        }
        // Step 1: create a structure 'operation' that carries all data necessary to complete the following operations
            // ADD -> UserAccount::add_password() ; this will need the struct password as apart of the operation struct
            // MODIFY -> UserAccount::modify_password() ; this will need the password_id, modifyType, and the string val
            // REMOVE -> UserAccount::remove_password() ; this willl need password_id
            // VIEW -> UserAccount::view_password() ; this will need password_id

      
        user_account_lock.lock();
        // !!CRITICAL CODE!! //
        user_account_lock.unlock();

       

        // EXIT Loop iteration
        this->operation_event_state = false;
        this->current_operation = nullptr;
        operation_event_lock.unlock();

        // TODO: add some an exit condition
        // thus if the app closing, break this while loop 
    }

}