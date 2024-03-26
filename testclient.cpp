#include "testclient.h"
#include "testclientsession.h"

#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QDir>

#define FIND_SESSION(ID, SESSION) \
    if (session != sessionId) \
        return; \
    if (!mSessions.contains(ID)) \
        return; \
    auto &sessions = mSessions[ID]; \
    if (!sessions.contains(SESSION)) \
        return; \
    auto ptr = sessions[SESSION];

TestClient::TestClient(const QJsonObject &configs, QObject *parent)
    : AbstractTest(configs, parent)
{

    initializeTests(configs);
}

TestClient::~TestClient()
{
}

void TestClient::start()
{
    if (mSocket)
        return;

    const auto configs = AbstractTest::configs();
    const auto port = configs.value(QStringLiteral("port")).toInt();
    const auto host = configs.value(QStringLiteral("host")).toString(QStringLiteral("127.0.0.1"));

    mSocket = new ARTClient(host, port, this);
    mHost = host;

    connect(mSocket, &ARTClient::connectedChanged, this, [this](){
        if (!mSocket->connected())
            return;

        auto test_itr = mSessions.begin();
        while (test_itr != mSessions.end())
        {
            const auto sessions = test_itr.value();
            auto session_itr = sessions.begin();
            while (session_itr != sessions.end())
            {
                mSocket->registerTest(test_itr.key(), session_itr.key());
                session_itr++;
            }
            test_itr++;
        }
    });

    mSessions.clear();
    for (const auto &t: mTests)
    {
        auto sessionId = QUuid::createUuid().toString(QUuid::WithoutBraces);
        auto ptr = TestClientSessionPtr::create(t, sessionId, this);
        mSessions[t.key][sessionId] = ptr;

        connect(mSocket, &ARTClient::updateFileRequest, ptr.get(), [this, t, sessionId](const QString &session, const QString &path, qint32 port, const QString &token){
            FIND_SESSION(t.key, session);
            ptr->updateFileRequest(path, port, token);
        });
        connect(ptr.get(), &TestClientSession::statusChanged, this, [this, t, sessionId](ARTClient::Status status){
            mSocket->statusChanged(sessionId, status);
        });
        connect(ptr.get(), &TestClientSession::submitLog, this, [this, t, sessionId](const QString &log, bool error){
            mSocket->submitLog(sessionId, log, error);
        });
        connect(ptr.get(), &TestClientSession::finalize, this, [this, t, sessionId](quint64 duration, const QString &error){
            mSocket->finalize(sessionId, duration, error);
        });

        if (mSocket->connected())
            mSocket->registerTest(t.key, sessionId);
    }
}

void TestClient::stop()
{
    mSessions.clear();
    delete mSocket;
    mHost.clear();
}

void TestClient::initializeTests(const QJsonObject &configs)
{
    mTests.clear();
    const auto testsJson = configs.value(QStringLiteral("tests")).toArray();
    for (const auto &item: testsJson)
    {
        if (!item.isObject())
            continue;

        const auto m = item.toObject();
        TestObject t;
        t.key = m.value(QStringLiteral("key")).toString();
        t.command = m.value(QStringLiteral("command")).toString();
        t.rootPath = m.value(QStringLiteral("root")).toString();
        t.beginKey = m.value(QStringLiteral("begin-key")).toString();
        t.endKey = m.value(QStringLiteral("end-key")).toString();
        t.startAtBegin = m.value(QStringLiteral("start-at-begin")).toBool();

        QDir().mkpath(t.rootPath);

        for (const auto &s: m.value(QStringLiteral("arguments")).toArray())
            t.arguments << s.toString();

        mTests[t.key] = t;
    }
}

QString TestClient::host() const
{
    return mHost;
}

QHash<QString, TestClient::TestObject> TestClient::tests() const
{
    return mTests;
}
