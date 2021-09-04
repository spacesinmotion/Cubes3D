#include "cubes3d.h"

#include "fesyntaxhighlighter.h"
#include "ui_cubes3d.h"

#include <QClipboard>
#include <QFileDialog>
#include <QPainter>
#include <QSettings>
#include <QShortcut>
#include <QTimer>

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

Cubes3D::Cubes3D(QWidget *parent)
  : QMainWindow(parent)
  , ui(new Ui::Cubes3D)
{
  ui->setupUi(this);

  QSettings s;
  restoreGeometry(s.value("Main/geomtry").toByteArray());
  restoreState(s.value("Main/state").toByteArray());
  ui->spTexts->restoreState(s.value("Main/spTexts").toByteArray());
  ui->spViews->restoreState(s.value("Main/spViews").toByteArray());
  ui->spLeftRight->restoreState(s.value("Main/spLeftRight").toByteArray());
  QSignalBlocker block(ui->cbAnimation);
  ui->cbAnimation->addItems({s.value("Main/RecentAnimation").toString()});

  connect(new QShortcut(QKeySequence::Print, this), &QShortcut::activated, this, [this] { updateAnimation(); });
  ui->teFeIn->installEventFilter(this);

  for (auto *sc : ui->teFeIn->findChildren<QShortcut *>())
    qDebug() << sc->key();

  QTimer::singleShot(10, this, [this] {
    QSettings s;
    const auto recent = s.value("Main/RecentFiles").toStringList();
    if (!recent.isEmpty())
      open_file(recent.front());
  });

  new FeSyntaxHighlighter(ui->teFeIn->document());

  connect(ui->teFeIn, &QTextEdit::cursorPositionChanged, this, &Cubes3D::additionalHighlights);

  startTimer(1000 / 20);
}

Cubes3D::~Cubes3D()
{
  delete ui;
}

void Cubes3D::closeEvent(QCloseEvent *e)
{
  QSettings s;
  s.setValue("Main/geomtry", saveGeometry());
  s.setValue("Main/state", saveState());
  s.setValue("Main/spTexts", ui->spTexts->saveState());
  s.setValue("Main/spViews", ui->spViews->saveState());
  s.setValue("Main/spLeftRight", ui->spLeftRight->saveState());

  QMainWindow::closeEvent(e);
}

void Cubes3D::timerEvent(QTimerEvent *t)
{
  if (m_animation.empty())
    return;

  ui->laPixelPreview->setPixmap(m_animation[m_animationStep % m_animation.size()].scaled(w * s, h * s));
  ++m_animationStep;
}

bool Cubes3D::eventFilter(QObject *o, QEvent *e)
{
  if (o == ui->teFeIn && (e->type() == QEvent::KeyRelease || e->type() == QEvent::KeyPress))
  {
    const auto match = [e](auto m, auto k, auto cb) {
      const auto *ke = static_cast<QKeyEvent *>(e);
      if (ke->modifiers() == m && ke->key() == k)
      {
        if (e->type() == QEvent::KeyRelease)
          cb();
        return true;
      }
      return false;
    };
    return match(Qt::ControlModifier, Qt::Key_X, [this] { cutSelection(); }) ||
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
           match(Qt::ControlModifier, Qt::Key_D, [this] { duplicateSelection(); });
  }
  return false;
}

void Cubes3D::additionalHighlights()
{
  QList<QTextEdit::ExtraSelection> extraSelections;

  QTextEdit::ExtraSelection selection;
  selection.format.setBackground(QColor(153, 211, 218, 50));
  selection.format.setProperty(QTextFormat::FullWidthSelection, true);
  selection.cursor = ui->teFeIn->textCursor();
  selection.cursor.clearSelection();
  extraSelections.append(selection);

  selection.format.setBackground(QColor(103, 161, 88, 100));
  selection.cursor = ui->teFeIn->textCursor();
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

  selection.cursor = ui->teFeIn->textCursor();
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

  ui->teFeIn->setExtraSelections(extraSelections);
}

void Cubes3D::open_file(const QString &f)
{
  QApplication::setOverrideCursor(Qt::WaitCursor);
  m_feFile = f;

  QFile fi(f);
  if (fi.open(QFile::ReadOnly))
  {
    const auto fe = fi.readAll();
    ui->teFeIn->setText(fi.readAll());
    eval_text(fe);
  }

  QSettings s;
  auto recent = s.value("Main/RecentFiles").toStringList();
  recent.removeAll(f);
  recent.prepend(f);
  s.setValue("Main/RecentFiles", recent);

  QApplication::restoreOverrideCursor();
}

void Cubes3D::on_actionopen_triggered()
{
  auto f = QFileDialog::getOpenFileName(this, tr("Open scene file"), m_feFile, tr("Scene (*.fe)"));
  if (f.isEmpty())
    return;

  open_file(f);
}

bool Cubes3D::eval_text(const QString &t)
{
  bool ok = true;

  ui->teFeOut->clear();
  ui->view3d->clear_scene();
  try
  {
    const auto out = m_feWrap.eval(t, *ui->view3d);
    ui->teFeOut->setTextColor(Qt::black);
    ui->teFeOut->append(out);
  }
  catch (const std::exception &e)
  {
    ui->teFeOut->setTextColor(Qt::red);
    ui->teFeOut->append(QString("ERROR: ") + e.what());
    ok = false;
  }

  updateAnimationList();

  try
  {
    const auto p = ui->teFeIn->textCursor().position();
    ui->teFeIn->setText(m_feWrap.format(t));

    auto c = ui->teFeIn->textCursor();
    c.movePosition(c.Right, c.MoveAnchor, p);
    ui->teFeIn->setTextCursor(c);
  }
  catch (const std::runtime_error &e)
  {
    ui->teFeOut->setTextColor(Qt::darkRed);
    ui->teFeOut->append(QString("FORMAT_ERROR: ") + e.what());
    ok = false;
  }

  return ok;
}

void Cubes3D::on_actionsave_triggered()
{
  QApplication::setOverrideCursor(Qt::WaitCursor);
  const auto fe = ui->teFeIn->toPlainText();
  if (eval_text(fe) && !m_feFile.isEmpty())
  {
    QFile f(m_feFile);
    if (f.open(QFile::WriteOnly))
      f.write(fe.toLocal8Bit());
  }
  QApplication::restoreOverrideCursor();
}

void Cubes3D::on_menuRecent_aboutToShow()
{
  auto files = ui->menuRecent->actions();
  files.pop_back();
  auto *sep = files.back();
  files.pop_back();
  qDeleteAll(files);
  files.clear();

  for (const auto f : QSettings().value("Main/RecentFiles").toStringList())
  {
    files << new QAction(QFileInfo(f).fileName());
    connect(files.back(), &QAction::triggered, this, [this, f] { open_file(f); });
  }
  ui->menuRecent->insertActions(sep, files);
}

void Cubes3D::on_actionclearRecent_triggered()
{
  QSettings().setValue("Main/RecentFiles", QStringList{});
}

void Cubes3D::on_cbAnimation_currentIndexChanged(const QString &name)
{
  ui->view3d->showAnimation(name);
  updateAnimation();
  QSettings().setValue("Main/RecentAnimation", name);
}

void Cubes3D::updateAnimation()
{
  m_animation = ui->view3d->allFrames(w, h);
  int i = 0;
  for (auto &f : m_animation)
  {
    QPainter p(&f);
    p.drawLine(0, 0, int(i / double(m_animation.size()) * w), 0);
    p.drawPoint(w - 1, h - 1);
    p.drawPoint(0, h - 1);
    p.drawPoint(w - 1, 0);
    p.setPen(Qt::lightGray);
    p.drawPoint(0, h / 2);
    p.drawPoint(w - 1, h / 2);
    ++i;
  }
}

void Cubes3D::updateAnimationList()
{
  const auto last = ui->cbAnimation->currentText();
  QSignalBlocker block(ui->cbAnimation);
  ui->cbAnimation->clear();
  ui->cbAnimation->addItems(ui->view3d->animations());
  ui->cbAnimation->setCurrentIndex(std::max(0, ui->cbAnimation->findText(last)));
  on_cbAnimation_currentIndexChanged(ui->cbAnimation->currentText());
}

void Cubes3D::duplicateSelection()
{
  auto c = ui->teFeIn->textCursor();
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
  ui->teFeIn->setTextCursor(c);
}

void Cubes3D::copySelection(bool remove)
{
  auto c = ui->teFeIn->textCursor();
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
    ui->teFeIn->setTextCursor(c);
}

void Cubes3D::cutSelection()
{
  copySelection(true);
}

void Cubes3D::insertSelection()
{
  auto c = ui->teFeIn->textCursor();
  auto t = qApp->clipboard()->text();
  if (t.startsWith("__::__"))
  {
    c.movePosition(c.StartOfLine);
    t = t.mid(6);
  }
  c.insertText(t);
}

void Cubes3D::cursorToOuterList(bool select)
{
  auto c = ui->teFeIn->textCursor();
  if (select)
  {
    c = to_outer_end(c);
    c.movePosition(c.Left, c.KeepAnchor);
  }
  c = to_outer(c, select);
  ui->teFeIn->setTextCursor(c);
}

void Cubes3D::cursorToInnerList(bool select)
{
  auto c = ui->teFeIn->textCursor();
  auto *d = c.document();
  while (!c.atEnd() && d->characterAt(c.position()).isSpace())
    c.movePosition(c.Right, select ? c.KeepAnchor : c.MoveAnchor);
  if (d->characterAt(c.position()) == '(')
    c.movePosition(c.Right, select ? c.KeepAnchor : c.MoveAnchor);
  ui->teFeIn->setTextCursor(c);
}

void Cubes3D::cursorToNextInList(bool select)
{
  auto c = ui->teFeIn->textCursor();
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
  ui->teFeIn->setTextCursor(c);
}

void Cubes3D::cursorToPrevInList(bool select)
{
  auto c = ui->teFeIn->textCursor();
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
  ui->teFeIn->setTextCursor(c);
}
