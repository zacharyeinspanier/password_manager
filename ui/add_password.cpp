#include "add_password.h"
#include "ui_add_password.h"

add_password::add_password(QWidget *parent, std::function<void(operation)> callback)
    : QDialog(parent)
    , ui(new Ui::add_password)
{

    ui->setupUi(this);
    this->queue_operation = callback;
}

add_password::~add_password()
{
    delete ui;
}

void add_password::set_callback(std::function<void(operation)> callback){
    this->queue_operation = callback;
}


void add_password::clear_input()
{
    this->ui->password_input->clear();
    this->ui->url_input->clear();
    this->ui->username_input->clear();
    this->ui->desc_input->clear();
}

password add_password::construct_password()
{
    password new_password;
    new_password.username = this->ui->username_input->text().toStdString();
    new_password.encryped_password = this->ui->password_input->text().toStdString();
    new_password.url = this->ui->url_input->text().toStdString();
    new_password.description = this->ui->desc_input->toPlainText().toStdString();
    new_password.date_created = 0;
    new_password.date_modified = 0;
    new_password.p_id = -1;
    this->clear_input();
    return new_password;
}
void add_password::on_buttonBox_accepted()
{
    password new_password = this->construct_password();
    operation new_operation;
    new_operation.curr_password = new_password;
    new_operation.operation_type = operationType::ADD;
    this->queue_operation(new_operation);
    this->clear_input();
    this->hide();

}


void add_password::on_buttonBox_rejected()
{
    this->clear_input();
    this->hide();
}

