#include "userAccount.hpp"

// userAccount implementation
UserAccount::UserAccount(std::string username, int user_id){
    this->account_username = username;
    this->user_id = user_id;
}

UserAccount::UserAccount(std::string username, int user_id, std::vector<password> user_data){
    this->account_username = username;
    this->user_id = user_id;
    //each password must be wrapped in a shared pinter

    for(int i = 0; i < user_data.size(); ++i){
        password cpy_password_for_maps;
        password * prt_password_cpy = &cpy_password_for_maps;
        memcpy(prt_password_cpy, &user_data[i], sizeof(user_data[i]));
        std::shared_ptr<password> pass_ptr(prt_password_cpy);
        try{
            this->descrioption_map.at(pass_ptr->description).push_back(pass_ptr.get());
            this->url_map.at(pass_ptr->url).push_back(pass_ptr.get());
            this->username_map.at(pass_ptr->username).push_back(pass_ptr.get());
            this->pass_id_map[pass_ptr->p_id] = pass_ptr;

        }catch (...) {
            std::cout << "could not insert password" << std::endl;
            throw;
        }

    }
}

// userAccount Tests

// Test 1: Add

// Test 2: Remove

// Test 3: Modify