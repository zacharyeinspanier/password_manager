#include "mainwindow.h"

#include <QApplication>
#include <QLocale>
#include <QTranslator>
#include <QMessageBox>

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


    // Main() plan
    //1: load database
    //2: user authentication
    //3: on success of user authentication load user passwords
    //4: create useraccount
    //5: call useraccount::data
    //6: call app launch app
    //7: quit mainwindow
    //8: create window; account window
    //9: display account window. 

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
    }else{
        env_db_path = QString::fromStdString(std::string(env_db_path_raw));
        w.SetDBPath(&env_db_path);
        w.show();
        return a.exec();
    }
}
