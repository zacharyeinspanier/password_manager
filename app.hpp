#ifndef APP // APP include gurard: app is only included once. 
#define APP

#include <iostream>
#include <vector>
#include <thread>
#include <condition_variable>
#include <dataTypes.cpp>
#include <userAccount.hpp>

mutex user_account_mutex; 
UserAccount * usr_acc;

bool operation_event;
condition_variable operation_cv;
mutex operation_mutex;
Operation operations;

bool search_event;
string search_term;
mutex search_term_mutex;
condition_variable search_term_cv;

mutex search_results_mutex;
vector<shared_ptr<password>> search_results;

// thread will need access to user account
// passwords will have unique id number to retrieve them from hash map
void operation_event();

// this thread will search through the list of passwords
// search results will then be passed to display content

// will need reference to user account to get read access to passwords
// pointers to password struct are added to list
void process_search_bar();

// when operations happen, data needs to be stored in the data base
// or every so often data needs to be stored
// or maybe on a button click
void periodic_data_store();

class DisplayContent{
    private:
        vector<password> display_passwords;

        void reload_display_list();
    public:
    DisplayContent(vector<shared_ptr<password>> display_passwords, condition_variable * operation_cv_ptr, condition_variable * search_term_cv_ptr);

    vector<password> get_display_list();

    // queue opteration and notify cond var
    void operation_event();
    
    // and notify the cond var
    void search_event(); 
};


// Initialize all variables
// start threads
// create_display content
DisplayContent launch_app();


// App work flow
// poll for input 500 ms
// process action
// DisplayContent::get_display_list()
    // DisplayContent::reload_display_list()

#endif