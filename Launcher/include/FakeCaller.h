#include <QtCore/QObject> // FAKE
#include <QtCore/QTimer> // FAKE

class FakeCaller : public QObject {
  Q_OBJECT
public:

  FakeCaller(const QString& message, QObject * parent = 0)
  : QObject(parent)
  , _message(message)
  {
  }

  ~FakeCaller() {}

public slots:
  void exec();

signals:
  void caller(QString arg);

private:
  QString _message;
};