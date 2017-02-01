#include "gui/mainwindow.h"
#include <QApplication>
#include <iostream>

int main(int argc, char *argv[]) {
    try {
        QApplication a(argc, argv);

        // Configure the application to use OpenGL 4.5
        QSurfaceFormat format;
        format.setVersion(4, 5);
        format.setProfile(QSurfaceFormat::CoreProfile);
        format.setDepthBufferSize(24);
        format.setSwapInterval(0); // This turns off vsync
        QSurfaceFormat::setDefaultFormat(format);

        MainWindow w;
        w.show();

        return a.exec();
    }
    catch(const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        std::exit(-1);
    }
}
