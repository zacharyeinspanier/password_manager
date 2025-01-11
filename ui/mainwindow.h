#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QtSql/QSqlDatabase>
#include "../app/objects/displayContent.hpp"
#include "./add_password.h"
#include "./view_password.h"

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

    void on_password_table_cellDoubleClicked(int row, int column);

    void on_cancel_search_btn_clicked();

private:
    Ui::MainWindow *ui = nullptr;
    DisplayContent * user_content = nullptr;
    add_password *password_form = nullptr;
    view_password *view_password_form = nullptr;

    std::mutex search_term_mutex; // search term
    std::mutex table_mutex; // ui table object
    std::mutex display_items_mutex; // table_display_items vector
    std::mutex user_content_mutex; // user content object

    std::string search_term;
    bool search_active;
    std::vector<password> table_display_items;
    std::thread refresh_display_thread;

    QString *db_path;
    bool user_loggedin;

    bool exit_refresh_display_thread;
    std::mutex exit_refresh_display_thread_mtx;

    void account_create_and_login_display();
    void user_account_display();
    void login(std::string username, int user_id);
    void logout();
    void update_display_table();
    void shutdown_app();
    QSqlDatabase GetDatabase();
};
#endif // MAINWINDOW_H
