#include "FeEdit.h"

#include "fesyntaxhighlighter.h"

#include <QAbstractItemView>
#include <QApplication>
#include <QClipboard>
#include <QColor>
#include <QColorDialog>
#include <QCompleter>
#include <QDebug>
#include <QDir>
#include <QKeyEvent>
#include <QScrollBar>
#include <QShortcut>

QTextCursor to_outer(QTextCursor c, bool select = false)
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

QTextCursor to_outer_end(QTextCursor c, bool select = false)
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
  QStringList forms = {"()",
                       "(vec3 0 0 0)",
                       "(color 0 0 0)",
                       "(fn (&1) &2)",
                       "(mac (&1) &2)",
                       "(if &1 &2)",
                       "(while &1 &2)",
                       "(= &1 &2)",
                       "(let &1 &2)",
                       "(rotateX 0 &1)",
                       "(rotateY 0 &1)",
                       "(rotateZ 0 &1)",
                       "(rotate 0 (vec3 1 0 0) &1)",
                       "(translate (vec3 0 0 0) &1)",
                       "(lfo &1 &2 &3)",
                       "(cube (vec3 1 1 1) (color 0 0 0))"};
  for (const auto *x : {"quote", "and", "or",    "do",    "cons", "car", "cdr", "setcar", "setcdr",    "list",
                        "not",   "is",  "atom",  "print", "sin",  "cos", "tan", "asin",   "acos",      "atan",
                        "deg",   "rad", "floor", "ceil",  "sqrt", "abs", "max", "min",    "animation", "require"})
    forms << QString("(%1 &1)").arg(x);
  m_complete = new QCompleter(forms, this);
  m_complete->setCaseSensitivity(Qt::CaseInsensitive);
  m_complete->setWidget(this);

  connect(m_complete, qOverload<const QString &>(&QCompleter::activated), this,
          [this](const auto &t) { insertCompletion(t); });

  connect(new QShortcut(Qt::ControlModifier + Qt::Key_Return, this), &QShortcut::activated, this,
          [this] { specialEditDialog(); });

  new FeSyntaxHighlighter(document());

  connect(this, &QTextEdit::cursorPositionChanged, this, &FeEdit::additionalHighlights);
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

  const auto match = [ke](auto m, auto k, auto cb) {
    if (ke->modifiers() == m && ke->key() == k)
    {
      cb();
      return true;
    }
    return false;
  };
  if (match(Qt::ControlModifier, Qt::Key_X, [this] { cutSelection(); }) ||
      match(Qt::ControlModifier, Qt::Key_C, [this] { copySelection(); }) ||
      match(Qt::ControlModifier, Qt::Key_V, [this] { insertSelection(); }) ||
      match(Qt::AltModifier, Qt::Key_Up, [this] { cursorToOuterList(); }) ||
      match(Qt::AltModifier, Qt::Key_Down, [this] { cursorToInnerList(); }) ||
      match(Qt::AltModifier, Qt::Key_Right, [this] { cursorToNextInList(); }) ||
      match(Qt::AltModifier, Qt::Key_Left, [this] { cursorToPrevInList(); }) ||
      match(Qt::AltModifier | Qt::ShiftModifier, Qt::Key_Up, [this] { cursorToOuterList(true); }) ||
      match(Qt::AltModifier | Qt::ShiftModifier, Qt::Key_Down, [this] { cursorToInnerList(true); }) ||
      match(Qt::AltModifier | Qt::ShiftModifier, Qt::Key_Right, [this] { cursorToNextInList(true); }) ||
      match(Qt::AltModifier | Qt::ShiftModifier, Qt::Key_Left, [this] { cursorToPrevInList(true); }) ||
      match(Qt::AltModifier, Qt::Key_Home, [this] { cursorToListStart(); }) ||
      match(Qt::AltModifier | Qt::ShiftModifier, Qt::Key_Home, [this] { cursorToListStart(true); }) ||
      match(Qt::AltModifier, Qt::Key_End, [this] { cursorToListEnd(); }) ||
      match(Qt::AltModifier | Qt::ShiftModifier, Qt::Key_End, [this] { cursorToListEnd(true); }) ||
      match(Qt::NoModifier, Qt::Key_F2, [this] { goToDefinition(); }) ||
      match(Qt::ControlModifier, Qt::Key_D, [this] { duplicateSelection(); }))
    return;

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

void FeEdit::additionalHighlights()
{
  QList<QTextEdit::ExtraSelection> extraSelections;

  QTextEdit::ExtraSelection selection;
  selection.format.setBackground(QColor(153, 211, 218, 50));
  selection.format.setProperty(QTextFormat::FullWidthSelection, true);
  selection.cursor = textCursor();
  selection.cursor.clearSelection();
  extraSelections.append(selection);

  selection.format.setBackground(QColor(103, 161, 88, 100));
  selection.cursor = textCursor();
  int closed = 1;
  do
  {
    selection.cursor.clearSelection();
    selection.cursor.movePosition(QTextCursor::Left, QTextCursor::KeepAnchor);
    const auto t = selection.cursor.selectedText();
    if (t == ")")
      ++closed;
    else if (t == "(")
    {
      --closed;
      if (closed == 0)
      {
        extraSelections.append(selection);
        break;
      }
    }
  } while (selection.cursor.position() > 0);

  selection.cursor = textCursor();
  closed = 1;
  do
  {
    selection.cursor.clearSelection();
    selection.cursor.movePosition(QTextCursor::Right, QTextCursor::KeepAnchor);
    const auto t = selection.cursor.selectedText();
    if (t == "(")
      ++closed;
    else if (t == ")")
    {
      --closed;
      if (closed == 0)
      {
        extraSelections.append(selection);
        break;
      }
    }
  } while (!selection.cursor.atEnd());

  setExtraSelections(extraSelections);
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

void FeEdit::duplicateSelection()
{
  auto c = textCursor();
  if (c.hasSelection())
  {
    const auto d = c.selectedText();
    c.clearSelection();
    c.insertText(d);
    c.movePosition(c.Left, c.KeepAnchor, d.size());
  }
  else
  {
    c.select(c.LineUnderCursor);
    const auto line = c.selectedText();
    c.movePosition(c.EndOfLine);
    c.insertText("\n" + line);
  }
  setTextCursor(c);
}

void FeEdit::copySelection(bool remove)
{
  auto c = textCursor();
  auto d = c.selectedText();
  if (d.isEmpty())
  {
    c.select(c.LineUnderCursor);
    c.movePosition(c.Right, c.KeepAnchor);
    d = "__::__" + c.selectedText();
  }
  if (remove)
    c.removeSelectedText();

  qApp->clipboard()->setText(d);

  if (remove)
    setTextCursor(c);
}

void FeEdit::cutSelection()
{
  copySelection(true);
}

void FeEdit::insertSelection()
{
  auto c = textCursor();
  auto t = qApp->clipboard()->text();
  if (t.startsWith("__::__"))
  {
    c.movePosition(c.StartOfLine);
    t = t.mid(6);
  }
  c.insertText(t);
}

void FeEdit::cursorToOuterList(bool select)
{
  auto c = textCursor();
  if (select)
  {
    c = to_outer_end(c);
    c.movePosition(c.Left, c.KeepAnchor);
  }
  c = to_outer(c, select);
  setTextCursor(c);
}

void FeEdit::cursorToInnerList(bool select)
{
  auto c = textCursor();
  auto *d = c.document();
  while (!c.atEnd() && d->characterAt(c.position()).isSpace())
    c.movePosition(c.Right, select ? c.KeepAnchor : c.MoveAnchor);
  if (d->characterAt(c.position()) == '(')
    c.movePosition(c.Right, select ? c.KeepAnchor : c.MoveAnchor);
  setTextCursor(c);
}

void FeEdit::cursorToNextInList(bool select)
{
  auto c = textCursor();
  auto *d = c.document();
  if (d->characterAt(c.position()) == '(')
  {
    c.movePosition(c.Right, select ? c.KeepAnchor : c.MoveAnchor);
    c = to_outer_end(c, select);
  }
  while (!c.atEnd() && !d->characterAt(c.position()).isSpace() && d->characterAt(c.position()) != ')')
    c.movePosition(c.Right, select ? c.KeepAnchor : c.MoveAnchor);
  while (!c.atEnd() && d->characterAt(c.position()).isSpace())
    c.movePosition(c.Right, select ? c.KeepAnchor : c.MoveAnchor);
  setTextCursor(c);
}

void FeEdit::cursorToPrevInList(bool select)
{
  auto c = textCursor();
  auto *d = c.document();

  c.movePosition(c.Left, select ? c.KeepAnchor : c.MoveAnchor);
  if (d->characterAt(c.position()) == '(')
    return;

  while (!c.atStart() && d->characterAt(c.position()).isSpace())
    c.movePosition(c.Left, select ? c.KeepAnchor : c.MoveAnchor);
  if (d->characterAt(c.position()) == ')')
  {
    c = to_outer(c, select);
  }
  else
  {
    while (!c.atStart() && !d->characterAt(c.position()).isSpace() && d->characterAt(c.position()) != '(')
      c.movePosition(c.Left, select ? c.KeepAnchor : c.MoveAnchor);
    c.movePosition(c.Right, select ? c.KeepAnchor : c.MoveAnchor);
  }
  setTextCursor(c);
}

void FeEdit::cursorToListStart(bool select)
{
  auto c = textCursor();
  c = to_outer(c, select);
  c.movePosition(c.Right, select ? c.KeepAnchor : c.MoveAnchor);
  setTextCursor(c);
}

void FeEdit::cursorToListEnd(bool select)
{
  auto c = textCursor();
  c = to_outer_end(c, select);
  c.movePosition(c.Left, select ? c.KeepAnchor : c.MoveAnchor);
  setTextCursor(c);
}

void FeEdit::goToDefinition()
{
  const auto backUp = textCursor();

  cursorToOuterList(true);
  auto selection = textCursor().selectedText();
  setTextCursor(backUp);

  if (selection.startsWith("(require "))
  {
    auto s = selection.replace("(", "").replace(")", "").split(' ', Qt::SkipEmptyParts);
    if (s.size() >= 2)
      emit requestGoToFile(s[1].replace("\"", "").replace(".", QDir::separator()).trimmed() + ".fe");
  }
}
