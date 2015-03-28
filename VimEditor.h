#ifndef KAWAUSO_VIM_EDITOR_H
#define KAWAUSO_VIM_EDITOR_H

#include <QPlainTextEdit>
#include <QRect>
#include <QString>
#include <QList>
#include <QTextEdit>
#include <QScopedPointer>
#include <fakevimhandler.h>
#include <QWidget>
#include <QPaintEvent>

namespace Kawauso {
  class VimEditor: public QPlainTextEdit {
    Q_OBJECT
  private:
    typedef QPlainTextEdit super;
    typedef FakeVim::Internal::FakeVimHandler FakeVimHandler;
    typedef FakeVim::Internal::ExCommand ExCommand;
  private:
    QRect cursorRect_;
    QString statusMessage_;
    QString statusData_;
    QString fileName_;
    QList<QTextEdit::ExtraSelection> searchSelection_;
    QList<QTextEdit::ExtraSelection> clearSelection_;
    QList<QTextEdit::ExtraSelection> blockSelection_;
    QScopedPointer<FakeVimHandler> fakeVim_;
  public:
    VimEditor(QWidget* parent = 0);
    void paintEvent(QPaintEvent* e);
    void openFile(const QString& fileName);
  Q_SIGNALS:
    void handleInput(const QString& keys);
    void statusMessageChanged(const QString& message);
  private Q_SLOTS:
    void changeStatusData(const QString& info);
    void highlightMatches(const QString& pattern);
    void changeStatusMessage(const QString &contents, int cursorPos);
    void changeExtraInformation(const QString &info);
    void updateStatusBar();
    void handleExCommand(bool *handled, const ExCommand &cmd);
    void requestSetBlockSelection(bool on = false);
    void requestHasBlockSelection(bool *on);
    void parseArguments();
private:
    void updateExtraSelections();
    bool wantSaveAndQuit(const ExCommand &cmd);
    bool wantSave(const ExCommand &cmd);
    bool wantQuit(const ExCommand &cmd);
    bool save();
    void cancel();
    void invalidate();
    bool hasChanges();
    QString content() const;
  };
}
// vim:set ts=2 sw=2 et:
#endif
