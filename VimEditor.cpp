#include "VimEditor.h"

#include <QPainter>
#include <QMessageBox>
#include <QApplication>
#include <QTextBlock>
#include <QTemporaryFile>
#include <QTextStream>


namespace Kawauso {
  VimEditor::VimEditor(QWidget* parent) :
    QPlainTextEdit(parent),
    fakeVim_(new FakeVimHandler(this, 0))
  {
    setCursorWidth(0);
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    connect(fakeVim_.data(),
        SIGNAL(commandBufferChanged(QString, int, int, int, QObject*)),
        SLOT(changeStatusMessage(QString, int)));
    connect(fakeVim_.data(),
        SIGNAL(extraInformationChanged(QString)),
        SLOT(changeExtraInformation(QString)));
    connect(fakeVim_.data(),
        SIGNAL(statusDataChanged(QString)),
        SLOT(changeStatusData(QString)));
    connect(fakeVim_.data(),
        SIGNAL(highlightMatches(QString)),
        SLOT(highlightMatches(QString)));
    connect(fakeVim_.data(),
        SIGNAL(handleExCommandRequested(bool*,ExCommand)),
        SLOT(handleExCommand(bool*,ExCommand)));
    connect(fakeVim_.data(),
        SIGNAL(requestSetBlockSelection(bool)),
        SLOT(requestSetBlockSelection(bool)));
    connect(fakeVim_.data(),
        SIGNAL(requestHasBlockSelection(bool*)),
        SLOT(requestHasBlockSelection(bool*)));
    connect(this,
        SIGNAL(handleInput(QString)),
        fakeVim_.data(),
        SLOT(handleInput(QString)));
    fakeVim_->handleCommand("set nopasskeys");
    fakeVim_->handleCommand("set nopasscontrolkey");

    // Set some Vim options.
    fakeVim_->handleCommand("set expandtab");
    fakeVim_->handleCommand("set shiftwidth=8");
    fakeVim_->handleCommand("set tabstop=16");
    fakeVim_->handleCommand("set autoindent");

    // Try to source file "fakevimrc" from current directory.
    fakeVim_->handleCommand("source fakevimrc");

    fakeVim_->installEventFilter();
    fakeVim_->setupWidget();
    setUndoRedoEnabled(false);
    setUndoRedoEnabled(true);
  }
  void VimEditor::paintEvent(QPaintEvent* e)
  {
    super::paintEvent(e);
    if (!cursorRect_.isNull() && e->rect().intersects(cursorRect_))
    {
      auto rect = cursorRect_;
      cursorRect_ = QRect();
      super::viewport()->update(rect);
    }
    auto rect = super::cursorRect();
    if (e->rect().intersects(rect))
    {
      QPainter painter(super::viewport());
      if (super::overwriteMode())
      {
        QFontMetrics fm(super::font());
        const auto position = super::textCursor().position();
        const auto c = super::document()->characterAt(position);
        rect.setWidth(fm.width(c));
        painter.setPen(Qt::NoPen);
        painter.setBrush(super::palette().color(QPalette::Base));
        painter.setCompositionMode(QPainter::CompositionMode_Difference);
      } else {
        rect.setWidth(super::cursorWidth());
        painter.setPen(super::palette().color(QPalette::Text));
      }
      painter.drawRect(rect);
      cursorRect_ = rect;
    }
  }
  void VimEditor::openFile(const QString& fileName)
  {
    emit handleInput(QString(":r %1<CR>").arg(fileName));
    fileName_ = fileName;
  }
  void VimEditor::changeStatusData(const QString& info)
  {
    statusData_ = info;
    updateStatusBar();
  }
  void VimEditor::highlightMatches(const QString& pattern)
  {
    auto cur = textCursor();
    QTextEdit::ExtraSelection selection;
    selection.format.setBackground(Qt::yellow);
    selection.format.setForeground(Qt::black);
    auto doc = document();
    QRegExp re(pattern);
    cur = doc->find(re);
    searchSelection_.clear();
    auto a = cur.position();
    while (!cur.isNull()) {
      if (cur.hasSelection()) {
        selection.cursor = cur;
        searchSelection_.append(selection);
      } else {
        cur.movePosition(QTextCursor::NextCharacter);
      }
      cur = doc->find(re, cur);
      auto b = cur.position();
      if (a == b) {
        cur.movePosition(QTextCursor::NextCharacter);
        cur = doc->find(re, cur);
        b = cur.position();
        if (a == b) break;
      }
      a = b;
    }
    updateExtraSelections();
  }
  void VimEditor::changeStatusMessage(const QString &contents, int cursorPos)
  {
      statusMessage_ = cursorPos == -1 ?
        contents
        : contents.left(cursorPos) + QChar(10073) + contents.mid(cursorPos);
      updateStatusBar();
  }
  void VimEditor::changeExtraInformation(const QString &info)
  {
      QMessageBox::information(this, tr("Information"), info);
  }
  void VimEditor::updateStatusBar()
  {
      auto slack = 80 - statusMessage_.size() - statusData_.size();
      auto msg = statusMessage_ + QString(slack, QLatin1Char(' ')) + statusData_;
      emit statusMessageChanged(msg);
  }
  void VimEditor::handleExCommand(bool *handled, const ExCommand &cmd)
  {
      if ( wantSaveAndQuit(cmd) ) {
          // :wq
          if (save())
              cancel();
      } else if ( wantSave(cmd) ) {
          save(); // :w
      } else if ( wantQuit(cmd) ) {
          if (cmd.hasBang)
              invalidate(); // :q!
          else
              cancel(); // :q
      } else {
          *handled = false;
          return;
      }
      *handled = true;
  }
  void VimEditor::requestSetBlockSelection(bool on)
  {
      auto pal = parentWidget() != NULL ?
        parentWidget()->palette()
        : QApplication::palette();
      blockSelection_.clear();
      clearSelection_.clear();
      if (on) {
        auto cur = textCursor();
        QTextEdit::ExtraSelection selection;
        selection.format.setBackground(pal.color(QPalette::Base));
        selection.format.setForeground(pal.color(QPalette::Text));
        selection.cursor = cur;
        clearSelection_.append(selection);
        selection.format.setBackground(pal.color(QPalette::Highlight));
        selection.format.setForeground(pal.color(QPalette::HighlightedText));
        auto from = cur.positionInBlock();
        auto to = cur.anchor() - cur.document()->findBlock(cur.anchor()).position();
        const auto min = qMin(cur.position(), cur.anchor());
        const auto max = qMax(cur.position(), cur.anchor());
        for (auto block = cur.document()->findBlock(min); block.isValid() && block.position() < max; block = block.next()) {
            cur.setPosition(block.position() + qMin(from, block.length()));
            cur.setPosition(block.position() + qMin(to, block.length()), QTextCursor::KeepAnchor);
            selection.cursor = cur;
            blockSelection_.append(selection);
        }
        connect(this, SIGNAL(selectionChanged()), SLOT(requestSetBlockSelection()), Qt::UniqueConnection );
        auto pal2 = palette();
        pal2.setColor(QPalette::Highlight, Qt::transparent);
        pal2.setColor(QPalette::HighlightedText, Qt::transparent);
        setPalette(pal2);
      } else {
          setPalette(pal);
          disconnect(this, SIGNAL(selectionChanged()), this, SLOT(requestSetBlockSelection()));
      }
      updateExtraSelections();
  }
  void VimEditor::requestHasBlockSelection(bool *on)
  {
      *on = !blockSelection_.isEmpty();
  }
  void VimEditor::parseArguments()
  {
      //auto args = qApp->arguments();
      //const QString editFileName = args.value(1, QString(_("/usr/share/vim/vim74/tutor/tutor")));
      //openFile(editFileName);
      //foreach (const QString &cmd, args.mid(2))
      //    emit handleInput(cmd);
  }
  void VimEditor::updateExtraSelections()
  {
    setExtraSelections(clearSelection_ + searchSelection_ + blockSelection_);
  }
  bool VimEditor::wantSaveAndQuit(const ExCommand &cmd)
  {
      return cmd.cmd == "wq";
  }
  bool VimEditor::wantSave(const ExCommand &cmd)
  {
      return cmd.matches("w", "write") || cmd.matches("wa", "wall");
  }
  bool VimEditor::wantQuit(const ExCommand &cmd)
  {
      return cmd.matches("q", "quit") || cmd.matches("qa", "qall");
  }
  bool VimEditor::save()
  {
      if (!hasChanges())
          return true;
      QTemporaryFile tmpFile;
      if (!tmpFile.open()) {
          QMessageBox::critical(this, tr("FakeVim Error"), tr("Cannot create temporary file: %1").arg(tmpFile.errorString()));
          return false;
      }
      QTextStream ts(&tmpFile);
      ts << content();
      ts.flush();
      QFile::remove(fileName_);
      if (!QFile::copy(tmpFile.fileName(), fileName_)) {
          QMessageBox::critical(this, tr("FakeVim Error"), tr("Cannot write to file \"%1\"").arg(fileName_));
          return false;
      }
      return true;
  }
  void VimEditor::cancel()
  {
      if (hasChanges()) {
          QMessageBox::critical(this, tr("FakeVim Warning"), tr("File \"%1\" was changed").arg(fileName_));
      } else {
          invalidate();
      }
  }
  void VimEditor::invalidate()
  {
      QApplication::quit();
  }
  bool VimEditor::hasChanges()
  {
      if (fileName_.isEmpty() && content().isEmpty())
          return false;
      QFile f(fileName_);
      if (!f.open(QIODevice::ReadOnly))
          return true;
      QTextStream ts(&f);
      return content() != ts.readAll();
  }
  QString VimEditor::content() const
  {
      return document()->toPlainText();
  }
}
// vim:set ts=2 sw=2 et:
