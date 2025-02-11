#ifndef DISPLAYCONTENT // DISPLAYCONTENT include gurard: displayContent is only included once.
#define DISPLAYCONTENT

#include <iostream>
#include <set>
#include <map>
#include <condition_variable>
#include <thread>
#include <queue>
#include <vector>
#include <sqlite3.h>
#include "./userAccount.hpp"
#include "../structs/operations.hpp"
#include "../structs/password.hpp"

////////////////////////////////////////////////////////////////////////////////////////////////////
/// @brief This class holds the Content the app will display to users.
///
/// This class is a singleton. There is only one display window is active at a time.
////////////////////////////////////////////////////////////////////////////////////////////////////
class DisplayContent
{
private:
    std::mutex display_passwords_mutex;
    std::mutex search_result_mutex;
    std::map<int, password> search_result;
    std::map<int, password> display_passwords;
    std::mutex user_account_mutex;
    UserAccount *usr_acc = nullptr;

    // Operations
    bool operation_event_state;
    bool operation_loop_exit;
    std::condition_variable operation_cv;
    std::mutex operation_mutex;
    std::mutex operation_loop_mutex;
    std::queue<operation> operation_queue;
    std::thread opeation_thread;

    // Search Bar
    bool search_event_state;
    bool search_loop_exit;
    std::string *search_term;
    std::mutex search_term_mutex;
    std::mutex search_loop_mutex;
    std::condition_variable search_term_cv;
    std::thread search_thread;

    // Data Store
    std::string db_path;
    std::mutex db_path_mutex;
    std::thread data_store_thread;
    std::mutex data_store_loop_mutex;
    bool data_store_exit;

    // This thread will process operations such as add, remove, modify, and view
    //
    // The thread will the object use UserAccount::<public operation functions>
    // The thread must lock the mutex user_account_mutex in order to mutate UserAccount
    //
    // Look at: operation struct in dataTypes.cpp
    void operation_process();

    // This thread will process searches and populate the vector search_results
    //
    // Searches will find passwords with the string search_term in their URL member
    // Partical matches are acceptable
    //
    // The thread must lock the mutex user_account_mutex in order to view UserAccount data
    void search_bar_process();

    void periodic_data_store();
    void start_processes();
    void stop_processes();

public:
    // Delete the copy constructor
    DisplayContent(std::string username, int user_id, std::string *db_path);
    ~DisplayContent();
    void reset_display_list();
    std::vector<password> get_display_list();
    std::vector<password> get_search_result();
    void operation_event(operation &new_operation);
    void search_event(std::string &new_search_term);
};

#endif