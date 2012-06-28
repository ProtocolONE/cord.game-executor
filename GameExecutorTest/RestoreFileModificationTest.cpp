#include <gtest/gtest.h>

#include <GameExecutor/Hook/RestoreFileModification.h>
#include <Core/Service>

#include <QtCore/QCoreApplication>
#include <QtCore/QDir>
#include <QtCore/QTextStream>
#include <Windows.h>

HANDLE openFile(const QString& path) {
  wchar_t tmp[MAX_PATH] = {0};
  path.toWCharArray(tmp);
  return ::CreateFileW(
    tmp, 
    GENERIC_READ | GENERIC_WRITE, 
    FILE_SHARE_DELETE | FILE_SHARE_READ | FILE_SHARE_WRITE,
    NULL,
    OPEN_EXISTING,
    FILE_ATTRIBUTE_NORMAL,
    NULL); 
}

TEST(RestoreFileModification, simpleTest)
{
  QString root = QCoreApplication::applicationDirPath();
  QString dirPath = QString("%1/RestoreFileModification").arg(root);
  QString filePath = QString("%1/1.txt").arg(dirPath);
  QDir dir(dirPath);
  dir.mkpath(dirPath);
  QFile file(filePath);
  ASSERT_TRUE(file.open(QIODevice::ReadWrite));
  QTextStream out(&file);
  out << "The magic number is: " << 49 << "\n";
  file.close();

  HANDLE handle = openFile(filePath);
  ASSERT_NE(INVALID_HANDLE_VALUE, handle);
  FILETIME time1;
  ASSERT_TRUE(GetFileTime(handle, 0, 0, &time1));
  CloseHandle(handle);

  GGS::GameExecutor::Hook::RestoreFileModification hook;
  GGS::Core::Service service;
  service.setInstallPath(dirPath);
  hook.PreExecute(service);

  handle = openFile(filePath);
  FILETIME timeFake;
  timeFake.dwLowDateTime = 1;
  timeFake.dwHighDateTime = 1;
  SetFileTime(handle, 0, 0, &timeFake);
  CloseHandle(handle);

  hook.PostExecute(service, GGS::GameExecutor::Success);

  handle = openFile(filePath);
  ASSERT_NE(INVALID_HANDLE_VALUE, handle);
  FILETIME time2;
  ASSERT_TRUE(GetFileTime(handle, 0, 0, &time2));
  CloseHandle(handle);

  ASSERT_EQ(time1.dwLowDateTime, time2.dwLowDateTime);
  ASSERT_EQ(time1.dwHighDateTime, time2.dwHighDateTime);
}