#include <iostream>
#include <condition_variable>
#include <thread>
#include "./objects/userAccount.cpp"
#include "./objects/displayContent.cpp"


DisplayContent* DisplayContent::instance_ptr = nullptr;
UserAccount * UserAccount::instance_ptr = nullptr;


// Initialize all variables
// start threads
// Create DisplayContent and return
DisplayContent launch_app();