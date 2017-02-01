#ifndef _MAINWINDOW_H_
#define _MAINWINDOW_H_

#include <QMainWindow>
#include <QMenu>
#include <QLabel>
#include <QSharedPointer>

#include "glwidget.h"

class MainWindow : public QMainWindow {
    Q_OBJECT

    public:
        MainWindow(QWidget *parent = 0);
        ~MainWindow();

    private:
        GLWidget* _gl_widget;
};

#endif // _MAINWINDOW_H_
