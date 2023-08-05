#include "mainwindow.h"

#include <QApplication>
#include <QMainWindow>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    MainWindow w;
    //w.setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);
    w.on_button_newGame_clicked();
    //w.setFixedSize(w.size());
    w.show();
    return a.exec();
}
