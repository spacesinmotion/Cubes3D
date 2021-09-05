#ifndef CUBES3D_H
#define CUBES3D_H

#include "fewrap.h"

#include <QMainWindow>
#include <QPointer>

class QLineEdit;

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

  bool eval_text(const QString &t);
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
  bool eventFilter(QObject *o, QEvent *e) final;

  void updateAnimation();
  void updateAnimationList();

  void showCommandPanel(const QStringList &, const std::function<void(const QString &)> &cb);
  void selectAnimation();

private:
  Ui::Cubes3D *ui;

  QPointer<QLineEdit> m_commandPanel;

  QString m_feFile;
  FeWrap m_feWrap;

  QVector<QPixmap> m_animation;
  int m_animationStep{0};

  static const int w = 24, h = 48, s = 3;
};

#endif // CUBES3D_H
