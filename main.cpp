#include <QApplication>
#include <QSettings>
#include "cubes3d.h"

int main(int argc, char *argv[])
{
  QApplication a(argc, argv);
  a.setOrganizationName("pjame");
  a.setApplicationName("Cubes3D");
  a.setApplicationVersion("0.1.0");

  QSettings::setDefaultFormat(QSettings::IniFormat);

  Cubes3D w;
  w.show();

  return a.exec();
}
