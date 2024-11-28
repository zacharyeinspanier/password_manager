#include "userAccount.hpp"

// userAccount implementation
UserAccount::UserAccount(std::string username, int user_id){
    this->account_username = username;
    this->user_id = user_id;
}

UserAccount::UserAccount(std::string username, int user_id, std::vector<password> user_data){
    this->account_username = username;
    this->user_id = user_id;

    for(int i = 0; i < user_data.size(); ++i){
        this->add_password(&user_data[i]);
    }
}

void UserAccount::add_password(password * new_password){
    // Copy password
    password cpy_new_password;
    password * prt_new_password = &cpy_new_password;
    memcpy(prt_new_password, new_password, sizeof(password));

    // shared pointers
    std::shared_ptr<password> pass_id_map(prt_new_password);
    std::shared_ptr<password> pass_url_map_ptr(prt_new_password);
    std::shared_ptr<password> pass_desc_map(prt_new_password);
    std::shared_ptr<password> pass_usr_map(prt_new_password); 

    try{
        std::lock_guard<std::mutex> unordered_map_lock(this->unordered_map_mutex);
         //description map
        if(this->descrioption_map.contains(prt_new_password->description)){
                this->descrioption_map.at(prt_new_password->description).push_back(pass_desc_map);
        }else{
            std::vector<std::shared_ptr<password>> map_vec;
            map_vec.push_back(pass_desc_map);
            this->descrioption_map.emplace(std::make_pair(prt_new_password->description, map_vec));
        }

        //url map
        if(this->url_map.contains(prt_new_password->url)){
                this->url_map.at(prt_new_password->url).push_back(pass_url_map_ptr);
        }else{
            std::vector<std::shared_ptr<password>> map_vec;
            map_vec.push_back(pass_url_map_ptr);
            this->descrioption_map.emplace(std::make_pair(prt_new_password->url, map_vec));
        }

        //username map
        if(this->username_map.contains(prt_new_password->username)){
            this->username_map.at(prt_new_password->username).push_back(pass_usr_map);
        }else{
            std::vector<std::shared_ptr<password>> map_vec;
            map_vec.push_back(pass_usr_map);
            this->descrioption_map.emplace(std::make_pair(prt_new_password->username, map_vec));
        }

        this->pass_id_map[prt_new_password->p_id] = pass_id_map;

    }catch (...) {
        std::cout << "could not insert password" << std::endl;
        throw;
    }
}

void UserAccount::remove_password(int p_id){

    std::lock_guard<std::mutex> unordered_map_lock(this->unordered_map_mutex);
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

    }
}

void UserAccount::modify_password(int p_id, std::string new_value, modifyType modify_type){
    std::lock_guard<std::mutex> unordered_map_lock(this->unordered_map_mutex);
    if(this->pass_id_map.contains(p_id)){
        std::shared_ptr<password> password_ptr = this->pass_id_map[p_id];
        std::lock_guard<std::mutex> password_ptr_mutex(password_ptr->password_mutex);
        switch (modify_type) {
            case modifyType::MODIFY_DESCRIPTION:
                password_ptr->description = new_value;
                break;
            case modifyType::MODIFY_PASSWORD:
                password_ptr->password = new_value;
                break;
            default:
                std::cout << "Password does not exist.";
        }
    }
}

std::string UserAccount::view_password(int p_id){
    std::lock_guard<std::mutex> unordered_map_lock(this->unordered_map_mutex);
    if(this->pass_id_map.contains(p_id)){
        std::shared_ptr<password> password_ptr = this->pass_id_map[p_id];
        std::lock_guard<std::mutex> password_ptr_mutex(password_ptr->password_mutex);
        std::string res = password_ptr->password;

        return res;
    }
    return "Err not found";
}

std::unordered_map<int, password> UserAccount::get_data_copy(){
    std::lock_guard<std::mutex> unordered_map_lock(this->unordered_map_mutex);
    std::unordered_map<int, password> answer;
    for(auto it = this->pass_id_map.begin(); it != this->pass_id_map.end(); ++it){
        std::lock_guard<std::mutex> unordered_map_lock(it->second->password_mutex);
        password cpy_password;
        memcpy(&cpy_password, &it->second, sizeof(password));
        answer.emplace(std::make_pair(cpy_password.p_id, cpy_password));
    }
    return answer;
 }

