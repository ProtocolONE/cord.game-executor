/****************************************************************************
** This file is a part of Syncopate Limited GameNet Application or it parts.
**
** Copyright (©) 2011 - 2012, Syncopate Limited and/or affiliates. 
** All rights reserved.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
****************************************************************************/

#include <GameExecutor/GameExecutorService.h>
#include <GameExecutor/Extension.h>
#include <GameExecutor/Executor/WebLink.h>

#include <RestApi/RestApiManager>
#include <RestApi/GameNetCredential>

#include <QtCore/QUrl>
#include <QtCore/QUrlQuery>
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

      void WebLink::execute(
        const GGS::Core::Service &service, 
        GameExecutorService *executorService,
        const GGS::RestApi::GameNetCredential& credential,
        const GGS::RestApi::GameNetCredential& secondCredential)
      {
        DEBUG_LOG;

        emit this->started(service);

        QUrl finalUrl;
        QString cookie;
        
        if (secondCredential.cookie().isEmpty())
          cookie = credential.cookie();
        else
          cookie = secondCredential.cookie();

        if (!cookie.isEmpty()) {
          finalUrl.setUrl("https://gnlogin.ru");
          QUrlQuery urlQuery(finalUrl);
          urlQuery.addQueryItem("auth", cookie);
          urlQuery.addQueryItem("rp", QUrl::toPercentEncoding(service.url().toString()));
          finalUrl.setQuery(urlQuery);
        } else {
          finalUrl = service.url();
        }
        
        OpenBrowserExtension *extension = reinterpret_cast<OpenBrowserExtension *>(
          executorService->extension(ExtensionTypes::OpenBrowser));

        if (extension) {
          extension->get()(finalUrl.toString());
          emit this->finished(service, GGS::GameExecutor::Success);
          return;
        }

        HINSTANCE result = 
          ShellExecute(0, 0, reinterpret_cast<const WCHAR*>(finalUrl.toString().utf16()), 0, 0, SW_NORMAL);
           
        //32 не магическое число - http://msdn.microsoft.com/en-us/library/windows/desktop/bb762153(v=vs.85).aspx
        if (reinterpret_cast<int>(result) > 32) {
          emit this->finished(service, GGS::GameExecutor::Success);
        } else {
          DEBUG_LOG << "Failed to execute " << finalUrl << "Error " << (DWORD)result;
          emit this->finished(service, GGS::GameExecutor::ExternalFatalError);
        }
      }
    }
  }
}