#include "artserver.h"

#include "asemansocketinterface.h"

#include <QTcpServer>
#include <QTcpSocket>
#include <QPair>
#include <QTimer>
#include <QDebug>
#include <QUuid>

class ARTServer::Private
{
public:
    struct InterfaceUnit
    {
        AsemanSocketInterface *interface;
        QList<QPair<QString, QVariantList>> queue;
    };

    QTcpServer *server;
    QHash<QString, InterfaceUnit> interfaces;
    qint16 port;
    QString host;
};

ARTServer::ARTServer(const QString &host, qint16 port, QObject *parent) :
    QObject(parent)
{
    p = new Private;
    p->port = port;
    p->host = host;
    p->server = Q_NULLPTR;
    qDebug() << "ARTServer: constructed" << port;

    initServer();
}

bool ARTServer::connected(const QString &id) const
{
    return p->interfaces.contains(id);
}

void ARTServer::sendRequest(const QString &id, const QString &method, const QVariantList &args)
{
    if (!p->interfaces.contains(id))
        return;

    p->interfaces[id].interface->call(method, args);
}

qint16 ARTServer::port() const
{
    return p->port;
}

void ARTServer::updateFileRequest(const QString &id, const QString &session, const QString &path, qint32 port, const QString &token)
{
    sendRequest(id, QStringLiteral("updateFileRequest"), {session, path, port, token});
}

void ARTServer::pushCache(const QString &id)
{
    if (!p->interfaces.contains(id))
        return;

    auto &unit = p->interfaces[id];
    for (auto pair: unit.queue)
        unit.interface->call(pair.first, pair.second);

    unit.queue.clear();
}

void ARTServer::initServer()
{
    if(p->server)
        p->server->deleteLater();

    p->server = new QTcpServer(this);
    bool listened = p->server->listen(QHostAddress(p->host), p->port);
    if (p->port == 0)
        p->port = p->server->serverPort();

    qDebug() << "ARTServer: Server listening" << p->server << p->port << listened << p->server->errorString();

    connect(p->server, &QTcpServer::acceptError, this, [this](QAbstractSocket::SocketError socketError){
        QVariant v = socketError;
        v.convert(QVariant::String);
        qDebug() << "ARTServer: socket accept error" << p->server << p->port << v.toString() << socketError;
    });
    connect(p->server, &QTcpServer::newConnection, this, [this](){
        const auto uuid = QUuid::createUuid().toString(QUuid::WithoutBraces);
        QTcpSocket *socket = p->server->nextPendingConnection();
        AsemanSocketInterface *interface = new AsemanSocketInterface(this, socket, socket);
        interface->setExtraArguments({uuid});

        qDebug() << "ARTServer: Socket connected" << socket;
        connect(socket, &QTcpSocket::disconnected, socket, [socket](){
            qDebug() << "ARTServer: Socket disconnected" << socket;
            socket->deleteLater();
        });
        connect(interface, &AsemanSocketInterface::destroyed, this, [this, uuid](){
            p->interfaces.remove(uuid);
            Q_EMIT connectedChanged(uuid, false);
        });

        Private::InterfaceUnit unit;
        unit.interface = interface;

        p->interfaces[uuid] = unit;
        Q_EMIT connectedChanged(uuid, true);

        QTimer::singleShot(1000, this, [this, uuid](){ pushCache(uuid); });
    });
}

ARTServer::~ARTServer()
{
    delete p;
}
