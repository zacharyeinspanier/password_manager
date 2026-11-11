#include "mainwindow.h"
#include "./ui_mainwindow.h"
#include "./create_account.h"
#include "../../app/app.cpp"

#include <QHeaderView>
#include <QMessageBox>
#include <QResizeEvent>
#include <QSignalBlocker>
#include <algorithm>
#include <cstdlib>
#include <QSqlQuery>
#include <QSqlRecord>
#include <QSqlError>
#include <QVariant>

namespace
{
    // How often the table picks up changes from the core's threads.
    constexpr int REFRESH_INTERVAL_MS = 1000;
    // How long to wait after the last keystroke before searching.
    constexpr int SEARCH_DEBOUNCE_MS = 300;
    // How long to give the core's search thread before showing its results.
    constexpr int SEARCH_RESULT_DELAY_MS = 100;

    // Logged-in view layout (pixels).
    constexpr int MARGIN = 16;
    constexpr int ROW_HEIGHT = 36;
    constexpr int ADD_BTN_WIDTH = 140;
    constexpr int LOGOUT_BTN_WIDTH = 120;
    constexpr int SPACING = 12;

    // True when the two lists would show different rows in the table.
    bool table_rows_differ(const std::vector<password> &a, const std::vector<password> &b)
    {
        if (a.size() != b.size())
        {
            return true;
        }
        for (size_t i = 0; i < a.size(); ++i)
        {
            if (a[i].p_id != b[i].p_id || a[i].username != b[i].username ||
                a[i].url != b[i].url || a[i].description != b[i].description ||
                a[i].encryped_password != b[i].encryped_password)
            {
                return true;
            }
        }
        return false;
    }
}

void display_message(QString msg, QMessageBox::Icon icon)
{
    QMessageBox infoBox;
    infoBox.setIcon(icon);
    infoBox.setText(msg);
    infoBox.setStandardButtons(QMessageBox::Ok);
    infoBox.exec();
}

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), ui(new Ui::MainWindow)
{
    std::function<void(operation)> temp_cb = [](operation temp_operation) ->void{};
    password temp;

    ui->setupUi(this);
    user_loggedin = false;
    search_active = false;
    user_content = nullptr;

    // Create form objects
    this->password_form = new add_password(this, temp_cb);
    this->view_password_form = new view_password(this, temp, temp_cb);

    // UI Objects initial state
    this->ui->password_input->setEchoMode(QLineEdit::Password);
    this->ui->search_input->setPlaceholderText("Search by description, URL or username");
    this->ui->search_input->setClearButtonEnabled(true);

    this->ui->password_table->setColumnCount(3);
    this->ui->password_table->setColumnWidth(0, 150);
    this->ui->password_table->setColumnWidth(1, 200);
    this->ui->password_table->horizontalHeader()->setStretchLastSection(true);
    this->ui->password_table->setHorizontalHeaderLabels({"Username", "URL", "Description"});
    this->ui->password_table->setEditTriggers(QAbstractItemView::NoEditTriggers); // double-click opens the dialog instead
    this->ui->password_table->setSelectionBehavior(QAbstractItemView::SelectRows);
    this->ui->password_table->setSelectionMode(QAbstractItemView::SingleSelection);

    // Timers (all on the GUI thread)
    this->refresh_timer.setInterval(REFRESH_INTERVAL_MS);
    connect(&this->refresh_timer, &QTimer::timeout, this, &MainWindow::refresh_display);

    this->search_debounce_timer.setSingleShot(true);
    this->search_debounce_timer.setInterval(SEARCH_DEBOUNCE_MS);
    connect(&this->search_debounce_timer, &QTimer::timeout, this, &MainWindow::apply_search_text);

    this->search_result_timer.setSingleShot(true);
    this->search_result_timer.setInterval(SEARCH_RESULT_DELAY_MS);
    connect(&this->search_result_timer, &QTimer::timeout, this, &MainWindow::refresh_display);

    // Set default display
    this->account_create_and_login_display();
}

MainWindow::~MainWindow()
{
    this->refresh_timer.stop();
    this->search_debounce_timer.stop();
    this->search_result_timer.stop();

    {
        std::lock_guard<std::mutex> user_content_lock(this->user_content_mutex);
        if(this->user_content != nullptr){
            delete this->user_content;
            this->user_content = nullptr;
        }
    }

    delete this->password_form;
    delete this->view_password_form;
    delete this->ui;
}

void MainWindow::SetDBPath(QString *env_db_path)
{
    this->db_path = env_db_path;
}

void MainWindow::account_create_and_login_display()
{
    // UI object to show
    this->ui->create_acc_btn->show();
    this->ui->login_btn->show();
    this->ui->password_input->show();
    this->ui->password_input->clear();
    this->ui->username_input->show();
    this->ui->username_input->clear();
    this->ui->password_label->show();
    this->ui->username_label->show();

    // UI object to hide
    this->ui->search_input->hide();
    this->ui->password_add_btn->hide();
    this->ui->password_table->hide();
    this->ui->logout_btn->hide();
}

void MainWindow::user_account_display()
{
    // UI object to show
    this->ui->search_input->show();
    this->ui->search_input->clear();
    this->ui->password_add_btn->show();
    this->ui->password_table->show();
    this->ui->logout_btn->show();

    // UI object to hide
    this->ui->create_acc_btn->hide();
    this->ui->login_btn->hide();
    this->ui->password_input->hide();
    this->ui->username_input->hide();
    this->ui->password_label->hide();
    this->ui->username_label->hide();

    this->layout_account_view();
    this->ui->search_input->setFocus();
}

// Logged-in view:
//   [search bar ..............................] [Add Password]
//   [password table                                          ]
//                                                   [Logout]
// Done in code (rather than fixed positions in mainwindow.ui) so it follows window resizing.
void MainWindow::layout_account_view()
{
    if (this->ui == nullptr || this->ui->centralwidget == nullptr)
    {
        return; // resize event before setupUi finished
    }
    const int width = this->ui->centralwidget->width();
    const int height = this->ui->centralwidget->height();

    const int add_x = std::max(MARGIN, width - MARGIN - ADD_BTN_WIDTH);
    this->ui->password_add_btn->setGeometry(add_x, MARGIN, ADD_BTN_WIDTH, ROW_HEIGHT);
    this->ui->search_input->setGeometry(MARGIN, MARGIN, std::max(0, add_x - SPACING - MARGIN), ROW_HEIGHT);

    const int logout_y = std::max(MARGIN, height - MARGIN - ROW_HEIGHT);
    this->ui->logout_btn->setGeometry(std::max(MARGIN, width - MARGIN - LOGOUT_BTN_WIDTH), logout_y,
                                      LOGOUT_BTN_WIDTH, ROW_HEIGHT);

    const int table_y = MARGIN + ROW_HEIGHT + SPACING;
    this->ui->password_table->setGeometry(MARGIN, table_y, std::max(0, width - 2 * MARGIN),
                                          std::max(0, logout_y - SPACING - table_y));
}

void MainWindow::resizeEvent(QResizeEvent *event)
{
    QMainWindow::resizeEvent(event);
    this->layout_account_view();
}

QSqlDatabase MainWindow::GetDatabase()
{
    // Reuse the default connection instead of re-adding it on every call
    // (re-adding logs "duplicate connection name" warnings).
    QSqlDatabase db_con = QSqlDatabase::contains()
        ? QSqlDatabase::database(QLatin1String(QSqlDatabase::defaultConnection), false)
        : QSqlDatabase::addDatabase("QSQLITE");
    db_con.setDatabaseName(*this->db_path);
    return db_con;
}

// Inserts a new row in ACCOUNTS. Called by the Create Account dialog.
bool MainWindow::insert_account(const QString &username, const QString &password, QString &error)
{
    QSqlDatabase db_con = this->GetDatabase();
    if (!db_con.open())
    {
        error = "The Data Base could not be opened:\n" + *this->db_path + "\n\n" + db_con.lastError().text();
        return false;
    }

    // Check if user account exists
    QSqlQuery userExistsQuery(db_con);
    userExistsQuery.prepare("SELECT * FROM ACCOUNTS WHERE USERNAME = :username");
    userExistsQuery.bindValue(":username", username);
    if (!userExistsQuery.exec())
    {
        error = "Tried to query database but something went wrong:\n" + userExistsQuery.lastError().text();
        db_con.close();
        return false;
    }
    if (userExistsQuery.next())
    {
        error = "The username " + username + " already exists. Pick a new username.";
        db_con.close();
        return false;
    }

    // Insert user into accounts
    QSqlQuery insertUserQuery(db_con);
    insertUserQuery.prepare("INSERT INTO ACCOUNTS (USERNAME, PASSWORD) VALUES (:username, :password)");
    insertUserQuery.bindValue(":username", username);
    insertUserQuery.bindValue(":password", password);
    if (!insertUserQuery.exec())
    {
        error = "Could not add user, something went wrong:\n" + insertUserQuery.lastError().text();
        db_con.close();
        return false;
    }

    db_con.close();
    return true;
}

void MainWindow::on_create_acc_btn_clicked()
{
    create_account dialog(this, [this](const QString &username, const QString &password, QString &error) {
        return this->insert_account(username, password, error);
    });

    if (dialog.exec() == QDialog::Accepted)
    {
        // Ready to log in with the new account.
        this->ui->username_input->setText(dialog.created_username());
        this->ui->password_input->clear();
        this->ui->password_input->setFocus();
    }
}

void MainWindow::on_login_btn_clicked()
{
    QSqlDatabase db_con = this->GetDatabase();
    if (!db_con.open())
    {
        QString err_msg = "The Data Base could not be opened:\n" + *this->db_path + "\n\n" + db_con.lastError().text();
        display_message(err_msg, QMessageBox::Critical);
        QApplication::quit();
        return;
    }

    QString username = this->ui->username_input->text();
    QString password = this->ui->password_input->text();

    QSqlQuery userAuthenticateQuery(db_con);
    userAuthenticateQuery.prepare("SELECT * FROM ACCOUNTS WHERE USERNAME = :username AND PASSWORD = :password");
    userAuthenticateQuery.bindValue(":username", username);
    userAuthenticateQuery.bindValue(":password", password);

    if (userAuthenticateQuery.exec())
    {
        if (userAuthenticateQuery.next())
        {
            QSqlRecord rec = userAuthenticateQuery.record();
            int username_index = rec.indexOf("USERNAME");
            int userid_index = rec.indexOf("USERID");
            QString username_query = userAuthenticateQuery.value(username_index).toString();
            int userid_query = userAuthenticateQuery.value(userid_index).toInt();

            db_con.close();
            this->login(username_query.toStdString(), userid_query);
            return;
        }
        else
        {
            QString err_msg = "There is no account for the username " + username + ". Please create an account.";
            display_message(err_msg, QMessageBox::Information);
        }
    }
    else
    {
        QString err_msg = "Tried to query database but something went wrong";
        display_message(err_msg, QMessageBox::Critical);
        QApplication::quit();
        db_con.close();
        return;
    }

    db_con.close();
}

// Replaces the table rows (only called when the rows actually changed, to avoid flicker
// and keep the user's selection/scroll position).
void MainWindow::populate_table(const std::vector<password> &items)
{
    this->table_display_items = items;

    QTableWidget *table = this->ui->password_table;
    table->setUpdatesEnabled(false);
    table->setRowCount(static_cast<int>(items.size()));
    for (int i = 0; i < static_cast<int>(items.size()); ++i)
    {
        table->setItem(i, 0, new QTableWidgetItem(QString::fromStdString(items[i].username)));
        table->setItem(i, 1, new QTableWidgetItem(QString::fromStdString(items[i].url)));
        table->setItem(i, 2, new QTableWidgetItem(QString::fromStdString(items[i].description)));
    }
    table->setUpdatesEnabled(true);
}

// Pulls the latest passwords from the core and updates the table.
//
// While searching, this asks the core's search thread to (re)run the current term and shows the
// results of the last completed search. Re-running on every refresh keeps results current after
// adds, edits and deletes; search_result_timer shows a new term's results right after it runs.
void MainWindow::refresh_display()
{
    std::vector<password> items;
    {
        std::lock_guard<std::mutex> user_content_lock(this->user_content_mutex);
        if (this->user_content == nullptr)
        {
            return;
        }
        if (this->search_active)
        {
            this->user_content->search_event(this->search_term);
            items = this->user_content->get_search_result();
        }
        else
        {
            this->user_content->reset_display_list();
            items = this->user_content->get_display_list();
        }
    }

    if (table_rows_differ(this->table_display_items, items))
    {
        this->populate_table(items);
    }
}

// Typing restarts the debounce timer; the search runs once the user pauses.
void MainWindow::on_search_input_textChanged(const QString &text)
{
    Q_UNUSED(text);
    this->search_debounce_timer.start();
}

void MainWindow::apply_search_text()
{
    const std::string term = this->ui->search_input->text().trimmed().toStdString();
    if (term == this->search_term && this->search_active == !term.empty())
    {
        return;
    }

    this->search_term = term;
    this->search_active = !term.empty();

    if (this->search_active)
    {
        {
            std::lock_guard<std::mutex> user_content_lock(this->user_content_mutex);
            if (this->user_content != nullptr)
            {
                this->user_content->search_event(this->search_term); // runs on the core's search thread
            }
        }
        this->search_result_timer.start();
    }
    else
    {
        this->refresh_display(); // back to every password, no waiting needed
    }
}

void MainWindow::login(std::string username, int user_id)
{
    std::string db_path_std_string = this->db_path->toStdString();

    this->ui->password_table->setRowCount(0);
    this->table_display_items.clear();
    this->search_term.clear();
    this->search_active = false;

    {
        std::lock_guard<std::mutex> user_content_lock(this->user_content_mutex);
        this->user_content = launch_app(username, user_id, &db_path_std_string);
    }

    std::function<void(operation)> queue_operation_callback = [this](operation new_operation) -> void{
        std::lock_guard<std::mutex> user_content_lock(this->user_content_mutex);
        if (this->user_content != nullptr)
        {
            this->user_content->operation_event(new_operation);
        }
        // Show the change right away instead of waiting for the next periodic refresh.
        QTimer::singleShot(SEARCH_RESULT_DELAY_MS, this, &MainWindow::refresh_display);
    };

    this->view_password_form->set_callback(queue_operation_callback);
    this->password_form->set_callback(queue_operation_callback);
    this->user_loggedin = true;

    // user_account_display() clears the search bar; block the signal so it doesn't start a search.
    {
        QSignalBlocker block_search(this->ui->search_input);
        this->user_account_display();
    }

    this->refresh_display();
    this->refresh_timer.start();
}

void MainWindow::logout()
{
    this->refresh_timer.stop();
    this->search_debounce_timer.stop();
    this->search_result_timer.stop();

    this->ui->password_table->setRowCount(0);
    this->table_display_items.clear();
    this->search_term.clear();
    this->search_active = false;

    std::function<void(operation)> temp_cb = [](operation new_operation) -> void {};
    this->view_password_form->set_callback(temp_cb);
    this->password_form->set_callback(temp_cb);

    // Delete user content (stops the core's threads; the data store thread saves one last time)
    {
        std::lock_guard<std::mutex> user_content_lock(this->user_content_mutex);
        delete this->user_content;
        this->user_content = nullptr;
    }

    this->user_loggedin = false;
    {
        QSignalBlocker block_search(this->ui->search_input);
        this->ui->search_input->clear();
    }
    this->account_create_and_login_display();
}

void MainWindow::on_logout_btn_clicked()
{
    if (this->user_loggedin == false)
    {
        QString warningMsg = "Error: user is not logged in!!";
        display_message(warningMsg, QMessageBox::Warning);
        return;
    }
    this->logout();
}

void MainWindow::on_password_add_btn_clicked()
{
    if (this->user_loggedin == false)
    {
        QString warningMsg = "Error: user is not logged in!!";
        display_message(warningMsg, QMessageBox::Warning);
        return;
    }

    this->password_form->show();
}


void MainWindow::on_password_table_cellDoubleClicked(int row, int column)
{
    Q_UNUSED(column);
    if (row < 0 || row >= static_cast<int>(this->table_display_items.size()))
    {
        return;
    }
    this->view_password_form->set_current_password(this->table_display_items[row]);
    this->view_password_form->load_current_password();
    this->view_password_form->show();
}
