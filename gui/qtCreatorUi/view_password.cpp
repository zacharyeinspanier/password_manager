#include "view_password.h"
#include "ui_view_password.h"

view_password::view_password(QWidget *parent, password current_password, std::function<void(operation)> callback)
    : QDialog(parent)
    , ui(new Ui::view_password)
{
    ui->setupUi(this);

    this->current_password = current_password;
    this->queue_operation = callback;
    this->modify_password = false;
}

view_password::~view_password()
{
    delete ui;
}

void view_password::set_current_password(password current_password)
{
    this->current_password = current_password;
}

void view_password::load_current_password()
{
    if(this->current_password.p_id == -1){
        return;
    }
    this->clear_input();
    this->ui->username_input->setText(QString::fromStdString(this->current_password.username));
    this->ui->password_input->setText(QString::fromStdString(this->current_password.encryped_password));
    this->ui->url_input->setText(QString::fromStdString(this->current_password.url));
    this->ui->desc_input->setText(QString::fromStdString(this->current_password.description));
    this->view_password_display();
}

void view_password::clear_input()
{
    this->ui->username_input->setText("");
    this->ui->password_input->setText("");
    this->ui->url_input->setText("");
    this->ui->desc_input->setText("");
}

void view_password::view_password_display()
{
    this->ui->username_input->setEnabled(false);
    this->ui->password_input->setEnabled(false);
    this->ui->url_input->setEnabled(false);
    this->ui->desc_input->setEnabled(false);
    this->ui->save_password_btn->setEnabled(false);
    this->ui->password_delete_btn ->setEnabled(true);
    this->modify_password = false;
}
void view_password::modify_password_display()
{
    this->ui->username_input->setEnabled(true);
    this->ui->password_input->setEnabled(true);
    this->ui->url_input->setEnabled(true);
    this->ui->desc_input->setEnabled(true);
    this->ui->save_password_btn->setEnabled(true);
    this->ui->password_delete_btn ->setEnabled(false);
    this->modify_password = true;
}

void view_password::on_buttonBox_accepted()
{
    password temp;
    this->current_password = temp;
    this->clear_input();
    this->hide();
}


void view_password::on_buttonBox_rejected()
{
    password temp;
    this->current_password = temp;
    this->clear_input();
    this->hide();
}


void view_password::on_modify_password_btn_clicked()
{
    if(this->modify_password){
        this->view_password_display();
    }else{
        this->modify_password_display();
    }
}


void view_password::on_save_password_btn_clicked()
{

    if(this->modify_password && this->current_password.p_id != -1){
        operation modify_operation;
        modify_operation.curr_password.username = this->ui->username_input->text().toStdString();
        modify_operation.curr_password.encryped_password = this->ui->password_input->text().toStdString();
        modify_operation.curr_password.url = this->ui->url_input->text().toStdString();
        modify_operation.curr_password.description = this->ui->desc_input->toPlainText().toStdString();
        modify_operation.curr_password.p_id = this->current_password.p_id;
        modify_operation.operation_type = operationType::MODIFY;
        this->queue_operation(modify_operation);
    }

}

void view_password::set_callback(std::function<void(operation)> callback)
{
    this->queue_operation = callback;
}



void view_password::on_password_delete_btn_clicked()
{
    if(this->current_password.p_id != -1){
        operation remove_operation;
        remove_operation.curr_password.username = this->ui->username_input->text().toStdString();
        remove_operation.curr_password.encryped_password = this->ui->password_input->text().toStdString();
        remove_operation.curr_password.url = this->ui->url_input->text().toStdString();
        remove_operation.curr_password.description = this->ui->desc_input->toPlainText().toStdString();
        remove_operation.curr_password.p_id = this->current_password.p_id;
        remove_operation.operation_type = operationType::REMOVE;
        this->queue_operation(remove_operation);
    }
}

