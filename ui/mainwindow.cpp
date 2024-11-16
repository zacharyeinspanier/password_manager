#include "mainwindow.h"
#include "./ui_mainwindow.h"
#include "createaccount.h"
#include "useraccount.h"
#include <QMessageBox>
#include <cstdlib>

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


void MainWindow::on_create_acc_btn_clicked()
{
    QSqlDatabase db_con = QSqlDatabase::addDatabase("QSQLITE");
    char* env_db_path = std::getenv("DB_PATH");
    QString db_path;
    if(env_db_path == nullptr){
        db_path = "null ptr";
    }else{
        db_path = QString::fromStdString(std::string(env_db_path));
    }

    //TODO: Database path should be an .env var, or a global path
    //db_con.setDatabaseName(db_path);
    QString msg = db_path;

    // if (db_con.open())
    // {
    //     msg = "db is open!!";


    // }else{
    //     msg = "db is not open:(";
    // }

    QMessageBox msgBox;
    msgBox.setIcon(QMessageBox::Critical);
    msgBox.setText(msg);
    msgBox.setStandardButtons(QMessageBox::Ok);
    msgBox.exec();
    QApplication::quit();

    //1: get username and password from input boxes
    //2: check if connection to db is open
    //3: check if username already exists
    //4: generate userid
    //5: insert into user table

    // We will just update the buttons that are displayed
    // So create accont will have:
        // Input: user name
        // Input: password
        // Input: re-type password
        // Button: back to login
        // Button: Register account

    // Temporary action while learning how to use QT
//     this->hide();
//     createAccount account_creation;
//     account_creation.setModal(true);
//     account_creation.exec();
}


void MainWindow::on_login_btn_clicked()
{

    user_account = new userAccount(this);
    user_account -> show();
    this->hide();

}

