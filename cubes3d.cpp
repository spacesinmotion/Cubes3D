#include "cubes3d.h"
#include "ui_cubes3d.h"

Cubes3D::Cubes3D(QWidget *parent) :
  QMainWindow(parent),
  ui(new Ui::Cubes3D)
{
  ui->setupUi(this);
}

Cubes3D::~Cubes3D()
{
  delete ui;
}
