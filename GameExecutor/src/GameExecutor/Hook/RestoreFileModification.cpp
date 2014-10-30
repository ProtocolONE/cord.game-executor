/****************************************************************************
** This file is a part of Syncopate Limited GameNet Application or it parts.
**
** Copyright (©) 2011 - 2012, Syncopate Limited and/or affiliates. 
** All rights reserved.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
****************************************************************************/

#include <GameExecutor/Hook/RestoreFileModification.h>

#include <QtCore/QString>
#include <QtCore/QDirIterator>
#include <QtCore/QDebug>

#include <Windows.h>

namespace GGS {
  namespace GameExecutor {
    namespace Hook {

      class CreateFileHelper {
      public:
        CreateFileHelper(const QString& filePath) : _handle(INVALID_HANDLE_VALUE) 
        {
            if (filePath.size() > MAX_PATH)
              return;

            wchar_t tmp[MAX_PATH + 1] = {0}; // При необходимости можно увеличить размер буффера, 
            // т.к. CreateFileW принимает строку до 32,767 символов
            filePath.toWCharArray(tmp);

            this->_handle = ::CreateFileW(
              tmp, 
              GENERIC_READ | GENERIC_WRITE, 
              FILE_SHARE_DELETE | FILE_SHARE_READ | FILE_SHARE_WRITE,
              NULL,
              OPEN_EXISTING,
              FILE_ATTRIBUTE_NORMAL,
              NULL);
        }

        ~CreateFileHelper() 
        {
          if (this->_handle != INVALID_HANDLE_VALUE)
            ::CloseHandle(this->_handle);
        }

        bool isValid() const 
        {
          return this->_handle != INVALID_HANDLE_VALUE;
        }

        HANDLE handle() const 
        {
          return this->_handle;
        }

      private:
        HANDLE _handle;
      };

      RestoreFileModification::RestoreFileModification(QObject *parent)
        : HookInterface(parent)
      {
      }

      RestoreFileModification::~RestoreFileModification()
      {
      }

      void RestoreFileModification::PreExecute(Core::Service &service)
      {
        this->_modifiedTime.clear();
        QString path = service.installPath();
        QDirIterator it(path, QDirIterator::Subdirectories);
        while (it.hasNext()) {
          QString filePath = it.next();
          if (!it.fileInfo().isDir()) {
            CreateFileHelper file(filePath);
            if (!file.isValid())
              continue;  

            FILETIME lastWriteTime;
            if (!::GetFileTime(file.handle(), 0, 0, &lastWriteTime))
              continue;

            this->_modifiedTime[filePath].first = lastWriteTime.dwLowDateTime;
            this->_modifiedTime[filePath].second = lastWriteTime.dwHighDateTime;
          }
        }

        emit this->preExecuteCompleted(service, GGS::GameExecutor::Success);
      }

      void RestoreFileModification::PostExecute(Core::Service &service, GGS::GameExecutor::FinishState state)
      {
        QHash<QString, QPair<quint32, quint32> >::const_iterator it = this->_modifiedTime.constBegin();
        QHash<QString, QPair<quint32, quint32> >::const_iterator end = this->_modifiedTime.constEnd();
        for (;it != end; ++it) {
          CreateFileHelper file(it.key());
          if (!file.isValid())
            continue;  

          FILETIME lastWriteTime;
          ::GetFileTime(file.handle(), 0, 0, &lastWriteTime);

          if (lastWriteTime.dwLowDateTime != it.value().first 
            || lastWriteTime.dwHighDateTime != it.value().second) {
              DEBUG_LOG << "Restore last write time for file: " << it.key();

              lastWriteTime.dwLowDateTime = it.value().first;
              lastWriteTime.dwHighDateTime = it.value().second;
              ::SetFileTime(file.handle(), 0, 0, &lastWriteTime);
          }
        }

        emit this->postExecuteCompleted(service);
      }

      QString RestoreFileModification::id()
      {
        return "798E4925-8D98-4191-ACF6-F844A327E0F7";
      }

    }
  }
}