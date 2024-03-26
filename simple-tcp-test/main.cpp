#include <QCoreApplication>
#include <QTcpSocket>
#include <QTcpServer>
#include <QFile>
#include <QDateTime>
#include <QtMath>

#define LISTEN_PORT 6753

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);

    auto startTime = QDateTime::currentDateTime();
    qint64 readedSize = 0;

    constexpr auto size_mb = 128;
    constexpr auto payload = 1024*1024;

    const auto args = app.arguments();
    if (args.contains("--server"))
    {
        auto server = new QTcpServer(&app);
        server->listen(QHostAddress::Any, LISTEN_PORT);
        server->connect(server, &QTcpServer::newConnection, [server](){
            auto s = server->nextPendingConnection();
            s->connect(s, &QTcpSocket::disconnected, qApp, &QCoreApplication::quit);

            qDebug() << "New client connected:" << s->peerAddress() << s->peerPort();

            QFile f("/dev/zero");
            f.open(QFile::ReadOnly);

            qint64 totalWrittern = 0;
            for (int i=0; i<size_mb; i++)
                totalWrittern += s->write(f.read(payload));
            s->flush();
            s->disconnectFromHost();
            qDebug() << totalWrittern << "Written";
        });
    }
    else if (args.length() == 2)
    {
        auto socket = new QTcpSocket(&app);
        socket->connect(socket, &QTcpSocket::connected, [socket, &startTime](){
            startTime = QDateTime::currentDateTime();
            qDebug() << "Connected to server:" << socket->peerAddress() << socket->peerPort();
        });
        socket->connect(socket, &QTcpSocket::disconnected, [socket, startTime, &readedSize](){
            const auto data = socket->readAll();
            readedSize += data.size();

            const auto ms = startTime.msecsTo(QDateTime::currentDateTime());
            qDebug() << "Finished:" << socket->peerAddress() << socket->peerPort();
            qDebug() << "Duration:" << ms << "ms";
            qDebug() << "Size:" << readedSize << "bytes";
            qDebug() << "speed:" << (ms? qFloor(10*(readedSize/payload)/(ms/1000))/10 : ms) << "mb/s";
            qApp->exit();
        });
        socket->connect(socket, &QTcpSocket::readyRead, [&readedSize, socket](){
            const auto data = socket->readAll();
            readedSize += data.size();
        });
        socket->connectToHost(args.at(1), LISTEN_PORT);
    }
    else
        return 0;

    return app.exec();
}
