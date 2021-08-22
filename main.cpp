#include "cubes3d.h"

#include <QApplication>
#include <QSettings>

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
