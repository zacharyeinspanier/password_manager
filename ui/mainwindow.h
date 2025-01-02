#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QtSql/QSqlDatabase>
#include "../app/objects/displayContent.hpp"
#include "./add_password.h"

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    void SetDBPath(QString *env_db_path);
    ~MainWindow();

private slots:
    void on_create_acc_btn_clicked();

    void on_login_btn_clicked();

    void on_search_btn_clicked();

    void on_logout_btn_clicked();

    void on_password_add_btn_clicked();

    void on_password_remove_btn_clicked();

    void on_password_modify_btn_clicked();

    void on_password_view_btn_clicked();

    void on_password_table_cellDoubleClicked(int row, int column);

private:
    Ui::MainWindow *ui;
    add_password *password_form;
    QString *db_path;
    QSqlDatabase GetDatabase();
    DisplayContent * user_content;
    std::vector<password> table_display_items;
    bool user_loggedin;
    bool search_active;


    void account_create_and_login_display();
    void user_account_display();
    void login(std::string username, int user_id);
    void logout();
    void update_display_table();


};
#endif // MAINWINDOW_H
