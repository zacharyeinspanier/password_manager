#ifndef ADD_PASSWORD_H
#define ADD_PASSWORD_H

#include <QDialog>
#include "../app/structs/password.hpp"
#include "../app/structs/operations.hpp"

namespace Ui {
class add_password;
}

class add_password : public QDialog
{
    Q_OBJECT

public:
    explicit add_password(QWidget *parent = nullptr, std::function<void(operation)> callback = {});
    ~add_password();

    void set_callback(std::function<void(operation)> callback);


    password construct_password();

private slots:
    void on_buttonBox_accepted();

    void on_buttonBox_rejected();

private:
    Ui::add_password *ui;
    std::chrono::system_clock::duration time_since_epoch;

    std::function<void(operation)> queue_operation;

    void clear_input();
};

#endif // ADD_PASSWORD_H
