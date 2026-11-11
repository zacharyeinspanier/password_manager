#include "./objects/userAccount.cpp"
#include "./objects/displayContent.cpp"
#include "./structs/password.cpp"
#include "./structs/operations.cpp"

std::mutex UserAccount::user_data_mutex;
std::vector<password> UserAccount::initial_user_data;
int UserAccount::max_password_id = 10;

/// @brief Launches the application with the given user account and database path.
/// @param username The username of the user account.
/// @param user_id The ID of the user account.
/// @param db_path The path to the database file.
/// @return A pointer to the DisplayContent object representing the application.
DisplayContent * launch_app(const std::string username, const int user_id, std::string *db_path){
    return new DisplayContent(username, user_id, db_path);
}
