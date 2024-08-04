#include "mainwindow.h"

#include <QApplication>

int main(int argc, char *argv[])
{
    auto application = QApplication{argc, argv};
    auto mainWindow = IconFonts::Viewer::MainWindow{};

    mainWindow.show();

    return application.exec();
}
