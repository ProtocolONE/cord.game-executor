#include <GameExecutor/GameExecutorService.h>
#include <GameExecutor/Executor/WebLink.h>

#include <RestApi/RestApiManager.h>
#include <RestApi/ProtocolOneCredential.h>

#include <QtCore/QUrl>
#include <QtCore/QUrlQuery>
#include <QtCore/QDebug>

#include <qt_windows.h>
#include <shellapi.h>

namespace P1 {
  namespace GameExecutor {
    namespace Executor {

      WebLink::WebLink(QObject *parent) 
        : ExecutorBase(parent)
      {
        this->_scheme = QString("http");
      }

      WebLink::~WebLink()
      {
      }

      void WebLink::execute(
        const P1::Core::Service &service, 
        GameExecutorService *executorService,
        const P1::RestApi::ProtocolOneCredential& credential)
      {
        DEBUG_LOG;

        emit this->started(service);

        // UNDONE refresh token, when add method getToken
        QUrl finalUrl = service.url();
        QUrlQuery urlQuery(finalUrl);
        urlQuery.addQueryItem("jwt", credential.acccessTokent());
        finalUrl.setQuery(urlQuery);

        HINSTANCE result = 
          ShellExecute(0, 0, reinterpret_cast<const WCHAR*>(finalUrl.toString().utf16()), 0, 0, SW_NORMAL);
           
        //32 не магическое число - http://msdn.microsoft.com/en-us/library/windows/desktop/bb762153(v=vs.85).aspx
        if (reinterpret_cast<int>(result) > 32) {
          emit this->finished(service, P1::GameExecutor::Success);
        } else {
          DEBUG_LOG << "Failed to execute " << finalUrl << "Error " << (DWORD)result;
          emit this->finished(service, P1::GameExecutor::ExternalFatalError);
        }
      }
    }
  }
}