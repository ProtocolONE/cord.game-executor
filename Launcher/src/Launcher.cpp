#include <Launcher.h>

#include <RestApi/RequestFactory>

#include <QtCore/QCoreApplication>
#include <QtCore/QStringList>

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

void Launcher::exec(QCoreApplication &app)
{
  QStringList args = app.arguments();
  if (args.size() != 2) {
    qDebug() << "Wrong command line arguments" << args;
    QCoreApplication::exit(-1);
    return;
  }
  
  client.setIpcName(args[1].trimmed());
  client.exec();
}

void Launcher::exit(int code)
{
  qDebug() << "Catch exit code " << code;
  QCoreApplication::exit(code);
}
