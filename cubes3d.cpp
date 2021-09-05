#include "cubes3d.h"

#include "ui_cubes3d.h"

#include <QClipboard>
#include <QCompleter>
#include <QFileDialog>
#include <QLineEdit>
#include <QPainter>
#include <QSettings>
#include <QShortcut>
#include <QTimer>

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

  connect(new QShortcut(Qt::Key_F1, this), &QShortcut::activated, this, [this] { showCommands(); });
  connect(new QShortcut(Qt::Key_Escape, this), &QShortcut::activated, this, [this] { delete m_commandPanel; });

  addAction("select animation", [this] { selectAnimation(); });
  addAction("open recent file", [this] { openRecentFile(); });
  addAction("update animation", [this] { updateAnimation(); });

  QTimer::singleShot(10, this, [this] {
    QSettings s;
    const auto recent = s.value("Main/RecentFiles").toStringList();
    if (!recent.isEmpty())
      open_file(recent.front());
  });

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
  {}
  return false;
}

void Cubes3D::open_file(const QString &f)
{
  QApplication::setOverrideCursor(Qt::WaitCursor);
  m_feFile = f;

  QFile fi(f);
  if (fi.open(QFile::ReadOnly))
  {
    QDir::setCurrent(QFileInfo(m_feFile).absolutePath());
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

void Cubes3D::showCommandPanel(const QStringList &list, const std::function<void(const QString &)> &cb)
{
  if (!m_commandPanel)
    m_commandPanel = new QLineEdit(this);

  auto *c = new QCompleter(list, m_commandPanel);
  c->setCaseSensitivity(Qt::CaseInsensitive);
  connect(c, qOverload<const QString &>(&QCompleter::activated), this, [this, cb](const auto &t) {
    delete m_commandPanel;
    cb(t);
  });
  m_commandPanel->setCompleter(c);

  const auto w = width() / 2;
  const auto h = m_commandPanel->sizeHint().height();
  m_commandPanel->setGeometry(QRect(width() / 4, 0, w, h));

  m_commandPanel->show();
  m_commandPanel->setFocus();
  c->complete(m_commandPanel->rect());
}

void getAllActions(const QList<QAction *> &actions, QHash<QString, QAction *> &a)
{
  for (auto *ac : actions)
  {
    if (auto *m = ac->menu())
      getAllActions(m->actions(), a);
    else if (!ac->isSeparator())
      a[ac->text()] = ac;
  }
}

void Cubes3D::showCommands()
{
  QHash<QString, QAction *> allActions;
  getAllActions(ui->menuBar->actions(), allActions);
  getAllActions(actions(), allActions);
  showCommandPanel(allActions.keys(), [allActions](const auto &s) {
    if (auto *a = allActions.value(s))
      a->trigger();
  });
}

void Cubes3D::selectAnimation()
{
  showCommandPanel(ui->view3d->animations(),
                   [this](const auto &s) { ui->cbAnimation->setCurrentIndex(ui->cbAnimation->findText(s)); });
}

void Cubes3D::openRecentFile()
{
  showCommandPanel(QSettings().value("Main/RecentFiles").toStringList(), [this](const auto &s) { open_file(s); });
}

void Cubes3D::addAction(const QString &name, const std::function<void()> &t)
{
  auto *ac = new QAction(name, this);
  connect(ac, &QAction::triggered, this, t);
  QMainWindow::addAction(ac);
}
