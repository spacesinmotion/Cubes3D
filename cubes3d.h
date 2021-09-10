#ifndef CUBES3D_H
#define CUBES3D_H

#include "fewrap.h"

#include <QMainWindow>
#include <QPointer>

class QLineEdit;
class QLabel;

namespace Ui
{
class Cubes3D;
}

class Cubes3D : public QMainWindow
{
  Q_OBJECT

public:
  explicit Cubes3D(QWidget *parent = 0);
  ~Cubes3D();

  void open_file(const QString &t);

private slots:
  void on_actionopen_triggered();
  void on_actionsave_triggered();

  void on_menuRecent_aboutToShow();
  void on_actionclearRecent_triggered();

  void on_cbAnimation_currentIndexChanged(const QString &name);

private:
  void closeEvent(QCloseEvent *e) final;
  void timerEvent(QTimerEvent *t) final;

  void updateAnimation();
  void updateAnimationList();

  void setEditFile(const QString &f);

  void showCommandPanel(const QStringList &, const std::function<void(const QString &)> &cb);

  void showCommands();
  void selectAnimation();
  void openRecentFile();
  void goToDefinition();
  void goToFile();
  void exportSpriteMap();

  void addAction(const QString &name, const QKeySequence &ks, const std::function<void()> &t);
  void addAction(const QString &name, const std::function<void()> &t);

  void eval_main();

private:
  Ui::Cubes3D *ui;

  QPointer<QLineEdit> m_commandPanel;
  QLabel *m_lineColumn{nullptr};

  QString m_feFile;
  QString m_editFile;
  std::unique_ptr<FeWrap> m_feWrap;

  QVector<QPixmap> m_animation;
  int m_animationStep{0};
  bool m_animationHelper{true};

  static const int w = 24, h = 48, s = 3;
};

#endif // CUBES3D_H
