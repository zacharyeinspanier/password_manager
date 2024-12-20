#include "../objects/displayContent.hpp"

void DisplayContent::operation_process()
{
    std::unique_lock<std::mutex> operation_event_lock(this->operation_mutex, std::defer_lock);
    std::unique_lock<std::mutex> operation_exit_lock(this->operation_loop_mutex, std::defer_lock);
    std::unique_lock<std::mutex> user_account_lock(this->user_account_mutex, std::defer_lock);

    while (true)
    {
        operation_event_lock.lock();
        while (this->operation_event_state == false && this->operation_queue.empty())
        {
            this->operation_cv.wait(operation_event_lock);
        }

        operation current_operation = this->operation_queue.front();
        this->operation_queue.pop();

        // EXIT CONDITION
        operation_exit_lock.lock();
        if (this->operation_loop_exit)
        {
            operation_exit_lock.unlock();
            break;
        }
        operation_exit_lock.unlock();

        user_account_lock.lock();
        // !!CRITICA CODE!! //
        switch (current_operation.operation_type)
        {
        case operationType::ADD:
            this->usr_acc->add_password(&current_operation.new_password);
            break;
        case operationType::REMOVE:
            this->usr_acc->remove_password(current_operation.new_password.p_id);
            break;
        case operationType::MODIFY:
            switch (current_operation.modify_type)
            {
            case modifyType::MODIFY_DESCRIPTION:
                this->usr_acc->modify_password(current_operation.new_password.p_id, current_operation.new_password.description, modifyType::MODIFY_DESCRIPTION);
                break;
            case modifyType::MODIFY_PASSWORD:
                this->usr_acc->modify_password(current_operation.new_password.p_id, current_operation.new_password.encryped_password, modifyType::MODIFY_PASSWORD);
                break;
            case modifyType::MODIFY_NONE:
                // TODO this is an error
                // On the otherhand... It is unlikley
                break;
            }
            break;
        case operationType::VIEW:
            this->usr_acc->view_password(current_operation.new_password.p_id);
            break;
        }

        user_account_lock.unlock();

        if(this->operation_queue.empty()){
            this->operation_event_state = false;
        }

        operation_event_lock.unlock();
    }
}