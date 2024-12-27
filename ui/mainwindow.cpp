#include "mainwindow.h"
#include "./ui_mainwindow.h"
#include <QMessageBox>
#include <cstdlib>
#include <QSqlQuery>
#include <QSqlRecord>
#include <QVariant>

void display_message(QString msg, QMessageBox::Icon icon){
    QMessageBox errMsgBox;
    errMsgBox.setIcon(icon);
    errMsgBox.setText(msg);
    errMsgBox.setStandardButtons(QMessageBox::Ok);
    errMsgBox.exec();
}

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::SetDBPath(QString *env_db_path){
    this->db_path = env_db_path;
}

QSqlDatabase MainWindow::GetDatabase(){

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

    //Check if user account exists
    QSqlQuery userExistsQuery(db_con);
    userExistsQuery.prepare("SELECT * FROM ACCOUNTS WHERE USERNAME = :username");
    userExistsQuery.bindValue(":username", username);
    if(userExistsQuery.exec()){
        user_exists = userExistsQuery.next() ? true : false;
    }else{
        QString err_msg = "Tried to query database but something went wrong";
        display_message(err_msg, QMessageBox::Critical);
        QApplication::quit();
        db_con.close();
        return;
    }

    // Insert into DB
    if(!user_exists){
        //Insert user into accounts
        QSqlQuery insertUserQuery(db_con);
        insertUserQuery.prepare("INSERT INTO ACCOUNTS (USERNAME, PASSWORD) VALUES (:username, :password)");
        insertUserQuery.bindValue(":username", username);
        insertUserQuery.bindValue(":password", password);

        if(insertUserQuery.exec()){
            QString info_msg = "Create account was successful";
            display_message(info_msg, QMessageBox::Information);
        }
        else{
            QString info_msg = "could not add user, something went wrong";
            display_message(info_msg, QMessageBox::Information);
            QApplication::quit();
            db_con.close();
            return;
        }

    }else{
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


    if(userAuthenticateQuery.exec()){
        if(userAuthenticateQuery.next()){
            QSqlRecord rec = userAuthenticateQuery.record();
            int username_index =  rec.indexOf("USERNAME");
            int password_index =  rec.indexOf("PASSWORD");
            int userid_index =  rec.indexOf("USERID");
            QString username_query = userAuthenticateQuery.value(username_index).toString();
            QString password_query = userAuthenticateQuery.value(password_index).toString();
            int userid_query = userAuthenticateQuery.value(userid_index).toInt();

            QString info_msg = " Username: " + username_query + " Password: " + password_query + " User Id: " + QString::number(userid_query);
            display_message(info_msg, QMessageBox::Information);
        }
        else{
            QString err_msg = "There is no account for the username " + username + ". Please create an account.";
            display_message(err_msg, QMessageBox::Information);
        }

    }else{
        QString err_msg = "Tried to query database but something went wrong";
        display_message(err_msg, QMessageBox::Critical);
        QApplication::quit();
        db_con.close();
        return;
    }

    db_con.close();

}
