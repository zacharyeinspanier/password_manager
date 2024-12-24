#include "mainwindow.h"

#include <QApplication>
#include <QLocale>
#include <QTranslator>
#include <QMessageBox>
#include "../app/app.cpp"

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
    w.SetDBPath(&env_db_path);
    w.show();
    return a.exec();

    // when the application window exits we will need to do the following
    // 1: retrieve the user_id
    // 2: load user data from sql data base
    // 3: call start app
    // 4: create a new application
    // 5: show and start the application.
}
