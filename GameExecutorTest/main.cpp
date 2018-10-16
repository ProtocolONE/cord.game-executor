#include <gtest/gtest.h>

#include <QtWidgets/QApplication>
#include <QtCore/QCoreApplication>

#include <Core/RegisterTypes.h>
#include <GameExecutor/RegisterTypes.h>
#include <RestApi/RegisterTypes.h>


#include <QMetaType>

int main(int argc, char *argv[])
{
  QApplication app(argc, argv);

  P1::Core::registerTypes();
  P1::GameExecutor::registerTypes();
  P1::RestApi::registerTypes();



  QStringList plugins;
  QString path = QCoreApplication::applicationDirPath();

  plugins << path + "/plugins";
  app.setLibraryPaths(plugins);

  testing::InitGoogleTest(&argc, argv);
  int r = RUN_ALL_TESTS();
  
  return r;
}