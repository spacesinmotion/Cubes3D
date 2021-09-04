#include "FeEdit.h"

#include <QAbstractItemView>
#include <QCompleter>
#include <QDebug>
#include <QKeyEvent>
#include <QScrollBar>

FeEdit::FeEdit(QWidget *parent)
  : QTextEdit(parent)
{
  QStringList forms = {"(vec3 0 0 0)",
                       "(color 0 0 0)",
                       "(fn (%1) %2)",
                       "(= %1 %2)",
                       "(rotateX 0 %1)",
                       "(rotateY 0 %1)",
                       "(rotateZ 0 %1)",
                       "(rotate 0 (vec3 1 0 0) %1)",
                       "(translate (vec3 0 0 0) %1)",
                       "(cube (vec3 1 1 1) (color 0 0 0))"};
  m_complete = new QCompleter(forms, this);
  m_complete->setCaseSensitivity(Qt::CaseInsensitive);
  m_complete->setWidget(this);

  connect(m_complete, qOverload<const QString &>(&QCompleter::activated), this,
          [this](const auto &t) { insertCompletion(t); });
}

void FeEdit::keyPressEvent(QKeyEvent *ke)
{
  if (m_complete->popup()->isVisible())
  {
    // The following keys are forwarded by the completer to the widget
    switch (ke->key())
    {
    case Qt::Key_Enter:
    case Qt::Key_Return:
    case Qt::Key_Escape:
    case Qt::Key_Tab:
    case Qt::Key_Backtab:
      ke->ignore();
      return; // let the completer do default behavior
    default:
      break;
    }
  }
  QTextEdit::keyPressEvent(ke);

  if (ke->text().isEmpty())
    return;

  const auto tc = textUnderCursor();
  const auto t = tc.selectedText();
  if (t.startsWith('('))
  {
    auto cr = cursorRect(tc);
    cr.setWidth(m_complete->popup()->sizeHintForColumn(0) +
                m_complete->popup()->verticalScrollBar()->sizeHint().width());
    m_complete->setCompletionPrefix(t);
    m_complete->popup()->setCurrentIndex(m_complete->completionModel()->index(0, 0));
    m_complete->complete(cr);
  }
  else
    m_complete->popup()->hide();
}

void FeEdit::focusInEvent(QFocusEvent *e)
{
  QTextEdit::focusInEvent(e);
}

void FeEdit::insertCompletion(const QString &completion)
{
  auto tc = textUnderCursor();
  const auto p = tc.position();
  tc.insertText(completion);
  tc.setPosition(p + 1);
  setTextCursor(tc);
}

QTextCursor FeEdit::textUnderCursor() const
{
  auto c = textCursor();
  auto *d = c.document();
  do
  {
    c.movePosition(c.Left, c.KeepAnchor);
  } while (!c.atStart() && !d->characterAt(c.position()).isSpace() && d->characterAt(c.position()) != '(');
  return c;
}
