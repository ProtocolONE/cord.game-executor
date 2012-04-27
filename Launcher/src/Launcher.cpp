#include <Launcher.h>
#include <QtCore/QCoreApplication>

Launcher::Launcher(QObject *parent) : QObject(parent)
{
  connect(&this->client, SIGNAL(exit(int)), 
          this, SLOT(exit(int)));
  
  this->request.setCache(&this->cache);
  this->restapi.setRequest(&request);

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
