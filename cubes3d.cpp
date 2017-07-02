#include "cubes3d.h"
#include "ui_cubes3d.h"

#include <QSettings>

Cubes3D::Cubes3D(QWidget *parent) : QMainWindow(parent), ui(new Ui::Cubes3D)
{
  ui->setupUi(this);

  restoreGeometry(QSettings().value("Main/geomtry").toByteArray());
}

Cubes3D::~Cubes3D()
{
  delete ui;
}

void Cubes3D::closeEvent(QCloseEvent *e)
{
  QSettings().setValue("Main/geomtry", saveGeometry());

  QMainWindow::closeEvent(e);
}
