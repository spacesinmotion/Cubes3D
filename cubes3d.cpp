#include "cubes3d.h"

#include <QFileDialog>
#include <QSettings>
#include <QTimer>

#include "fesyntaxhighlighter.h"
#include "ui_cubes3d.h"

Cubes3D::Cubes3D(QWidget *parent) : QMainWindow(parent), ui(new Ui::Cubes3D)
{
  ui->setupUi(this);

  QSettings s;
  restoreGeometry(s.value("Main/geomtry").toByteArray());
  restoreState(s.value("Main/state").toByteArray());
  ui->splitter->restoreState(s.value("Main/splitter1state").toByteArray());
  ui->splitter_2->restoreState(s.value("Main/splitter2state").toByteArray());

  QTimer::singleShot(10, this, [this] {
    const auto recent = QSettings().value("Main/RecentFiles").toStringList();
    if (!recent.isEmpty())
      open_file(recent.front());
  });

  new FeSyntaxHighlighter(ui->teFeIn->document());

  connect(ui->teFeIn, &QTextEdit::cursorPositionChanged, this, &Cubes3D::highlightBraces);
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
  s.setValue("Main/splitter1state", ui->splitter->saveState());
  s.setValue("Main/splitter2state", ui->splitter_2->saveState());

  QMainWindow::closeEvent(e);
}

void Cubes3D::highlightBraces()
{
  QList<QTextEdit::ExtraSelection> extraSelections;

  QTextEdit::ExtraSelection selection;
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
  QApplication::restoreOverrideCursor();
}

void Cubes3D::on_actionopen_triggered()
{
  auto f = QFileDialog::getOpenFileName(this, tr("Open scene file"), m_feFile, tr("Scene (*.fe)"));
  if (f.isEmpty())
    return;

  QSettings s;
  auto recent = s.value("Main/RecentFiles").toStringList();
  recent.removeAll(f);
  recent.prepend(f);
  s.setValue("Main/RecentFiles", recent);

  open_file(f);
}

bool Cubes3D::eval_text(const QString &t)
{
  bool ok = true;

  ui->teFeOut->clear();
  ui->teFeOut->setTextColor(Qt::darkBlue);
  ui->teFeOut->append(t);
  try
  {
    ui->view3d->showScene(m_feWrap.eval(t, [this](const auto &o) {
      ui->teFeOut->setTextColor(Qt::black);
      ui->teFeOut->append(o);
    }));
  }
  catch (const std::exception &e)
  {
    ui->teFeOut->setTextColor(Qt::red);
    ui->teFeOut->append(QString("ERROR: ") + e.what());
    ok = false;
  }

  const auto p = ui->teFeIn->textCursor().position();
  ui->teFeIn->setText(m_feWrap.format(t));

  auto c = ui->teFeIn->textCursor();
  c.movePosition(c.Right, c.MoveAnchor, p);
  ui->teFeIn->setTextCursor(c);

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
