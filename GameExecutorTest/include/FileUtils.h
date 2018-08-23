#pragma once
#include <QtCore/QString>
#include <QtCore/QDir>
#include <QtCore/QFile>
#include <QtCore/QFileInfo>
#include <QtCore/QFileInfoList>

class FileUtils
{
public:
  static bool removeDir(const QString &dirName);
};
