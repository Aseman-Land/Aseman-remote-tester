#include "testserver.h"
#include "testserversession.h"

#include <QJsonArray>
#include <QJsonObject>

#define CHECK_AND_GET_SESSION(ID, SESSION) \
    if (!mSessions.contains(ID)) \
        return; \
    auto &client = mSessions[ID]; \
    if (!mSessions[ID].contains(SESSION)) \
        return; \
    auto ptr = client[SESSION];

TestServer::TestServer(const QJsonObject &configs, QObject *parent)
    : AbstractTest(configs, parent)
{
    initializeTests(configs);
}

TestServer::~TestServer()
{
}

void TestServer::start()
{
    if (mServer)
        return;

    const auto configs = AbstractTest::configs();
    const auto port = configs.value(QStringLiteral("port")).toInt();
    const auto host = configs.value(QStringLiteral("host")).toString(QStringLiteral("127.0.0.1"));

    mServer = new ARTServer(host, port, this);

    connect(mServer, &ARTServer::connectedChanged, this, &TestServer::connectedChanged);
    connect(mServer, &ARTServer::registerTest, this, &TestServer::registerTest);
    connect(mServer, &ARTServer::statusChanged, this, &TestServer::statusChanged);
    connect(mServer, &ARTServer::submitLog, this, &TestServer::submitLog);
    connect(mServer, &ARTServer::finalize, this, &TestServer::finalize);
}

void TestServer::stop()
{
    if (!mServer)
        return;

    mSessions.clear();
    mServer->deleteLater();
    mServer = nullptr;
}

void TestServer::connectedChanged(const QString &id, bool connected)
{
    if (connected)
        mSessions[id] = QHash<QString, TestServerSessionPtr>();
    else
        mSessions.remove(id);
}

void TestServer::registerTest(const QString &id, const QString &testId, const QString &session)
{
    qDebug() << "Register request"<< id << testId << session;
    if (!mSessions.contains(id))
        return;

    auto &client = mSessions[id];
    if (client.contains(session))
        return;

    auto ptr = TestServerSessionPtr::create(session, this);
    client[session] = ptr;

    ptr->registerTest(testId);

    connect(ptr.get(), &TestServerSession::updateFileRequest, this, [this, id, session](const QString &path, qint32 port, const QString &token){
        mServer->updateFileRequest(id, session, path, port, token);
    });
}

void TestServer::statusChanged(const QString &id, const QString &session, int status)
{
    CHECK_AND_GET_SESSION(id, session);
    ptr->statusChanged(status);
}

void TestServer::submitLog(const QString &id, const QString &session, const QString &log, bool error)
{
    CHECK_AND_GET_SESSION(id, session);
    ptr->submitLog(log, error);
}

void TestServer::finalize(const QString &id, const QString &session, quint64 duration, const QString &error)
{
    CHECK_AND_GET_SESSION(id, session);
    ptr->finalize(duration, error);
}

void TestServer::updateFileRequest(const QString &id, const QString &session, const QString &path, qint32 port, const QString &token)
{
    mServer->updateFileRequest(id, session, path, port, token);
}

void TestServer::initializeTests(const QJsonObject &configs)
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
        t.startServerBeforeRemote = m.value(QStringLiteral("start-server-before-remote")).toBool();

        for (const auto &s: m.value(QStringLiteral("watches")).toArray())
            t.watchList << s.toString();
        for (const auto &s: m.value(QStringLiteral("arguments")).toArray())
            t.arguments << s.toString();

        mTests[t.key] = t;
    }
}

QHash<QString, TestServer::TestObject> TestServer::tests() const
{
    return mTests;
}
