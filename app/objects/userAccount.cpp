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
        std::shared_ptr<password> pass_id_map(prt_password_cpy);
        std::shared_ptr<password> pass_url_map_ptr(prt_password_cpy);
        std::shared_ptr<password> pass_desc_map(prt_password_cpy);
        std::shared_ptr<password> pass_usr_map(prt_password_cpy);
        try{
            this->descrioption_map.at(prt_password_cpy->description).push_back(pass_desc_map);
            this->url_map.at(prt_password_cpy->url).push_back(pass_url_map_ptr);
            this->username_map.at(prt_password_cpy->username).push_back(pass_usr_map);
            this->pass_id_map[prt_password_cpy->p_id] = pass_id_map;

        }catch (...) {
            std::cout << "could not insert password" << std::endl;
            throw;
        }

    }
}

void UserAccount::add_password(password new_password){
    password * prt_new_password = &new_password;
    std::shared_ptr<password> pass_id_map(prt_new_password);
    std::shared_ptr<password> pass_url_map_ptr(prt_new_password);
    std::shared_ptr<password> pass_desc_map(prt_new_password);
    std::shared_ptr<password> pass_usr_map(prt_new_password);
   

    try{
        this->descrioption_map.at(prt_new_password->description).push_back(pass_desc_map);
        this->url_map.at(prt_new_password->url).push_back(pass_url_map_ptr);
        this->username_map.at(prt_new_password->username).push_back(pass_usr_map);
        this->pass_id_map[prt_new_password->p_id] = pass_id_map;

    }catch (...) {
        std::cout << "could not insert password" << std::endl;
        throw;
    }

}

void UserAccount::remove_password(int p_id){

    if(this->pass_id_map.contains(p_id)){
        // extract key/value pair from map
        auto node_handle = this->pass_id_map.extract(p_id);
        std::shared_ptr<password> removed_password = node_handle.mapped();
        if(this->url_map.contains(removed_password->url)){
            std::vector<std::shared_ptr<password>> url_vec = this->url_map.at(removed_password->url);
            int remove_index = -1;
            for(int i = 0; i < url_vec.size(); ++i){
                if(url_vec[i]->p_id == p_id){
                    remove_index = i;
                }
            }
            if(remove_index != -1){
                url_vec.erase(url_vec.begin() + remove_index);
            }
   
        }
        if(this->descrioption_map.contains(removed_password->description)){
            std::vector<std::shared_ptr<password>> descrioption_vec = this->descrioption_map.at(removed_password->description);
            int remove_index = -1;
            for(int i = 0; i < descrioption_vec.size(); ++i){
                if(descrioption_vec[i]->p_id == p_id){
                    remove_index = i;
                }
            }
            if(remove_index != -1){
                descrioption_vec.erase(descrioption_vec.begin() + remove_index);
            }
            
        }
        if(this->username_map.contains(removed_password->username)){
            std::vector<std::shared_ptr<password>>  username_vec = this->descrioption_map.at(removed_password->username);
            int remove_index = -1;
            for(int i = 0; i < username_vec.size(); ++i){
                if(username_vec[i]->p_id == p_id){
                    remove_index = i;
                }
            }
            if(remove_index != -1){
                username_vec.erase(username_vec.begin() + remove_index);
            }
        }

    }else{
        std::cout << "Password does not exist." << std::endl;
    }
}

// userAccount Tests

// Test 1: Add

// Test 2: Remove

// Test 3: Modify