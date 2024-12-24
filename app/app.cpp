#include "./objects/userAccount.cpp"
#include "./objects/displayContent.cpp"
#include "./structs/password.cpp"
#include "./structs/operations.cpp"

DisplayContent *DisplayContent::instance_ptr = nullptr;
std::mutex DisplayContent::display_content_mutex;
UserAccount *UserAccount::instance_ptr = nullptr;
std::mutex UserAccount::user_acc_mutex;

DisplayContent * launch_app(std::string const username, const int user_id, const std::vector<password> *user_data){
    UserAccount::initialize_instance(username, user_id, user_data);
    DisplayContent * display = DisplayContent::get_instance();
    display-> start_processes();
    return display;
}

void quit_app(DisplayContent * display){
    display->stop_processes();
}