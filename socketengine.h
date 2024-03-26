#ifndef SOCKETENGINE_H
#define SOCKETENGINE_H

#include <QObject>

class SocketEngine : public QObject
{
    Q_OBJECT
public:
    explicit SocketEngine(QObject *parent = nullptr);
    virtual ~SocketEngine();

Q_SIGNALS:
};

#endif // SOCKETENGINE_H
