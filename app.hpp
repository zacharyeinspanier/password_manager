#ifndef APP // APP include gurard: app is only included once. 
#define APP

#include <iostream>
#include <vector>
#include <thread>
#include <condition_variable>
#include <dataTypes.cpp>
#include <userAccount.hpp>

mutex user_account_mutex; 
UserAccount usr_acc;

bool operation_event;
condition_variable operation_cv;
mutex operation_mutex;
operation operations;

bool search_event;
string search_term;
mutex search_term_mutex;
condition_variable search_term_cv;

bool * data_store_event;
mutex data_store_mutex;
condition_variable * data_store_cv;

mutex search_results_mutex;
vector<shared_ptr<password>> search_results;


// This thread will process operations such as add, remove, modify, and view
//
// The thread will the object use UserAccount::<public operation functions>
// The thread must lock the mutex user_account_mutex in order to mutate UserAccount
// 
// Look at: operation struct in dataTypes.cpp
void operation_event();


// This thread will process searches and populate the vector search_results
// 
// Searches will find passwords with the string search_term in their URL member
// Partical matches are acceptable 
//
// The thread must lock the mutex user_account_mutex in order to view UserAccount data
void process_search_bar();

// This thread is responsible to storing data in the data base
//
// periodically the data base must be updated so save user data
// 
// Currently I am unsure how this will happen...
// Questions:
// How will data be updated to match what is in user account??
// What SQL querey will be used to do this update?
// Will data need to be fomatted so that insert and modify quereys can be run easly? 

// IDEA:
    // every 2 seconds the following sequence occurs
    // lock userAccount mutex
    // copy all data in UserAccount::pass_id_map to a temporary vector
    // call cond_var.notify() to wake sql store thread
        // thread will then iterate over vector and store/update data
    // release lock
void periodic_data_store();

class DisplayContent{
    private:
        vector<password> display_passwords;

        void reload_display_list();
    public:
    DisplayContent(vector<shared_ptr<password>> display_passwords, condition_variable * operation_cv_ptr, condition_variable * search_term_cv_ptr);

    vector<password> get_display_list();

    // update opteration variable and notify cond var
    void operation_event();
    
    // update search term variable and notify the cond var
    void search_event(); 
};


// Initialize all variables
// start threads
// Create DisplayContent and return
DisplayContent launch_app(UserAccount acc, condition_variable * data_cv, bool * data_store_event);


// App work flow
// poll for input 500 ms
// process action
// DisplayContent::get_display_list()
    // DisplayContent::reload_display_list()

#endif