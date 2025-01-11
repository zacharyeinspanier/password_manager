#include "mainwindow.h"
#include "./ui_mainwindow.h"
#include "../app/app.cpp"

#include <QMessageBox>
#include <cstdlib>
#include <QSqlQuery>
#include <QSqlRecord>
#include <QVariant>

void display_message(QString msg, QMessageBox::Icon icon)
{
    QMessageBox infoBox;
    infoBox.setIcon(icon);
    infoBox.setText(msg);
    infoBox.setStandardButtons(QMessageBox::Ok);
    infoBox.exec();
}

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), ui(new Ui::MainWindow)
{
    std::function<void(operation)> temp_cb = [](operation temp_operation) ->void{};
    password temp;

    ui->setupUi(this);
    user_loggedin = false;
    search_active = false;
    user_content = nullptr;

    // Create form objects
    this->password_form = new add_password(this, temp_cb);
    this->view_password_form = new view_password(this, temp, temp_cb);

    // UI Objects initial state
    this->ui->password_input->setEchoMode(QLineEdit::Password);
    {
        std::lock_guard<std::mutex> ui_table_lock(this->table_mutex);
        this->ui->password_table->setColumnCount(3);
        this->ui->password_table->setColumnWidth(0, 100);
        this->ui->password_table->setColumnWidth(1, 200);
        this->ui->password_table->setColumnWidth(2, 300);
    }

    // Set default display
    this->account_create_and_login_display();
}

MainWindow::~MainWindow()
{
    this->shutdown_app();

    {
        std::lock_guard<std::mutex> user_content_lock(this->user_content_mutex);
        if(this->user_content != nullptr){
            delete this->user_content;
        }
    }

    delete this->password_form;
    delete this->view_password_form;
    delete this->ui;
}

void MainWindow::SetDBPath(QString *env_db_path)
{
    this->db_path = env_db_path;
}

void MainWindow::account_create_and_login_display()
{
    // UI object to show
    this->ui->create_acc_btn->show();
    this->ui->login_btn->show();
    this->ui->password_input->show();
    this->ui->password_input->clear();
    this->ui->username_input->show();
    this->ui->username_input->clear();
    this->ui->password_label->show();
    this->ui->username_label->show();

    // UI object to hide
    this->ui->search_btn->hide();
    this->ui->cancel_search_btn->hide();
    this->ui->cancel_search_btn->setEnabled(false);
    this->ui->search_input->hide();
    this->ui->password_add_btn->hide();
    // TODO need lock here
    {
        std::lock_guard<std::mutex> ui_table_lock(this->table_mutex);
        this->ui->password_table->hide();
    }

    this->ui->logout_btn->hide();
}

void MainWindow::user_account_display()
{
    // UI object to show
    this->ui->search_btn->show();
    this->ui->search_input->show();
    this->ui->search_input->clear();
    this->ui->password_add_btn->show();
    {
        std::lock_guard<std::mutex> ui_table_lock(this->table_mutex);
        this->ui->password_table->show();
    }
    this->ui->logout_btn->show();

    // UI object to hide
    this->ui->create_acc_btn->hide();
    this->ui->login_btn->hide();
    this->ui->password_input->hide();
    this->ui->username_input->hide();
    this->ui->password_label->hide();
    this->ui->username_label->hide();
    this->ui->cancel_search_btn->hide();
    this->ui->cancel_search_btn->setEnabled(false);
}

QSqlDatabase MainWindow::GetDatabase()
{

    QSqlDatabase db_con = QSqlDatabase::addDatabase("QSQLITE");
    db_con.setDatabaseName(*this->db_path);
    return db_con;
}

void MainWindow::on_create_acc_btn_clicked()
{
    QSqlDatabase db_con = this->GetDatabase();
    if (!db_con.open())
    {
        QString err_msg = "The Data Base could not be opened";
        display_message(err_msg, QMessageBox::Critical);
        QApplication::quit();
        return;
    }

    QString username = this->ui->username_input->text();
    QString password = this->ui->password_input->text();
    bool user_exists = true;

    // Check if user account exists
    QSqlQuery userExistsQuery(db_con);
    userExistsQuery.prepare("SELECT * FROM ACCOUNTS WHERE USERNAME = :username");
    userExistsQuery.bindValue(":username", username);
    if (userExistsQuery.exec())
    {
        user_exists = userExistsQuery.next() ? true : false;
    }
    else
    {
        QString err_msg = "Tried to query database but something went wrong";
        display_message(err_msg, QMessageBox::Critical);
        QApplication::quit();
        db_con.close();
        return;
    }

    // Insert into DB
    if (!user_exists)
    {
        // Insert user into accounts
        QSqlQuery insertUserQuery(db_con);
        insertUserQuery.prepare("INSERT INTO ACCOUNTS (USERNAME, PASSWORD) VALUES (:username, :password)");
        insertUserQuery.bindValue(":username", username);
        insertUserQuery.bindValue(":password", password);

        if (insertUserQuery.exec())
        {
            QString info_msg = "Account for user" + username + " was successfully created";
            display_message(info_msg, QMessageBox::Information);
        }
        else
        {
            QString info_msg = "could not add user, something went wrong";
            display_message(info_msg, QMessageBox::Information);
            QApplication::quit();
            db_con.close();
            return;
        }
    }
    else
    {
        QString err_msg = "The username " + username + " already exists. Pick a new username";
        this->ui->username_input->setText("");
        this->ui->password_input->setText("");
        display_message(err_msg, QMessageBox::Warning);
    }
    db_con.close();
}

void MainWindow::on_login_btn_clicked()
{
    QSqlDatabase db_con = this->GetDatabase();
    if (!db_con.open())
    {
        QString err_msg = "The Data Base could not be opened";
        display_message(err_msg, QMessageBox::Critical);
        QApplication::quit();
        return;
    }

    QString username = this->ui->username_input->text();
    QString password = this->ui->password_input->text();

    QSqlQuery userAuthenticateQuery(db_con);
    userAuthenticateQuery.prepare("SELECT * FROM ACCOUNTS WHERE USERNAME = :username AND PASSWORD = :password");
    userAuthenticateQuery.bindValue(":username", username);
    userAuthenticateQuery.bindValue(":password", password);

    if (userAuthenticateQuery.exec())
    {
        if (userAuthenticateQuery.next())
        {
            QSqlRecord rec = userAuthenticateQuery.record();
            int username_index = rec.indexOf("USERNAME");
            int password_index = rec.indexOf("PASSWORD");
            int userid_index = rec.indexOf("USERID");
            QString username_query = userAuthenticateQuery.value(username_index).toString();
            QString password_query = userAuthenticateQuery.value(password_index).toString();
            int userid_query = userAuthenticateQuery.value(userid_index).toInt();

            this->login(username_query.toStdString(), userid_query);
        }
        else
        {
            QString err_msg = "There is no account for the username " + username + ". Please create an account.";
            display_message(err_msg, QMessageBox::Information);
        }
    }
    else
    {
        QString err_msg = "Tried to query database but something went wrong";
        display_message(err_msg, QMessageBox::Critical);
        QApplication::quit();
        db_con.close();
        return;
    }

    db_con.close();
}

void MainWindow::update_display_table()
{
    // sleep 3 s

    // aquire display passwords lock
    // aquire search term lock
    // aquire user content lock

    if(this->search_active){
        this->user_content->search_event(this->search_term);
    }else{
        this->user_content->reset_display_list();
    }
    // release search term lock
    this->table_display_items.clear();

    if(this->search_active){
        this->table_display_items = this->user_content->get_search_result();
    }
    else{
        this->table_display_items = this->user_content->get_display_list();
    }
    // release user content lock
    // release search term lock



    // aquire table lock
    this->ui->password_table->clearContents();
    this->ui->password_table->setRowCount(this->table_display_items.size());

    for(int i = 0; i < this->table_display_items.size(); ++i){
        this->ui->password_table->setItem(i, 0, new QTableWidgetItem(QString::fromStdString(this->table_display_items[i].username)));
        this->ui->password_table->setItem(i, 1, new QTableWidgetItem(QString::fromStdString(this->table_display_items[i].url)));
        this->ui->password_table->setItem(i, 2, new QTableWidgetItem(QString::fromStdString(this->table_display_items[i].description)));
    }
    // release table lock
    // release display passwords lock

}

void MainWindow::login(std::string username, int user_id)
{
    std::string db_path_std_string = this->db_path->toStdString();

    {
        std::lock_guard<std::mutex> ui_table_lock(this->table_mutex);
        this->ui->password_table->clear();
    }

    {
        std::lock_guard<std::mutex> display_items_lock(this->display_items_mutex);
        this->table_display_items.clear();
    }

    {
        std::lock_guard<std::mutex> search_term_lock(this->search_term_mutex);
        this->search_term = "";
        this->search_active = false;
    }

    {
        std::lock_guard<std::mutex> exit_refresh_thread_lock(this->exit_refresh_display_thread_mtx);
        this->exit_refresh_display_thread = false;
    }

    {
        std::lock_guard<std::mutex> user_content_lock(this->user_content_mutex);
        this->user_content = launch_app(username, user_id, &db_path_std_string);
    }


    std::function<void(operation)> queue_operation_callback = [this](operation new_operation) -> void{
        std::lock_guard<std::mutex> user_content_lock(this->user_content_mutex);
        this->user_content->operation_event(new_operation);
    };

    this->view_password_form->set_callback(queue_operation_callback);
    this->password_form->set_callback(queue_operation_callback);
    this->user_loggedin = true;
    this->user_account_display();

    // TODO: Here the Thread will be launched
    this->update_display_table();
}

void MainWindow::logout()
{
    // Command refresh thread to exit
    {
        std::lock_guard<std::mutex> exit_refresh_thread_lock(this->exit_refresh_display_thread_mtx);
        this->exit_refresh_display_thread = true;
    }
    this->refresh_display_thread.join();

    // Clear display table
    {
        std::lock_guard<std::mutex> ui_table_lock(this->table_mutex);
        this->ui->password_table->clear();
    }

    // Clear display items vector
    {
        std::lock_guard<std::mutex> display_items_lock(this->display_items_mutex);
        this->table_display_items.clear();
    }

    // Reset search term
    {
        std::lock_guard<std::mutex> search_term_lock(this->search_term_mutex);
        this->search_term = "";
        this->search_active = false;
    }


    std::function<void(operation)> temp_cb = [](operation new_operation) -> void {};
    this->view_password_form->set_callback(temp_cb);
    this->password_form->set_callback(temp_cb);

    // Delete user content
    {
        std::lock_guard<std::mutex> user_content_lock(this->user_content_mutex);
        delete this->user_content;
        this->user_content = nullptr;
    }

    this->user_loggedin = false;
    this->account_create_and_login_display();
}

void MainWindow::on_search_btn_clicked()
{
    if (this->user_loggedin == false)
    {
        QString warningMsg = "Error: user is not logged in!!";
        display_message(warningMsg, QMessageBox::Warning);
        return;
    }

    std::lock_guard<std::mutex> search_term_lock(this->search_term_mutex);
    this->search_term = this->ui->search_input->text().toStdString();
    this->search_active = true;

    this->ui->search_btn->hide();
    this->ui->search_btn->setEnabled(false);

    this->ui->cancel_search_btn->show();
    this->ui->cancel_search_btn->setEnabled(true);
}


void MainWindow::on_cancel_search_btn_clicked()
{
    std::lock_guard<std::mutex> search_term_lock(this->search_term_mutex);
    this->search_term = "";
    this->search_active = false;
    this->ui->search_input->setText("");

    this->ui->cancel_search_btn->show();
    this->ui->cancel_search_btn->setEnabled(false);

    this->ui->search_btn->show();
    this->ui->search_btn->setEnabled(true);
}

void MainWindow::on_logout_btn_clicked()
{
    if (this->user_loggedin == false)
    {
        QString warningMsg = "Error: user is not logged in!!";
        display_message(warningMsg, QMessageBox::Warning);
        return;
    }
    this->logout();
}

void MainWindow::on_password_add_btn_clicked()
{
    if (this->user_loggedin == false)
    {
        QString warningMsg = "Error: user is not logged in!!";
        display_message(warningMsg, QMessageBox::Warning);
        return;
    }

    this->password_form->show();
}


void MainWindow::on_password_table_cellDoubleClicked(int row, int column)
{
    {
        std::lock_guard<std::mutex> ui_table_lock(this->table_mutex);
        std::lock_guard<std::mutex> display_items_lock(this->display_items_mutex);
        this->view_password_form->set_current_password(this->table_display_items[row]);
    }
    this->view_password_form->load_current_password();
    this->view_password_form->show();
}

void MainWindow::shutdown_app(){
    {
        std::lock_guard<std::mutex> exit_refresh_display_thread_lock(this->exit_refresh_display_thread_mtx);
        this->exit_refresh_display_thread = true;
    }

    // TODO: check that thread is running, Calling join on empty thread have problems.
    this->refresh_display_thread.join();
}



