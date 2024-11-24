#ifndef APP // APP include gurard: app is only included once. 
#define APP

#include <iostream>
#include <vector>
#include <thread>
#include <condition_variable>
#include <dataTypes.cpp>

mutex user_account_mutex; 

bool operation_event;
condition_variable operation_cv;
mutex operation_mutex;
Operation operations;

bool search_event;
string search_term;
mutex search_term_mutex;
condition_variable search_term_cv;

// thread will need access to user account
// passwords will have unique id number to retrieve them from hash map
void operation_event();

// this thread will search through the list of passwords
// search results will then be passed to display content

// will need reference to user account to get read access to passwords
// pointers to password struct are added to list
// call back to display_content::reload_display_list()
void process_search_bar();


// when operations happen, data needs to be stored in the data base
// or every so often data needs to be stored
// or maybe on a button click
void periodic_data_store();

class DisplayContent{
    private:
        vector<shared_ptr<password>> display_passwords;
        condition_variable * operation_cv_ptr;
        condition_variable * search_term_cv_ptr;

    public:
    DisplayContent(vector<shared_ptr<password>> display_passwords, condition_variable * operation_cv_ptr, condition_variable * search_term_cv_ptr);
    
    // This function will be called by process_search_bar() after a search is complete
    // the private member display_passwords will be updated to be the result of the search
    void reload_display_list();

    vector<password> get_display_list();

    // use as call back fn
    // queue opteration and notify cond var
    void queue_operation();
    
    // This function will update search term
    // and notify the cond var
    void search(); 
};


// Initialize all variables
// start threads
// create_display content
void launch_app();

#endif