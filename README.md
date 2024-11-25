# password_manager

## Start App (MAC OS)

To use enviroment variables, an env file must be sourced and the qt_create.app must be run from the same terminal. 

1. rename `.env.sample` to `.env`
2. update the var `DP_PATH` to a absolute path to database file. Just get the absolute path to ./db/user_accounts.bd. 
3. run the command `source .env`
4. change directory to qt folder
    - `cd /Users/$USER/qt`
5. open qt `open Qt_Creator.app`
    - Users may need to rename the app `Qt Creator.app` to `Qt_Creator.app`