# password_manager

## Start App (MAC OS)

To use enviroment variables, an env file must be sourced and the qt_create.app must be run from the same terminal. 

1. rename `.env.sample` to `.env`
2. update the var `DP_PATH` to a absolute path to database file. Just get the absolute path to ./db/user_accounts.bd. 
3. run the command `source .env`
4. change directory to qt folder
    - `cd /Users/$USER/qt`
5. open qt `open "Qt Creator.app"`

## Future Work

1. **Password encryption:** Encrypt passwords before storing them in the database, and decrypt a password only when the user asks to view it.
2. **SQL injection prevention:** Login and save operations currently build SQL queries from user input, which leaves them open to SQL injection. Switch to parameterized queries so input is always treated strictly as data.
3. **Search improvements:** Make search faster and more responsive, and improve match accuracy across name, URL, and description fields.

**NOTE** 
This project was compiled using `-std=c++20`