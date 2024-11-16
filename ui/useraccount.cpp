#include "useraccount.h"
#include "ui_useraccount.h"

userAccount::userAccount(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::userAccount)
{
    ui->setupUi(this);
}

userAccount::~userAccount()
{
    delete ui;
}
