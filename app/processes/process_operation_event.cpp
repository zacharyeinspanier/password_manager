#include "../objects/displayContent.hpp"


void DisplayContent::operation_process(){
    std::unique_lock<std::mutex> operation_event_lock(this->operation_mutex, std::defer_lock);
    std::unique_lock<std::mutex> operation_exit_lock(this->operation_loop_mutex, std::defer_lock);
    std::unique_lock<std::mutex> user_account_lock(this->user_account_mutex, std::defer_lock);
   
    while(true){
        operation_event_lock.lock();
        while(this->operation_event_state == false || this->current_operation == nullptr){
            this->operation_cv.wait(operation_event_lock);
        }

        // EXIT CONDITION
        operation_exit_lock.lock();
        if(this->operation_loop_exit){
            operation_exit_lock.unlock();
            break;
        }
        operation_exit_lock.unlock();


        user_account_lock.lock();
        // !!CRITICA CODE!! //
        switch(this->current_operation->operation_type){
            case operationType::ADD:
                this->usr_acc->add_password(&this->current_operation->new_password);
            case operationType::REMOVE:
                this->usr_acc->remove_password(this->current_operation->new_password.p_id);
            case operationType::MODIFY:
                switch(this->current_operation->modify_type){
                    case modifyType::MODIFY_DESCRIPTION:
                        this->usr_acc->modify_password(this->current_operation->new_password.p_id, this->current_operation->new_password.description, modifyType::MODIFY_DESCRIPTION);
                    case modifyType::MODIFY_PASSWORD:
                        this->usr_acc->modify_password(this->current_operation->new_password.p_id, this->current_operation->new_password.encryped_password, modifyType::MODIFY_PASSWORD);
                    case modifyType::MODIFY_NONE:
                        // TODO this is an error
                        // On the otherhand... It is unlikley
                        break; 
                }
            case operationType::VIEW:
                this->usr_acc->view_password(this->current_operation->new_password.p_id);
        }
        user_account_lock.unlock();

        this->operation_event_state = false;
        this->current_operation = nullptr;
        operation_event_lock.unlock();

    }

}