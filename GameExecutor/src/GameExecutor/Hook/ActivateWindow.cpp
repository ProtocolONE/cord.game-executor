#include <GameExecutor/Hook/ActivateWindow.h>

#include <Application/WindowHelper.h>

namespace GGS {
  namespace GameExecutor {
    namespace Hook {

      ActivateWindow::ActivateWindow(QObject *parent /*= 0*/) :
        _window(nullptr)
      {
      }

      ActivateWindow::~ActivateWindow()
      {
      }

      void ActivateWindow::PreExecute(Core::Service &service)
      {
        if (!this->_window || QSysInfo::windowsVersion() >= QSysInfo::WV_WINDOWS8)
          return;

        this->_window->show();
        GGS::Application::WindowHelper::activate(reinterpret_cast<HWND>(this->_window->winId()));

        this->preExecuteCompleted(service, GGS::GameExecutor::FinishState::Success);
      }

      QString ActivateWindow::id()
      {
        return "11C155F1-D80C-4576-89B6-D2D3C138A405";
      }

      void ActivateWindow::setWidget(QWidget* widget)
      {
        this->_window = widget;
      }

    }
  }
}