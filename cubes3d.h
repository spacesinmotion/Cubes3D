#ifndef CUBES3D_H
#define CUBES3D_H

#include <QMainWindow>

namespace Ui {
class Cubes3D;
}

class Cubes3D : public QMainWindow
{
  Q_OBJECT

public:
  explicit Cubes3D(QWidget *parent = 0);
  ~Cubes3D();

private:
  Ui::Cubes3D *ui;
};

#endif // CUBES3D_H
