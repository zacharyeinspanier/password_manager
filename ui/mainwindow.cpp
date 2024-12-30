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
    ui->setupUi(this);
    user_loggedin = false;
    user_content = nullptr;
    this->account_create_and_login_display();
}

MainWindow::~MainWindow()
{
    delete ui;
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
    this->ui->password_input->setEchoMode(QLineEdit::Password);
    this->ui->username_input->show();
    this->ui->username_input->clear();
    this->ui->password_label->show();
    this->ui->username_label->show();

    // UI object to hide
    this->ui->search_btn->hide();
    this->ui->search_input->hide();
    this->ui->password_add_btn->hide();
    this->ui->password_remove_btn->hide();
    this->ui->password_modify_btn->hide();
    this->ui->password_view_btn->hide();
    this->ui->password_table->hide();
    this->ui->logout_btn->hide();
}

void MainWindow::user_account_display()
{
    // UI object to hide
    this->ui->create_acc_btn->hide();
    this->ui->login_btn->hide();
    this->ui->password_input->hide();
    this->ui->username_input->hide();
    this->ui->password_label->hide();
    this->ui->username_label->hide();

    // UI object to show
    this->ui->search_btn->show();
    this->ui->search_input->show();
    this->ui->search_input->clear();
    this->ui->password_add_btn->show();
    this->ui->password_remove_btn->show();
    this->ui->password_modify_btn->show();
    this->ui->password_view_btn->show();
    this->ui->password_table->show();
    this->ui->logout_btn->show();
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

void MainWindow::login(std::string username, int user_id)
{
    std::string db_path_std_string = this->db_path->toStdString();
    this->user_content = launch_app(username, user_id, &db_path_std_string);
    this->user_loggedin = true;
    this->user_account_display();
}

void MainWindow::logout()
{
    delete this->user_content;
    this->user_content = nullptr;
    this->user_loggedin = false;
    this->account_create_and_login_display();
}

void MainWindow::on_search_btn_clicked()
{
    if (this->user_loggedin == false || this->user_content == nullptr)
    {
        QString warningMsg = "Error: user is not logged in!!";
        display_message(warningMsg, QMessageBox::Warning);
        return;
    }
}

void MainWindow::on_logout_btn_clicked()
{
    if (this->user_loggedin == false || this->user_content == nullptr)
    {
        QString warningMsg = "Error: user is not logged in!!";
        display_message(warningMsg, QMessageBox::Warning);
        return;
    }
    this->logout();
}

void MainWindow::on_password_add_btn_clicked()
{
    if (this->user_loggedin == false || this->user_content == nullptr)
    {
        QString warningMsg = "Error: user is not logged in!!";
        display_message(warningMsg, QMessageBox::Warning);
        return;
    }

    //1: create input field box
    //2: capture user input
    //3: create an ADD operation
    //4: call operation event
    //5: call reset display

}

void MainWindow::on_password_remove_btn_clicked()
{
    if (this->user_loggedin == false || this->user_content == nullptr)
    {
        QString warningMsg = "Error: user is not logged in!!";
        display_message(warningMsg, QMessageBox::Warning);
        return;
    }

    //1: get the item
        // this->ui->password_table->currentItem();
    //2: get the create a temporary password
    //3: create remove operation
    //4: call operation event
    //5: call reset display
}

void MainWindow::on_password_modify_btn_clicked()
{
    if (this->user_loggedin == false || this->user_content == nullptr)
    {
        QString warningMsg = "Error: user is not logged in!!";
        display_message(warningMsg, QMessageBox::Warning);
        return;
    }
    //1: get the item
    // this->ui->password_table->currentItem();
    //2: get the create a temporary password
    //3: create movidy operation
    //4: call operation event
    //5: call reset display
}

void MainWindow::on_password_view_btn_clicked()
{
    if (this->user_loggedin == false || this->user_content == nullptr)
    {
        QString warningMsg = "Error: user is not logged in!!";
        display_message(warningMsg, QMessageBox::Warning);
        return;
    }

    //1: get the item
    // this->ui->password_table->currentItem();
    //2: get the create a temporary password
    //3: create movidy operation
    //4: call operation event
    //5: display password is a qt box widget
        // unsure how the password will be set to ui...
}
