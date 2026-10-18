#include "mainwindow.h"

#include <QApplication>
#include <QLocale>
#include <QTranslator>
#include <QMessageBox>
#include <QFileInfo>


QString env_db_path = NULL;
// User account
int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    QTranslator translator;
    const QStringList uiLanguages = QLocale::system().uiLanguages();
    for (const QString &locale : uiLanguages) {
        const QString baseName = "ui_" + QLocale(locale).name();
        if (translator.load(":/i18n/" + baseName)) {
            a.installTranslator(&translator);
            break;
        }
    }


    // Load Enviroment Variable
    MainWindow w;
    char* env_db_path_raw = std::getenv("DB_PATH");
    if(env_db_path_raw == nullptr){
        QString err_msg = "The enviroment variable DB_PATH is not sourced";
        QMessageBox errMsgBox;
        errMsgBox.setIcon(QMessageBox::Critical);
        errMsgBox.setText(err_msg);
        errMsgBox.setStandardButtons(QMessageBox::Ok);
        errMsgBox.exec();
        QApplication::quit();
        return 1;
    }
    env_db_path = QString::fromStdString(std::string(env_db_path_raw));

    // Catch a wrong path up front. Otherwise SQLite either fails with a vague "could not be opened"
    // or silently creates a new, empty database file at the wrong location.
    if(!QFileInfo(env_db_path).isFile()){
        QString err_msg = "DB_PATH does not point to an existing database file:\n\n" + env_db_path
                          + "\n\nSet DB_PATH to the full path of db/user_accounts.db"
                            " (environment variables like %USERPROFILE% are not expanded).";
        QMessageBox errMsgBox;
        errMsgBox.setIcon(QMessageBox::Critical);
        errMsgBox.setText(err_msg);
        errMsgBox.setStandardButtons(QMessageBox::Ok);
        errMsgBox.exec();
        return 1;
    }
    w.SetDBPath(&env_db_path);
    w.show();
    return a.exec();
}
