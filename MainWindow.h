#ifndef KAWAUSO_MAIN_WINDOW_H
#define KAWAUSO_MAIN_WINDOW_H

#include <QMainWindow>
#include "VimEditor.h"
#include <QApplication>
#include <QStatusBar>

namespace Kawauso {
  class MainWindow : public QMainWindow {
  public:
    MainWindow()
    {
      setWindowTitle("FakeVim");
      auto vimEditor = new VimEditor();
      setCentralWidget(vimEditor);
      resize(800, 600);
      auto font = QApplication::font();
      font.setFamily("Monospace");
      vimEditor->setFont(font);
      statusBar()->setFont(font);
      connect(vimEditor, &VimEditor::statusMessageChanged,
          [=](const QString& message) {
            statusBar()->showMessage(message);
          });
    }
  };
}
// vim:set ts=2 sw=2 et:
#endif
