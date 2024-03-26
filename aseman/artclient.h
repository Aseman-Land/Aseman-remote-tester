#ifndef ARTCLIENT_H
#define ARTCLIENT_H

#include <QObject>
#include <QUrl>
#include <QVariant>
#include <QPoint>
#include <QSharedPointer>

class ARTClient : public QObject
{
    Q_OBJECT
    Q_PROPERTY(bool connected READ connected NOTIFY connectedChanged)

    class Private;

public:
    enum class Status {
        None,
        Installing,
        Starting,
        Started,
        Closing,
        Finished,
    };

    ARTClient(const QString &host, qint16 port, QObject *parent = Q_NULLPTR);
    virtual ~ARTClient();

    bool connected() const;

    void sendRequest(const QString &method, const QVariantList &args = QVariantList());

public Q_SLOTS:
    void registerTest(const QString &testId, const QString &session);
    void statusChanged(const QString &session, ARTClient::Status status);
    void submitLog(const QString &session, const QString &log, bool error);
    void finalize(const QString &session, quint64 duration, const QString &error);

Q_SIGNALS:
    void updateFileRequest(const QString &session, const QString &path, qint32 port, const QString &token);
    void connectedChanged();

protected:
    void initSocket();

private:
    Private *p;
};

typedef QSharedPointer<ARTClient> ARTClientPtr;

#endif // ARTCLIENT_H
