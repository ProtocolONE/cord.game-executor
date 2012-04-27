#include <GameExecutor/GameExecutorService.h>
#include <GameExecutor/Executor/WebLink.h>

#include <RestApi/RestApiManager>
#include <RestApi/GameNetCredential>

#include <QtCore/QUrl>
#include <QtCore/QDebug>

#include <qt_windows.h>
#include <shellapi.h>

namespace GGS {
  namespace GameExecutor {
    namespace Executor {
      WebLink::WebLink(QObject *parent) : ExecutorBase(parent)
      {
        this->_scheme = QString("http");
      }

      WebLink::~WebLink()
      {
      }

      void WebLink::execute(const Core::Service &service, GameExecutor::GameExecutorService *executorService)
      {
        qDebug() << __LINE__ << __FUNCTION__;

        emit this->started(service);
        
        RestApi::RestApiManager *mgr = executorService->respApiManager();
        RestApi::GameNetCredential cred = mgr->credential();

        QUrl finalUrl;
        QString cookie = cred.cookie();
        if (!cookie.isEmpty()) {
          finalUrl.setPath("https://gnlogin.ru");
          finalUrl.addQueryItem("auth", cookie);
          finalUrl.addQueryItem("rp", service.url().toString());
        } else {
          finalUrl = service.url();
        }
        
        HINSTANCE result = 
          ShellExecute(0, 0, reinterpret_cast<const WCHAR*>(finalUrl.toString().utf16()), 0, 0, SW_NORMAL);

        //32 не магическое число - http://msdn.microsoft.com/en-us/library/windows/desktop/bb762153(v=vs.85).aspx
        if (reinterpret_cast<int>(result) > 32)
          emit this->finished(service, GGS::GameExecutor::Success);
        else
          emit this->finished(service, GGS::GameExecutor::ExternalFatalError);
      }
    }
  }
}