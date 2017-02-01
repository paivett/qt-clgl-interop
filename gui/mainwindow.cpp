#include "mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    //Configure window
    setWindowTitle("OpenGL-OpenCL interop");
    setFixedSize(500, 500);

    _gl_widget = new GLWidget(this);
    setCentralWidget(_gl_widget);
}

MainWindow::~MainWindow() {

}
