#include <QApplication>
#include <QDebug>
#include <QWidget>
#include "oculuswidget.hpp"
using namespace OVR;

int main(int argc, char *argv[])
{
  QApplication a(argc, argv);
  OculusWidget window;

  window.show();

  return a.exec();
}
