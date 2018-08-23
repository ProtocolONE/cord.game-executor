#include <gtest/gtest.h>

#include <QtWidgets/QApplication>
#include <QtCore/QCoreApplication>

#include <QMetaType>

int main(int argc, char *argv[])
{
  QApplication app(argc, argv);

  QStringList plugins;
  QString path = QCoreApplication::applicationDirPath();

  plugins << path + "/plugins";
  app.setLibraryPaths(plugins);

  testing::InitGoogleTest(&argc, argv);
  int r = RUN_ALL_TESTS();
  
  return r;
}