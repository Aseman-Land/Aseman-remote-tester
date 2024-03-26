#ifndef TESTSERVER_H
#define TESTSERVER_H

#include "abstracttest.h"
#include "aseman/artserver.h"

class TestServerSession;
typedef QSharedPointer<TestServerSession> TestServerSessionPtr;

class TestServer : public AbstractTest
{
    Q_OBJECT
public:
    struct TestObject
    {
        QString key;
        QStringList watchList;
        QString command;
        QString rootPath;
        QStringList arguments;
        QString beginKey;
        QString endKey;
        bool startAtBegin = false;
        bool startServerBeforeRemote = false;
    };

    explicit TestServer(const QJsonObject &configs, QObject *parent = nullptr);
    virtual ~TestServer();

    virtual void start() Q_DECL_OVERRIDE;
    virtual void stop() Q_DECL_OVERRIDE;

    QHash<QString, TestObject> tests() const;

private Q_SLOTS:
    void connectedChanged(const QString &id, bool connected);

    void registerTest(const QString &id, const QString &testId, const QString &session);
    void statusChanged(const QString &id, const QString &session, int status);
    void submitLog(const QString &id, const QString &session, const QString &log, bool error);
    void finalize(const QString &id, const QString &session, quint64 duration, const QString &error);
    void updateFileRequest(const QString &id, const QString &session, const QString &path, qint32 port, const QString &token);

protected:
    void initializeTests(const QJsonObject &configs);

private:
    ARTServer *mServer = nullptr;
    QHash<QString, QHash<QString, TestServerSessionPtr>> mSessions;
    QHash<QString, TestObject> mTests;
};

#endif // TESTSERVER_H
