#include "cubes3d.h"

#include "ui_cubes3d.h"

#include <QAbstractItemView>
#include <QClipboard>
#include <QCompleter>
#include <QFileDialog>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QLineEdit>
#include <QPainter>
#include <QSettings>
#include <QShortcut>
#include <QStatusBar>
#include <QTimer>

Cubes3D::Cubes3D(QWidget *parent)
  : QMainWindow(parent)
  , ui(new Ui::Cubes3D)
{
  ui->setupUi(this);

  m_feWrap = std::make_unique<FeWrap>(*ui->view3d);

  m_lineColumn = new QLabel{QString("0,0"), this};
  m_lineColumn->setFont(ui->teFeIn->font());
  statusBar()->addPermanentWidget(m_lineColumn);
  connect(ui->teFeIn, &QTextEdit::cursorPositionChanged, this, [this] {
    const auto c = ui->teFeIn->textCursor();
    m_lineColumn->setText(QString("%1, %2").arg(c.blockNumber() + 1, 3).arg(c.columnNumber(), 3));
  });

  QSettings s;
  restoreGeometry(s.value("Main/geomtry").toByteArray());
  restoreState(s.value("Main/state").toByteArray());
  ui->spTexts->restoreState(s.value("Main/spTexts").toByteArray());
  ui->spViews->restoreState(s.value("Main/spViews").toByteArray());
  ui->spLeftRight->restoreState(s.value("Main/spLeftRight").toByteArray());
  QSignalBlocker block(ui->cbAnimation);
  ui->cbAnimation->addItems({s.value("Main/RecentAnimation").toString()});

  addAction("select animation", [this] { selectAnimation(); });
  addAction("open recent file", [this] { openRecentFile(); });
  addAction("update animation", [this] { updateAnimation(); });
  addAction("go to definition", Qt::ControlModifier + Qt::Key_M, [this] { goToDefinition(); });
  addAction("go to file", Qt::ControlModifier + Qt::Key_Tab, [this] { goToFile(); });
  addAction("export sprite map", [this] { exportSpriteMap(); });

  new QShortcut(Qt::Key_F1, this, [this] { showCommands(); });
  new QShortcut(Qt::Key_Escape, this, [this] {
    delete m_commandPanel;
    ui->teFeIn->setFocus();
  });

  connect(ui->teFeIn->document(), &QTextDocument::contentsChanged, this, [this] { setEditFile(m_editFile); });

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

void Cubes3D::open_file(const QString &f)
{
  QApplication::setOverrideCursor(Qt::WaitCursor);

  m_feFile = f;

  setEditFile(m_feWrap->newSession(m_feFile));
  eval_main();

  ui->teFeIn->document()->setModified(false);
  setEditFile(m_editFile);

  QSettings s;
  auto recent = s.value("Main/RecentFiles").toStringList();
  recent.removeAll(f);
  recent.prepend(f);
  s.setValue("Main/RecentFiles", recent);

  QApplication::restoreOverrideCursor();
}

void Cubes3D::on_actionnew_triggered()
{
  auto f = QFileDialog::getSaveFileName(this, tr("New scene file"), m_feFile, tr("Scene (*.fe)"));
  if (f.isEmpty())
    return;

  {
    QFile fe(f);
    fe.open(QFile::WriteOnly);
    fe.write(R"(
(= lp (vec3 0.4 -3 5))

(= cubian (fn ()
  (rotateZ (fn (t) (deg t))
    (cube (vec3 0.25) (color 200 200 200)))))
    
(animation "guy_front" 1 lp
  (cubian)))");
  }

  open_file(f);
}

void Cubes3D::on_actionopen_triggered()
{
  auto f = QFileDialog::getOpenFileName(this, tr("Open scene file"), m_feFile, tr("Scene (*.fe)"));
  if (f.isEmpty())
    return;

  open_file(f);
}

void Cubes3D::on_actionsave_triggered()
{
  QApplication::setOverrideCursor(Qt::WaitCursor);

  m_feWrap->setCodeOf(m_editFile, ui->teFeIn->toPlainText());
  eval_main();

  m_feWrap->saveFiles();
  ui->teFeIn->document()->setModified(false);
  setEditFile(m_editFile);

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
  if (m_animationHelper)
  {
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

void Cubes3D::setEditFile(const QString &f)
{
  m_editFile = f;
  const auto changed = m_feWrap->hasChanges() || ui->teFeIn->document()->isModified();
  setWindowTitle(QApplication::applicationName() + QString("%2 (%1)").arg(m_editFile).arg(changed ? "*" : ""));
}

void Cubes3D::showCommandPanel(const QStringList &list, const std::function<void(const QString &)> &cb)
{
  if (!m_commandPanel)
  {
    m_commandPanel = new QLineEdit(this);
    m_commandPanel->setFont(ui->teFeIn->font());

    auto *c = new QCompleter(list, m_commandPanel);
    c->setCaseSensitivity(Qt::CaseInsensitive);
    c->popup()->setFont(ui->teFeIn->font());
    connect(c, qOverload<const QString &>(&QCompleter::activated), this, [this, cb](const auto &t) {
      delete m_commandPanel;
      cb(t);
    });
    m_commandPanel->setCompleter(c);
  }

  const auto w = width() / 2;
  const auto h = m_commandPanel->sizeHint().height();
  m_commandPanel->setGeometry(QRect(width() / 4, 0, w, h));

  m_commandPanel->show();
  m_commandPanel->setFocus();
  m_commandPanel->completer()->complete(m_commandPanel->rect());
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

void Cubes3D::goToDefinition()
{
  const auto fe = ui->teFeIn->toPlainText();

  QList<QPair<int, QString>> def;
  int maxLength = 0;
  m_feWrap->eachDefinitionAtLine(fe, [&](int l, const auto &d) {
    def << qMakePair(l, d);
    maxLength = std::max(maxLength, d.size());
  });

  maxLength += 4;
  QStringList defNames;
  for (const auto &d : def)
    defNames << QString("%1%3:%2").arg(d.second).arg(d.first, 3).arg(QString(maxLength - d.second.size(), ' '));

  showCommandPanel(defNames, [this](const auto &s) {
    const auto line = s.split(":").back().toInt() - 1;
    auto c = ui->teFeIn->textCursor();
    c.movePosition(c.StartOfLine);
    while (c.blockNumber() > line && !c.atStart())
      c.movePosition(c.Up);
    while (c.blockNumber() < line && !c.atEnd())
      c.movePosition(c.Down);
    ui->teFeIn->setTextCursor(c);
    ui->teFeIn->setFocus();
  });
}

void Cubes3D::goToFile()
{
  showCommandPanel(m_feWrap->usedFiles(), [this](const auto &s) {
    m_feWrap->setCodeOf(m_editFile, ui->teFeIn->toPlainText());
    ui->teFeIn->setText(m_feWrap->codeOf(s));

    setEditFile(s);
    ui->teFeIn->setFocus();
  });
}

void Cubes3D::exportSpriteMap()
{
  const auto currentAnimation = ui->cbAnimation->currentIndex();
  m_animationHelper = false;
  if (currentAnimation == 0)
    updateAnimation();

  QVector<QVector<QPixmap>> allAnimations;
  for (int i = 0; i < ui->cbAnimation->count(); ++i)
  {
    ui->cbAnimation->setCurrentIndex(i);
    allAnimations << m_animation;
  }
  m_animationHelper = true;
  ui->cbAnimation->setCurrentIndex(currentAnimation);

  QSize s(0, 0);
  for (const auto &a : allAnimations)
  {
    s.setHeight(s.height() + a.front().height());
    s.setWidth(std::max(s.width(), a.size() * a.front().width()));
  }

  QPixmap all(s);
  all.fill(Qt::transparent);
  QPainter p(&all);

  QPoint tl(0, 0);
  QJsonArray jsonAnimations;
  int index = 0;
  for (const auto &a : allAnimations)
  {
    QJsonArray jsonSprites;

    for (const auto &f : a)
    {
      p.drawPixmap(tl, f);

      jsonSprites.append(QJsonObject{
          {"x", tl.x()},
          {"y", tl.y()},
          {"w", f.width()},
          {"h", f.height()},
      });
      tl.rx() += f.width();
    }
    tl.rx() = 0;
    tl.ry() += a.front().height();

    jsonAnimations.append(QJsonObject{
        {"name", ui->cbAnimation->itemText(index++)},
        {"sprites", jsonSprites},
    });
  }

  auto *l = new QLabel;
  l->setAttribute(Qt::WA_DeleteOnClose);
  l->setPixmap(all);
  l->show();

  const auto baseName = QFileInfo(m_feFile).baseName();
  const auto d = QFileInfo(m_feFile).dir();
  all.save(d.absoluteFilePath(baseName + ".png"));

  QFile jsonFile(d.absoluteFilePath(baseName + ".json"));
  jsonFile.open(QFile::WriteOnly);
  jsonFile.write(QJsonDocument(QJsonObject{
                                   {"file", baseName + ".png"},
                                   {"animations", jsonAnimations},
                               })
                     .toJson(QJsonDocument::Indented));
}

void Cubes3D::addAction(const QString &name, const QKeySequence &ks, const std::function<void()> &t)
{
  auto *ac = new QAction(name, this);
  connect(ac, &QAction::triggered, this, t);
  QMainWindow::addAction(ac);
  if (!ks.isEmpty())
    new QShortcut(ks, this, t);
}

void Cubes3D::addAction(const QString &name, const std::function<void()> &t)
{
  addAction(name, {}, t);
}

void Cubes3D::eval_main()
{
  ui->teFeOut->clear();
  ui->view3d->clear_scene();
  try
  {
    ui->teFeOut->setTextColor(Qt::black);
    ui->teFeOut->append(m_feWrap->eval());
  }
  catch (const std::exception &e)
  {
    ui->teFeOut->setTextColor(Qt::red);
    ui->teFeOut->append(QString("ERROR: ") + e.what());
    return;
  }

  updateAnimationList();

  try
  {
    const auto p = ui->teFeIn->textCursor().position();
    ui->teFeIn->setText(m_feWrap->format(m_feWrap->codeOf(m_editFile)));

    auto c = ui->teFeIn->textCursor();
    c.movePosition(c.Right, c.MoveAnchor, p);
    ui->teFeIn->setTextCursor(c);
  }
  catch (const std::runtime_error &e)
  {
    ui->teFeOut->setTextColor(Qt::darkRed);
    ui->teFeOut->append(QString("FORMAT_ERROR: ") + e.what());
  }
}
