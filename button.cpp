#include "button.h"

Button::Button(QWidget *parent) : QPushButton(parent)
{

}

void Button::mousePressEvent(QMouseEvent *e)
{
    if(e->button()==Qt::RightButton)
    {
        e->accept();
        emit rightClicked();
        return;
    }
    QPushButton::mousePressEvent(e);
}


