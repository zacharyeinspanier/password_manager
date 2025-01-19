#include "./objects/userAccount.cpp"
#include "./objects/displayContent.cpp"
#include "./structs/password.cpp"
#include "./structs/operations.cpp"

std::mutex UserAccount::user_data_mutex;
std::vector<password> UserAccount::initial_user_data;
int UserAccount::max_password_id = 10;

DisplayContent * launch_app(const std::string username, const int user_id, std::string *db_path){
    return new DisplayContent(username, user_id, db_path);
}
