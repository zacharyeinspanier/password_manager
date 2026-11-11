#ifndef VIEW_PASSWORD_H
#define VIEW_PASSWORD_H

#include <QDialog>

#include "../app/structs/password.hpp"
#include "../app/structs/operations.hpp"

namespace Ui {
class view_password;
}

class view_password : public QDialog
{
    Q_OBJECT

public:
    explicit view_password(QWidget *parent = nullptr, password current_password = {}, std::function<void(operation)> callback = {});
    void set_callback(std::function<void(operation)> callback);
    void set_current_password(password current_password);
    void load_current_password();
    ~view_password();

private slots:
    void on_buttonBox_accepted();

    void on_buttonBox_rejected();

    void on_modify_password_btn_clicked();

    void on_save_password_btn_clicked();

    void on_password_delete_btn_clicked();

private:
    Ui::view_password *ui;
    password current_password;
    std::function<void(operation)> queue_operation;
    bool modify_password;

    void view_password_display();
    void modify_password_display();
    void clear_input();
};

#endif // VIEW_PASSWORD_H
