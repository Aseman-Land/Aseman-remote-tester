#ifndef TESTCLIENT_H
#define TESTCLIENT_H

#include "abstracttest.h"
#include "aseman/artclient.h"

class TestClientSession;
typedef QSharedPointer<TestClientSession> TestClientSessionPtr;

class TestClient : public AbstractTest
{
    Q_OBJECT
public:
    struct TestObject
    {
        QString key;
        QString rootPath;
        QString command;
        QStringList arguments;
        QString beginKey;
        QString endKey;
        bool startAtBegin = false;
    };

    explicit TestClient(const QJsonObject &configs, QObject *parent = nullptr);
    virtual ~TestClient();

    virtual void start() Q_DECL_OVERRIDE;
    virtual void stop() Q_DECL_OVERRIDE;

    QHash<QString, TestObject> tests() const;

    QString host() const;

protected:
    void initializeTests(const QJsonObject &configs);

private:
    QString mHost;
    ARTClient *mSocket = nullptr;
    QHash<QString, TestObject> mTests;
    QHash<QString, QHash<QString, TestClientSessionPtr>> mSessions;
};

#endif // TESTCLIENT_H
