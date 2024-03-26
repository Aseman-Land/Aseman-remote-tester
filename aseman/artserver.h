#ifndef ARTSERVER_H
#define ARTSERVER_H

#include <QObject>
#include <QVariant>
#include <QUrl>
#include <QSharedPointer>

class AsemanSocketInterface;
class ARTServer : public QObject
{
    Q_OBJECT
    class Private;

public:
    ARTServer(const QString &host, qint16 port, QObject *parent = Q_NULLPTR);
    virtual ~ARTServer();

    bool connected(const QString &id) const;
    void sendRequest(const QString &id, const QString &method, const QVariantList &args = QVariantList());

    qint16 port() const;

public Q_SLOTS:
    void updateFileRequest(const QString &id, const QString &session, const QString &path, qint32 port, const QString &token);

Q_SIGNALS:
    void connectedChanged(const QString &id, bool connected);

    void registerTest(const QString &id, const QString &testId, const QString &session);
    void statusChanged(const QString &id, const QString &session, int status);
    void submitLog(const QString &id, const QString &session, const QString &log, bool error);
    void finalize(const QString &id, const QString &session, quint64 duration, const QString &error);

protected:
    void pushCache(const QString &id);
    void initServer();

private:
    Private *p;
};

typedef QSharedPointer<ARTServer> ARTServerPtr;

#endif // ARTSERVER_H
