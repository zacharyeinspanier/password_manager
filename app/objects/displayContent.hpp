#ifndef DISPLAYCONTENT // DISPLAYCONTENT include gurard: displayContent is only included once. 
#define DISPLAYCONTENT

#include <iostream>
#include <vector>
#include <condition_variable>
#include "./userAccount.cpp"
#include "../structs/operations.hpp"
#include "../structs/password.cpp"

////////////////////////////////////////////////////////////////////////////////////////////////////
/// @brief This class holds the Content the app will display to users. 
///
/// This class is a singleton. There is only one display window is active at a time. 
////////////////////////////////////////////////////////////////////////////////////////////////////
class DisplayContent{
    private:

        static DisplayContent* instance_ptr;
        static std::mutex display_content_mutex;

        std::mutex display_passwords_mutex;
        std::vector<password> display_passwords;
        std::mutex user_account_mutex; 
        const UserAccount * usr_acc;

        // Operations
        bool operation_event_state;
        std::condition_variable operation_cv;
        std::mutex operation_mutex;
        operation * current_operation;

        // Search Bar
        bool search_event_state;
        std::string * search_term;
        std::mutex search_term_mutex;
        std::condition_variable search_term_cv;

        // Methods
        DisplayContent();

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


    public:
        // Delete the copy constructor
        DisplayContent(const DisplayContent& obj) = delete;
        static DisplayContent* get_instance();
        void start_processes();
        std::vector<password> get_display_list();
        void operation_event(operation &new_operation);
        void search_event(std::string &new_search_term); 
};

#endif