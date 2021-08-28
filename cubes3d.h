#ifndef CUBES3D_H
#define CUBES3D_H

#include "fewrap.h"

#include <QMainWindow>

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

private:
  void closeEvent(QCloseEvent *e);
  void timerEvent(QTimerEvent *t);

  void highlightBraces();

private:
  Ui::Cubes3D *ui;

  QString m_feFile;
  FeWrap m_feWrap;

  QVector<QPixmap> m_animation;
  int m_animationStep{0};

  static const int w = 32, h = 64, s = 4;
};

#endif // CUBES3D_H
