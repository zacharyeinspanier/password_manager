#ifndef DISPLAYCONTENT // APP include gurard: app is only included once. 
#define DISPLAYCONTENT

#include <iostream>
#include <vector>
#include <condition_variable>
#include "../structs/password.hpp"


class DisplayContent{
    private:
        std::vector<password> display_passwords;

        void reload_display_list();
    public:
    DisplayContent(std::vector<std::shared_ptr<password>> display_passwords, std::condition_variable * operation_cv_ptr, std::condition_variable * search_term_cv_ptr);

    std::vector<password> get_display_list();

    // update opteration variable and notify cond var
    void operation_event();
    
    // update search term variable and notify the cond var
    void search_event(); 
};

#endif