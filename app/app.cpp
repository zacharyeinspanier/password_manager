#include <iostream>
#include <condition_variable>
#include <thread>
#include "./objects/userAccount.cpp"
#include "./objects/displayContent.cpp"


std::mutex user_account_mutex; 
UserAccount usr_acc;


// Initialize all variables
// start threads
// Create DisplayContent and return
DisplayContent launch_app(UserAccount acc, std::condition_variable * data_cv, bool * data_store_event);