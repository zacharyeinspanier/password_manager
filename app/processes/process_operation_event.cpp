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

        user_account_lock.lock();
        // !!CRITICA CODE!! //
        switch (current_operation.operation_type)
        {
        case operationType::NONE:
        break;
        case operationType::ADD:
            this->usr_acc->add_password(&current_operation.curr_password);
            break;
        case operationType::REMOVE:
            this->usr_acc->remove_password(current_operation.curr_password.p_id);
            break;
        case operationType::MODIFY:
             this->usr_acc->modify_password(&current_operation.curr_password);
            break;
        default:
            break;
        }

        user_account_lock.unlock();

        // EXIT CONDITION
        // only exit if the queue is empty
        operation_exit_lock.lock();
        if (this->operation_loop_exit && this->operation_queue.empty())
        {
            operation_exit_lock.unlock();
            break;
        }
        operation_exit_lock.unlock();

        if (this->operation_queue.empty())
        {
            this->operation_event_state = false;
        }

        operation_event_lock.unlock();
    }
}