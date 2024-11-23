#include <iostream>

namespace app{
    // bool operations
    // cond_var operation_evet
    // mutex operation mutex
    // Queue<Operations> operations_queue

    void operation_event(){
        // thread will need access to user account
        // passwords will have unique id number to retrieve them from hash map
    }

    // std::string search_term
    // mutex search_term_mutex
    // cond_var search_term_cond_var
    // bool search_processed

    void process_search_bar(){
        // this thread will search through the list of passwords
        // search results will then be passed to display content

        // will need reference to user account to get read access to passwords
        // pointers to password struct are added to list
        // call back to display_content::reload_display_list()

    }

    void launch_app(){
        // Initialize all variables
        // start threads
        // create_display content
    }


    class display_content{
        // This class will be passed to the user account window
        
        // list<Passwords *> display list

        // void reload_display_list(){}
        // List<> get_display_list(){}

        void queue_operation(){
            // use as call back fn
            // queue opteration and notify cond var
        } 

        void search(){
            // This function will update search term
            // and notify the cond var
        }
    
    };
};