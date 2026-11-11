#include "../objects/displayContent.hpp"

void DisplayContent::search_bar_process()
{
    std::unique_lock<std::mutex> search_term_lock(this->search_term_mutex, std::defer_lock);
    std::unique_lock<std::mutex> search_loop_lock(this->search_loop_mutex, std::defer_lock);
    std::unique_lock<std::mutex> user_account_lock(this->user_account_mutex, std::defer_lock);
    std::unique_lock<std::mutex> search_result_lock(this->search_result_mutex, std::defer_lock);

    while (true)
    {
        search_term_lock.lock();
        while (this->search_event_state == false && this->search_term == nullptr)
        {
            this->search_term_cv.wait(search_term_lock);
        }

        // EXIT LOOP
        search_loop_lock.lock();
        if (this->search_loop_exit)
        {
            search_loop_lock.unlock();
            break;
        }
        search_loop_lock.unlock();

        // !!CRITICAL CODE!! //
        user_account_lock.lock();
        std::unordered_map<int, password> unordered_search_result = this->usr_acc->search(*this->search_term);
        user_account_lock.unlock();

        // !!CRITICAL CODE!! //
        search_result_lock.lock();
        for (const auto &item : unordered_search_result)
        {
            if (this->search_result.contains(item.first))
            {
                this->search_result.at(item.first).username = item.second.username;
                this->search_result.at(item.first).encryped_password = item.second.encryped_password;
                this->search_result.at(item.first).url = item.second.url;
                this->search_result.at(item.first).description = item.second.description;
            }
            else
            {
                this->search_result.emplace(item.first, item.second);
            }
        }

        for (auto it = this->search_result.begin(); it != this->search_result.end();)
        {
            if (!unordered_search_result.contains(it->first))
            {
                it = this->search_result.erase(it);
            }
            else
            {
                ++it;
            }
        }
        search_result_lock.unlock();

        this->search_event_state = false;
        this->search_term = nullptr;
        search_term_lock.unlock();
    }
}