#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QPushButton>
#include <QVector>
#include <QQueue>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

public slots:
    void on_button_newGame_clicked();

private slots:
    void on_pushButton_hint_clicked();

    void on_solver_button_clicked();

private:
    Ui::MainWindow *ui;

    //create variables
    int rows;
    int cols;
    int bombs;
    int flags;
    int total_buttons;
    int buttons_pressed;
    int timerValue;
    bool gameRunning = false;

    QTimer *timer = nullptr;

    QTimer* timer2 = nullptr;

    int windowWidth;
    int windowHeight;

    QVector<QVector<int>> model;

    QVector<QVector<QPushButton*>> buttons;

    QVector<QVector<int>> offsets = {{-1,-1},{-1,0},{-1,1},{0,-1},{0,1},{1,-1},{1,0},{1,1}};

    QQueue <QPushButton*> prevPressedQ;

    //set the default style for the buttons
    QString normalStyle = "QPushButton { background-color: lightgrey; color: black; }";
    QString bombStyle = "QPushButton { background-color: red; color: black; }";
    QString pressedStyle = "QPushButton { background-color: grey; color: black; }";
    QString flagStyle = "QPushButton { background-color: lightgreen; color: black; }";

    //functions
    QVector<QVector<int>> create_model(int rows, int cols, int bombs);

    QPushButton* getButton(int row, int col);

    void handle_button_right_clicked();

    void handle_button_clicked();

    void show_bombs();
    void show_bombs_win();

    void update_timer_label();

    void zero_pressed(int row, int col);

    void press_button(QPushButton* button);

    void start_timer();

    void flag_button(QPushButton* button);

    void on_solver_timer_timeout();



};
#endif // MAINWINDOW_H
