#define CHECK_CONNECTION \
    if(!p->interface) return; \
    if(!connected()) return;

#include "artclient.h"

#include "asemansocketinterface.h"

#include <QHostAddress>
#include <QPointer>
#include <QTcpSocket>

class ARTClient::Private
{
public:
    QTcpSocket *socket;
    static QPointer<ARTClient> instance;
    QPointer<AsemanSocketInterface> interface;
    qint16 port;
    QString host;
};

QPointer<ARTClient> ARTClient::Private::instance;

ARTClient::ARTClient(const QString &host, qint16 port, QObject *parent) :
    QObject(parent)
{
    p = new Private;
    p->host = host;
    p->port = port;
    p->socket = Q_NULLPTR;
    p->interface = Q_NULLPTR;

    initSocket();
}

bool ARTClient::connected() const
{
    return p->interface;
}

void ARTClient::sendRequest(const QString &method, const QVariantList &args)
{
    if (!p->interface)
        return;

    p->interface->call(method, args);
}

void ARTClient::registerTest(const QString &testId, const QString &session)
{
    sendRequest(QStringLiteral("registerTest"), {testId, session});
}

void ARTClient::statusChanged(const QString &session, Status status)
{
    sendRequest(QStringLiteral("statusChanged"), {session, static_cast<int>(status)});
}

void ARTClient::submitLog(const QString &session, const QString &log, bool error)
{
    sendRequest(QStringLiteral("submitLog"), {session, log, error});
}

void ARTClient::finalize(const QString &session, quint64 duration, const QString &error)
{
    sendRequest(QStringLiteral("finalize"), {session, duration, error});
}

void ARTClient::initSocket()
{
    if(p->socket)
        p->socket->deleteLater();

    p->socket = new QTcpSocket(this);
    p->socket->connectToHost(p->host, p->port);

    connect(p->socket, &QTcpSocket::errorOccurred, this, [](QAbstractSocket::SocketError socketError){
        QVariant v = socketError;
        v.convert(QVariant::String);
        qDebug() << "ARTSocket: Socket connected" << v.toString() << socketError;
    });
    connect(p->socket, &QTcpSocket::connected, this, [this](){
        qDebug() << "ARTSocket: Socket connected" << p->socket;

        AsemanSocketInterface *interface = new AsemanSocketInterface(this, p->socket, p->socket);
        connect(interface, &AsemanSocketInterface::destroyed, this, [this, interface](){
            if(p->interface != interface)
                return;

            p->interface = Q_NULLPTR;
            Q_EMIT connectedChanged();
        });

        p->interface = interface;
        Q_EMIT connectedChanged();
    });
    connect(p->socket, &QTcpSocket::disconnected, this, [this](){
        qDebug() << "ARTSocket: Socket disconnected" << p->socket;
        p->socket->deleteLater();
    });
    qDebug() << "ARTSocket: Socket initialize" << p->socket;
}

ARTClient::~ARTClient()
{
    delete p;
}
