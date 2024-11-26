#ifndef SEARCHEVENT 
#define SEARCHEVENT

#include <iostream>
#include <vector>
#include <thread>
#include <condition_variable>
#include "../structs/password.hpp"

bool search_event;
std::string search_term;
std::mutex search_term_mutex;
std::condition_variable search_term_cv;

std::mutex search_results_mutex;
std::vector<std::shared_ptr<password>> search_results;

// This thread will process searches and populate the vector search_results
// 
// Searches will find passwords with the string search_term in their URL member
// Partical matches are acceptable 
//
// The thread must lock the mutex user_account_mutex in order to view UserAccount data
void process_search_bar();

#endif