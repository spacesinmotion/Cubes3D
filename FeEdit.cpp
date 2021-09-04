#include "FeEdit.h"

#include <QAbstractItemView>
#include <QColor>
#include <QColorDialog>
#include <QCompleter>
#include <QDebug>
#include <QKeyEvent>
#include <QScrollBar>
#include <QShortcut>

QTextCursor xto_outer(QTextCursor c, bool select = false)
{
  auto *d = c.document();
  int i = 1;
  while (!c.atStart())
  {
    c.movePosition(c.Left, select ? c.KeepAnchor : c.MoveAnchor);
    auto x = d->characterAt(c.position());
    if (x == '(')
    {
      --i;
      if (i == 0)
        break;
    }
    else if (x == ')')
      ++i;
  }
  return c;
}

QTextCursor xto_outer_end(QTextCursor c, bool select = false)
{
  auto *d = c.document();
  int i = 1;
  while (!c.atEnd())
  {
    c.movePosition(c.Right, select ? c.KeepAnchor : c.MoveAnchor);
    auto x = d->characterAt(c.position());
    if (x == ')')
    {
      --i;
      if (i == 0)
      {
        c.movePosition(c.Right, select ? c.KeepAnchor : c.MoveAnchor);
        break;
      }
    }
    else if (x == '(')
      ++i;
  }
  return c;
}

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

  connect(new QShortcut(Qt::ControlModifier + Qt::Key_Return, this), &QShortcut::activated, this,
          [this] { specialEditDialog(); });
}

void FeEdit::keyPressEvent(QKeyEvent *ke)
{
  if (m_complete->popup()->isVisible())
  {
    switch (ke->key())
    {
    case Qt::Key_Enter:
    case Qt::Key_Return:
    case Qt::Key_Escape:
    case Qt::Key_Tab:
    case Qt::Key_Backtab:
      ke->ignore();
      return;
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

void FeEdit::cursorToOuterList(bool select)
{
  auto c = textCursor();
  if (select)
  {
    c = xto_outer_end(c);
    c.movePosition(c.Left, c.KeepAnchor);
  }
  c = xto_outer(c, select);
  setTextCursor(c);
}

void FeEdit::specialEditDialog()
{
  const auto c = textCursor();
  cursorToOuterList(true);
  auto t = textCursor().selectedText();
  if (t.startsWith("(color"))
  {
    t.replace("(", "").replace(")", "");
    auto es = t.split(' ', Qt::SkipEmptyParts);
    if (es.size() >= 4)
    {
      QColor c(es[1].toInt(), es[2].toInt(), es[3].toInt());
      c = QColorDialog::getColor(c, this);
      if (c.isValid())
      {
        const auto text = QString("(color %1 %2 %3)").arg(c.red()).arg(c.green()).arg(c.blue());
        textCursor().insertText(text);
      }
    }
  }

  setTextCursor(c);
}
