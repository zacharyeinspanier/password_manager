#ifndef CREATE_ACCOUNT_H
#define CREATE_ACCOUNT_H

#include <QDialog>
#include <QString>
#include <functional>

namespace Ui {
class create_account;
}

// Dialog for creating a new account: username, password, confirm password.
//
// The dialog validates the input; the actual database insert is done by the callback
// (MainWindow::insert_account), which returns false and fills `error` if it fails
// (for example, the username is taken). The dialog stays open on failure.
class create_account : public QDialog
{
    Q_OBJECT

public:
    using create_callback = std::function<bool(const QString &username, const QString &password, QString &error)>;

    explicit create_account(QWidget *parent, create_callback callback);
    ~create_account();

    // The username that was created (valid after exec() returns QDialog::Accepted).
    QString created_username() const;

private slots:
    void on_create_btn_clicked();
    void on_cancel_btn_clicked();

private:
    Ui::create_account *ui;
    create_callback create_account_cb;
    QString new_username;

    void reject_input(const QString &message, QWidget *field);
};

#endif // CREATE_ACCOUNT_H
