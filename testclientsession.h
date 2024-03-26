#ifndef TESTCLIENTSESSION_H
#define TESTCLIENTSESSION_H

#include "testclient.h"
#include "aseman/artclient.h"

#include <QObject>
#include <QSharedPointer>
#include <QTcpSocket>
#include <QFile>
#include <QTimer>
#include <QProcess>
#include <QPointer>

class TestClientSession : public QObject
{
    Q_OBJECT
public:
    explicit TestClientSession(const TestClient::TestObject &test, const QString &sessionId, TestClient *client);
    virtual ~TestClientSession();

public Q_SLOTS:
    void updateFileRequest(const QString &path, qint32 port, const QString &token);
    void runTest();

Q_SIGNALS:
    void statusChanged(ARTClient::Status status);
    void submitLog(const QString &log, bool error);
    void finalize(quint64 duration, const QString &error);

protected:
    void doRunTest();
    void terminate();

private:
    TestClient *mClient;
    TestClient::TestObject mTest;
    QString mSessionId;
    QHash<QTcpSocket*, QFile*> mSocketFiles;
    QTimer *mRunTimer;

    QPointer<QProcess> mProcess;
    QDateTime mStartedTime;
};

typedef QSharedPointer<TestClientSession> TestClientSessionPtr;

#endif // TESTCLIENTSESSION_H
