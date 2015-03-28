#include <QApplication>
#include "MainWindow.h"

namespace Kawauso {
  extern "C"
  int main(int argc, char** argv)
  {
    QApplication app(argc, argv);
    app.setOrganizationName("Volpts");
    app.setApplicationName("Kawauso");
    MainWindow main;
    main.show();
    return app.exec();
  }
}

// vim:set sw=2 ts=2 et:
