#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTimer>
#include <QtSql/QSqlDatabase>
#include "../../app/objects/displayContent.hpp"
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

protected:
    void resizeEvent(QResizeEvent *event) override;

private slots:
    void on_create_acc_btn_clicked();

    void on_login_btn_clicked();

    void on_logout_btn_clicked();

    void on_password_add_btn_clicked();

    void on_password_table_cellDoubleClicked(int row, int column);

    void on_search_input_textChanged(const QString &text);

private:
    Ui::MainWindow *ui = nullptr;
    DisplayContent * user_content = nullptr;
    add_password *password_form = nullptr;
    view_password *view_password_form = nullptr;

    std::mutex user_content_mutex; // user content object (operation callbacks)

    // Search: empty term = show every password.
    std::string search_term;
    bool search_active;
    std::vector<password> table_display_items;

    // All timers run on the GUI thread, so the table is only ever touched from that thread.
    QTimer refresh_timer;        // periodic: pick up changes made by the core's threads
    QTimer search_debounce_timer; // single shot: the user stopped typing in the search bar
    QTimer search_result_timer;  // single shot: show results once the search thread has run

    QString *db_path;
    bool user_loggedin;

    void account_create_and_login_display();
    void user_account_display();
    void layout_account_view();
    void login(std::string username, int user_id);
    void logout();
    void apply_search_text();
    void refresh_display();
    void populate_table(const std::vector<password> &items);
    bool insert_account(const QString &username, const QString &password, QString &error);
    QSqlDatabase GetDatabase();
};
#endif // MAINWINDOW_H
