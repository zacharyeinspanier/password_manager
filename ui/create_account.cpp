#include "create_account.h"
#include "ui_create_account.h"

#include <QMessageBox>

create_account::create_account(QWidget *parent, create_callback callback)
    : QDialog(parent)
    , ui(new Ui::create_account)
    , create_account_cb(std::move(callback))
{
    ui->setupUi(this);
    this->ui->password_input->setEchoMode(QLineEdit::Password);
    this->ui->confirm_input->setEchoMode(QLineEdit::Password);
    this->ui->username_input->setFocus();
}

create_account::~create_account()
{
    delete ui;
}

QString create_account::created_username() const
{
    return this->new_username;
}

void create_account::reject_input(const QString &message, QWidget *field)
{
    QMessageBox::warning(this, "Create Account", message);
    field->setFocus();
}

void create_account::on_create_btn_clicked()
{
    const QString username = this->ui->username_input->text().trimmed();
    const QString password = this->ui->password_input->text();
    const QString confirm = this->ui->confirm_input->text();

    if (username.isEmpty())
    {
        this->reject_input("Please enter a username.", this->ui->username_input);
        return;
    }
    if (password.isEmpty())
    {
        this->reject_input("Please enter a password.", this->ui->password_input);
        return;
    }
    if (password != confirm)
    {
        this->ui->confirm_input->clear();
        this->reject_input("The passwords don't match. Please confirm your password again.", this->ui->confirm_input);
        return;
    }

    QString error;
    if (!this->create_account_cb || !this->create_account_cb(username, password, error))
    {
        this->reject_input(error.isEmpty() ? QString("The account could not be created.") : error,
                           this->ui->username_input);
        return;
    }

    this->new_username = username;
    QMessageBox::information(this, "Create Account", "Account \"" + username + "\" was created. You can log in now.");
    this->accept();
}

void create_account::on_cancel_btn_clicked()
{
    this->reject();
}
