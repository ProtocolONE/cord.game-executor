#include <Launcher.h>
#include <QtCore/QCoreApplication>
#include <RestApi/RequestFactory>

Launcher::Launcher(QObject *parent) : QObject(parent)
{
  connect(&this->client, SIGNAL(exit(int)), 
          this, SLOT(exit(int)));
  
  this->restapi.setCache(&this->cache);
  this->restapi.setRequest(GGS::RestApi::RequestFactory::Http);

  this->client.setRestApiManager(&this->restapi);
}

Launcher::~Launcher()
{
}

void Launcher::exec()
{
  client.exec();
}

void Launcher::exit(int code)
{
  qDebug() << "Catch exit code " << code;
  QCoreApplication::exit(code);
}
