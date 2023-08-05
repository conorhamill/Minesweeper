#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "button.h"

#include <QPushButton>
#include <QGridLayout>
#include <QVector>
#include <QRandomGenerator>
#include <QObject>
#include <QTimer>
#include <QVector>
#include <QQueue>
#include <QMessageBox>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_button_newGame_clicked()
{
    //reset newgame
    ui->button_newGame->setStyleSheet(normalStyle);
    buttons_pressed = 0;
    gameRunning = false;
    ui->solver_label->setText("");

    if (buttons.empty() == false)
    {
        buttons.clear();
    }

    //check which settings have been selected
    if (ui->radioButton_easy->isChecked())
    {
        rows = 9;
        cols = 9;
        bombs = 10;
        total_buttons = rows * cols;
        ui->lineEdit_rows->setText("9");
        ui->lineEdit_cols->setText("9");
        ui->lineEdit_mines->setText("10");
    }
    else if (ui->radioButton_medium->isChecked())
    {
        rows = 16;
        cols = 16;
        bombs = 40;
        total_buttons = rows * cols;
        ui->lineEdit_rows->setText("16");
        ui->lineEdit_cols->setText("16");
        ui->lineEdit_mines->setText("40");
    }
    else if (ui->radioButton_hard->isChecked())
    {
        rows = 16;
        cols = 30;
        bombs = 99;
        total_buttons = rows * cols;
        ui->lineEdit_rows->setText("16");
        ui->lineEdit_cols->setText("30");
        ui->lineEdit_mines->setText("99");
    }
    else if (ui->radioButton_custom->isChecked())
    {
        rows = ui->lineEdit_rows->text().toInt();
        cols = ui->lineEdit_cols->text().toInt();
        bombs = ui->lineEdit_mines->text().toInt();
        total_buttons = rows * cols;
        //check user input is correct
        if (!(rows > 0 && rows <= 39 && cols > 0 && cols <= 39 && bombs > 0 && bombs < total_buttons))
        {
            QMessageBox::critical(this, "Incorrect input", "Please enter valid input. rows: 0-40 cols: 0-40 mines: < rows*cols");
            //set to hard as default
            rows = 16;
            cols = 30;
            bombs = 49;
            total_buttons = rows * cols;
            ui->lineEdit_rows->setText("16");
            ui->lineEdit_cols->setText("30");
            ui->lineEdit_mines->setText("99");
        }
    }

    //check if there is a game already created and if so clear the grid layout
    if (ui->Grid->layout() != nullptr)
    {
        while ( QWidget* w = ui->Grid->findChild<QWidget*>() )
        {
            delete w;
        }
        delete ui->Grid->layout();
    }

    //create a new grid layout for the Grid widget on the main ui
    auto gridLayout = new QGridLayout(ui->Grid);
    gridLayout->setSpacing(0);

    //for the given number of rows and cols create buttons in the grid layout
    for (int i = 0; i < rows; i++)
    {
        //create a row for the buttons that can be added to the main buttons 2dvector
        QVector<QPushButton*> buttonRow;

        for (int j = 0; j < cols; j++)
        {
            auto button = new Button(ui->Grid);
            button->setFixedSize(20,20);
            button->setStyleSheet(normalStyle);
            gridLayout->addWidget(button,i,j);
            button->setProperty("row",i);
            button->setProperty("col",j);
            button->setProperty("flag",false);
            button->setProperty("pressed",false);

            //add the button to the button row which can be added to the 2Dvector
            buttonRow.push_back(button);

            //for each button connect a function in the event of the button being clicked
            connect(button, &Button::clicked, this, &MainWindow::handle_button_clicked);

            //create the right click action
            connect(button, &Button::rightClicked, this, &MainWindow::handle_button_right_clicked);
        }

        //add button rows to the main button 2d vector
        buttons.push_back(buttonRow);
    }
    ui->Grid->setLayout(gridLayout);

    //update the window size
    windowWidth = 20*cols + 40;
    windowHeight = 20*rows + 180;
    MainWindow::setFixedSize(windowWidth, windowHeight);

    //create a new model for the game.
    model = create_model(rows,cols,bombs);

    //initialise the flag counter
    flags = bombs;
    ui->flag_label->setText(QString::number(flags));

    start_timer();
}

QPushButton *MainWindow::getButton(int row, int col)
{
    auto gridLayout =  static_cast <QGridLayout*> (ui->Grid->layout());
    return static_cast <QPushButton*> (gridLayout->itemAtPosition(row,col)->widget());
}

void MainWindow::handle_button_right_clicked()
{
    //get the button details from the button click event and change its style
    QPushButton* button = qobject_cast <QPushButton*> (sender());
    flag_button(button);
}

void MainWindow::handle_button_clicked()
{
    //start the timer on the first button click
    if (buttons_pressed == 0)
    {
        timer->start();
        gameRunning = true;
    }

    //get the button details from the button click event
    QPushButton* button = qobject_cast <QPushButton*> (sender());
    if (button->property("flag").toBool() == true)
    {
        return;
    }
    press_button(button);
}

QVector<QVector<int>> MainWindow::create_model(int rows, int cols, int bombs)
{

    //create an empty 2D array for the given size
    QVector<QVector<int>> model;
    model.resize(rows);
    for (int i = 0; i < rows; i++)
    {
        model[i].resize(cols);
    }

    //create a list with the number of elements = num of buttons and randomly assign -1 as bombs
    QVector<int> model_list;
    model_list.resize((rows*cols),0);
    int num;
    for (int i = 0; i < bombs; i++)
    {
        num = QRandomGenerator::global()->bounded(0,model_list.size());
        if (model_list[num] != -1)
        {
            model_list[num] = -1;
        }
        //this ensures if a grid has been selected more than once there will still be correct num of bombs
        else
        {
            i--;
        }
    }

    //convert the 1D list to the 2d vector model
    int index = 0;
    for (int i = 0; i < rows; i++)
    {
        for (int j = 0; j < cols; j++)
        {
            model[i][j] = model_list[index];
            index++;
        }
    }

    //for each cell check through the 8 cells around the cell in question and add how many bombs are found
    int row, col, bomb_count;
    for (int i = 0; i < rows; i++)
    {
        for (int j = 0; j < cols; j++)
        {
            //only check and change value if the cell is not already a bomb
            if (model[i][j] != -1)
            {
                bomb_count = 0;
                for (int k = 0; k < offsets.size(); k++)
                {
                    //go through the offsets and check if they are within the limits of the 2d vector
                    row = i + offsets[k][0];
                    col = j + offsets[k][1];
                    if (row >= 0 && row < rows && col >= 0 && col < cols)
                    {
                        //if a bomb is found increment the counter
                        if (model[row][col] == -1)
                        {
                            bomb_count++;
                        }
                    }
                }
                //set the model value to the bomb count
                model[i][j] = bomb_count;
            }
        }
    }
    return model;
}

void MainWindow::show_bombs()
{
    // show all the values for the model
    if (ui->Grid->layout() != nullptr)
    {
        for (int i = 0; i < rows; i++)
        {
            for (int j = 0; j < cols; j++)
            {
                if (model[i][j] == -1)
                {
                    getButton(i,j)->setText("*");
                    getButton(i,j)->setStyleSheet(bombStyle);
                }
                getButton(i,j)->setEnabled(false);
            }
        }
    }
}

void MainWindow::show_bombs_win()
{
    // show all the values for the model
    if (ui->Grid->layout() != nullptr)
    {
        for (int i = 0; i < rows; i++)
        {
            for (int j = 0; j < cols; j++)
            {
                if (model[i][j] == -1)
                {
                    getButton(i,j)->setText("*");
                    getButton(i,j)->setStyleSheet(flagStyle);
                }
                getButton(i,j)->setEnabled(false);
            }
        }
    }
}

void MainWindow::update_timer_label()
{
    timerValue++;
    ui->timer_label->setText(QString::number(timerValue));
}

void MainWindow::zero_pressed(int row, int col)
{
    //for each cell check through the 8 cells around the cell in question
    //change the button status to pressed until all surrounding cells are not zero
    QPushButton* button;
    int new_row, new_col;

    for (int i = 0; i < offsets.size(); i++)
    {
        //check if the offset is within the limits of the 2d vector
        new_row = row + offsets[i][0];
        new_col = col + offsets[i][1];
        if (new_row >= 0 && new_row < rows && new_col >= 0 && new_col < cols)
        {
            //set the button to the correct pointer from the main 2dvector
            button = buttons[new_row][new_col];

            //ensure the cell has not been pressed
            if (button->property("pressed").toBool() == false)
            {
                //show its value and mark it pressed
                button->setText(QString::number(model[new_row][new_col]));
                button->setStyleSheet(pressedStyle);
                button->setEnabled(false);
                buttons_pressed++;
                button->setProperty("pressed",true);

                //if its zero then enter the algorithm again, also remove the zero number and just show blank
                if (model[new_row][new_col] == 0)
                {
                    button->setText("");
                    zero_pressed(new_row,new_col);
                }
            }
        }
    }
}

void MainWindow::press_button(QPushButton* buttonToPress)
{

    QPushButton* button = buttonToPress;
    int row = button->property("row").toInt();
    int col = button->property("col").toInt();

    //if the button you pressed was a bomb then stop game and show all bombs
    if (model[row][col] == -1)
    {
        button->setStyleSheet(bombStyle);
        show_bombs();
        ui->button_newGame->setStyleSheet(bombStyle);
        timer->stop();
        gameRunning = false;
        ui->solver_label->setText("Game lost");
    }
    //otherwise press that button and show number
    else
    {
        button->setText(QString::number(model[row][col]));
        button->setStyleSheet(pressedStyle);
        button->setEnabled(false);
        buttons_pressed++;
        button->setProperty("pressed",true);

        //if the value of the cell is 0 then keep opening all the other adjacent cells until there are no more surrounding zeros
        if (model[row][col] == 0)
        {
            button->setText("");
            zero_pressed(row,col);
        }

        //check if all buttons have been pressed and all flags laid down - game is win
        if (buttons_pressed >= (total_buttons - bombs) && flags == 0)
        {
            show_bombs_win();
            ui->button_newGame->setStyleSheet(flagStyle);
            timer->stop();
            gameRunning = false;
            ui->solver_label->setText("Game won");
        }
    }
}

void MainWindow::start_timer()
{
    //create a timer and connect it to a function that will increment every 1s.
    if (timer == nullptr)
    {
        timer = new QTimer(this);
        timer->setInterval(1000);
        connect(timer, &QTimer::timeout, this, &MainWindow::update_timer_label);
    }

    //reset the timer
    timer->stop();
    timerValue = 0;
    ui->timer_label->setText("timer");
}

void MainWindow::flag_button(QPushButton *buttonToFlag)
{
    QPushButton* button = buttonToFlag;

    //if the button does not already have a flag and there are flags remaining then add one
    if (button->property("flag").toBool() == false)
    {
        if (flags > 0)
        {
            button->setStyleSheet(flagStyle);
            button->setProperty("flag",true);
            flags--;
            ui->flag_label->setText(QString::number(flags));

            //check if flags are zero and all buttons are pressed - game is win
            if (buttons_pressed >= (total_buttons - bombs) && flags == 0)
            {
                show_bombs_win();
                ui->button_newGame->setStyleSheet(flagStyle);
                timer->stop();
                gameRunning = false;
                ui->solver_label->setText("Game won");
            }
        }
    }
    //otherwise if there is a flag then remove it
    else
    {
        button->setStyleSheet(normalStyle);
        button->setProperty("flag",false);
        flags++;
        ui->flag_label->setText(QString::number(flags));
    }
}

void MainWindow::on_pushButton_hint_clicked()
{
    QQueue <QPushButton*> pressedQ;
    QQueue <QPushButton*> unpressedLocalQ;
    QQueue <QPushButton*> flaggedLocalQ;
    QPushButton* button;
    int row,col, new_row, new_col, unpressed = 0;


    //create a queue of all the pressed and unpressed buttons greater than zero in the grid
    for (int i =0; i < buttons.size(); i++)
    {
        for (int j = 0; j < buttons[0].size(); j++)
        {
            //check if it is pressed and if the value is greater than 0
            if (buttons[i][j]->property("pressed").toBool() == true && model[i][j] > 0)
            {
                pressedQ.enqueue(buttons[i][j]);
            }
        }
    }
    //if the queue for pressed buttons is zero, press a corner button to get started
    if (pressedQ.isEmpty() == true)
    {
        start_timer();
        timer->start();
        press_button(buttons[0][0]);
        ui->solver_label->setText("start in corner");
        gameRunning = true;
    }
    //check if the previous pressed queue is the same as the current pressed queue - no hint available.
    else if (pressedQ == prevPressedQ)
    {
        ui->solver_label->setText("no hint. user move");
        gameRunning = false;
        return;
    }
    prevPressedQ = pressedQ;

    //for all pressed buttons: check around them and count the number of unpressed buttons and flagged buttons, add these to a temp local queue
    //we make an assumption here that all flagged buttons are bombs - this is dependant on correct user input
    while (pressedQ.isEmpty() == false)
    {
        button = pressedQ.dequeue();
        row = button->property("row").toInt();
        col = button->property("col").toInt();

        for (int i = 0; i < offsets.size(); i++)
        {
            //check if the offset are within the limits of the 2d vector
            new_row = row + offsets[i][0];
            new_col = col + offsets[i][1];
            if (new_row >= 0 && new_row < rows && new_col >= 0 && new_col < cols)
            {
                button = buttons[new_row][new_col];
                //check if the button is unpressed and not flagged.if so add it to the local queue
                if (button->property("pressed").toBool() == false && button->property("flag").toBool() == false)
                {
                    unpressedLocalQ.enqueue(button);
                }
                //check if the button is flaged and if so add it to the local queue
                if (button->property("flag").toBool() == true)
                {
                    flaggedLocalQ.enqueue(button);
                }
            }
        }
        //check if the qty of unpressed buttons and flagged buttons is same as cell number. then all unpressed are bombs. flag them.
        if (unpressedLocalQ.size() + flaggedLocalQ.size()== model[row][col])
        {
            while (!unpressedLocalQ.isEmpty())
            {
                flag_button(unpressedLocalQ.dequeue());
            }
        }
        //check if the qty of flags is same as the cell number. then all are fine. press them.
        else if (flaggedLocalQ.size() == model[row][col])
        {
            while (!unpressedLocalQ.isEmpty())
            {
                press_button(unpressedLocalQ.dequeue());
            }
        }
        //clear the local unpressed and flagged queues
        unpressedLocalQ.clear();
        flaggedLocalQ.clear();
    }

    //check for any unpressed buttons greater than zero in the grid
    for (int i =0; i < buttons.size(); i++)
    {
        for (int j = 0; j < buttons[0].size(); j++)
        {
            //check if it is pressed
            if (buttons[i][j]->property("pressed").toBool() == false && buttons[i][j]->property("flag").toBool() == false)
            {
                unpressed++;
            }
        }
    }
    //check if there are no unpressed buttons and no remaining flags - game won.
    if (unpressed == 0 && flags == 0)
    {
        show_bombs_win();
        ui->button_newGame->setStyleSheet(flagStyle);
        gameRunning = false;
        return;
    }
}

void MainWindow::on_solver_timer_timeout()
{
    if (gameRunning)
    {
        ui->solver_label->setText("Solver running");
        MainWindow::on_pushButton_hint_clicked();
    }
    else
    {
        timer2->stop();
    }
}

void MainWindow::on_solver_button_clicked()
{
    //if first time create a timer and connect it to a function that will increment every 1s.
    if (timer2 == nullptr)
    {
        //initialise and reset the timer
        timer2 = new QTimer(this);
        timer2->setInterval(500);
        connect(timer2, &QTimer::timeout, this, &MainWindow::on_solver_timer_timeout);
        timer2->start();
    }
    else
    {
        timer2->start();
    }
    gameRunning = true;
}

