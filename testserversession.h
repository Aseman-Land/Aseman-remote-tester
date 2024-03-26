#ifndef TESTSERVERSESSION_H
#define TESTSERVERSESSION_H

#include "testserver.h"
#include "filewatcher.h"

#include <QObject>
#include <QPointer>
#include <QProcess>
#include <QSharedPointer>
#include <QTcpServer>
#include <QTimer>

class TestServerSession : public QObject
{
    Q_OBJECT
public:
    explicit TestServerSession(const QString &sessionId, TestServer *server);
    virtual ~TestServerSession();

    void registerTest(const QString &testId);
    void statusChanged(int status);
    void submitLog(const QString &log, bool error);
    void finalize(quint64 duration, const QString &error);

    QString sessionId() const;

public Q_SLOTS:
    void runTest();

Q_SIGNALS:
    void updateFileRequest(const QString &path, qint32 port, const QString &token);

protected:
    void handleFileUpdate(const QString &changed);
    void initFileServer();

    void doRunTest();
    void terminate();

private:
    TestServer *mServer;
    QString mSessionId;

    TestServer::TestObject mTest;
    FileWatcher *mWatcher = nullptr;

    QTcpServer *mFileServer = nullptr;

    QHash<QString, QString> mTokenFiles;
    QHash<QString, QString> mFileTokens;

    QTimer *mRunTimer;

    QPointer<QProcess> mProcess;
    QDateTime mStartedTime;
};

typedef QSharedPointer<TestServerSession> TestServerSessionPtr;

#endif // TESTSERVERSESSION_H
