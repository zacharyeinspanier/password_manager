#ifndef OPERATIONEVENT 
#define OPERATIONEVENT

#include <iostream>
#include <vector>
#include <thread>
#include <condition_variable>
#include "../structs/operations.hpp"

bool operation_event;
std::condition_variable operation_cv;
std::mutex operation_mutex;
operation operations;


// This thread will process operations such as add, remove, modify, and view
//
// The thread will the object use UserAccount::<public operation functions>
// The thread must lock the mutex user_account_mutex in order to mutate UserAccount
// 
// Look at: operation struct in dataTypes.cpp
void operation_event();

#endif