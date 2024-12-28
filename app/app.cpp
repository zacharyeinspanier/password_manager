#include "./objects/userAccount.cpp"
#include "./objects/displayContent.cpp"
#include "./structs/password.cpp"
#include "./structs/operations.cpp"

DisplayContent *DisplayContent::instance_ptr = nullptr;
std::mutex DisplayContent::display_content_mutex;
std::mutex UserAccount::user_data_mutex;
std::vector<password> UserAccount::initial_user_data;

DisplayContent * launch_app(const std::string username, const int user_id, std::string *db_path){
    DisplayContent * display = DisplayContent::get_instance(db_path);
    display-> start_processes();
    return display;
}

void quit_app(DisplayContent * display){
    display->stop_processes();
    DisplayContent::deinitialize_instance();
}