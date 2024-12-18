#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "useraccount.h"
#include <QtSql/QSqlDatabase>

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

private:
    Ui::MainWindow *ui;
    userAccount *user_account;
    QString *db_path;
    QSqlDatabase GetDatabase();
};
#endif // MAINWINDOW_H
