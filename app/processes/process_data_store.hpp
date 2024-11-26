#ifndef DATASTORE 
#define DATASTORE

#include <iostream>
#include <thread>
#include <condition_variable>

bool * data_store_event;
std::mutex data_store_mutex;
std::condition_variable * data_store_cv;

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

#endif