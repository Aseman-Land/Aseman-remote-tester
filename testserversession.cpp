#include "testserversession.h"

#include <QCoreApplication>
#include <QFile>
#include <QFileInfo>
#include <QTcpServer>
#include <QTcpSocket>
#include <QUuid>

TestServerSession::TestServerSession(const QString &sessionId, TestServer *server)
    : QObject()
    , mServer(server)
    , mSessionId(sessionId)
{
    mRunTimer = new QTimer(this);
    mRunTimer->setInterval(500);
    mRunTimer->setSingleShot(true);

    connect(mRunTimer, &QTimer::timeout, this, &TestServerSession::doRunTest);
}

TestServerSession::~TestServerSession()
{
}

void TestServerSession::registerTest(const QString &testId)
{
    const auto &tests = mServer->tests();
    if (!tests.contains(testId))
        return;

    mTest = tests.value(testId);
    mWatcher = new FileWatcher(mTest.watchList, this);

    connect(mWatcher, &FileWatcher::changed, this, &TestServerSession::handleFileUpdate);
}

void TestServerSession::statusChanged(int status)
{

}

void TestServerSession::submitLog(const QString &log, bool error)
{
    if (error)
        qDebug() << mTest.key << log;
    else
        qInfo() << mTest.key << log;
}

void TestServerSession::finalize(quint64 duration, const QString &error)
{
    if (error.length())
        qDebug() << "ERROR:" << error;
    qInfo() << "FINISHED" << duration << "ms";
}

QString TestServerSession::sessionId() const
{
    return mSessionId;
}

void TestServerSession::runTest()
{
    mRunTimer->stop();
    mRunTimer->start();
}

void TestServerSession::handleFileUpdate(const QString &path)
{
    qDebug() << "File updated" << path;
    initFileServer();

    if (mFileTokens.contains(path))
    {
        auto token = mFileTokens.take(path);
        mTokenFiles.remove(token);
    }

    const auto token = QUuid::createUuid().toString(QUuid::WithoutBraces);
    mFileTokens[path] = token;
    mTokenFiles[token] = path;

    Q_EMIT updateFileRequest(path, mFileServer->serverPort(), token);

    runTest();
}

void TestServerSession::initFileServer()
{
    if (mFileServer)
        return;

    mFileServer = new QTcpServer(qApp);
    mFileServer->listen();

    connect(mFileServer, &QTcpServer::newConnection, this, [this](){
        auto s = mFileServer->nextPendingConnection();
        connect(s, &QTcpSocket::readyRead, this, [this, s](){
            const auto token = QString::fromUtf8(s->readAll());
            if (!mTokenFiles.contains(token))
            {
                s->disconnectFromHost();
                return;
            }

            const auto path = mTokenFiles.take(token);
            mFileTokens.remove(path);

            auto f = new QFile(path, s);
            if (!f->open(QFile::ReadOnly))
            {
                delete f;
                s->disconnectFromHost();
                return;
            }

            QFileInfo inf(path);

            s->write(inf.fileName().toUtf8() + '\n');

            constexpr auto size = 1024*1024;
            while (!f->atEnd())
                s->write( f->read(size) );

            s->flush();
            s->disconnectFromHost();
        });
        connect(s, &QTcpSocket::disconnected, s, &QTcpSocket::deleteLater);
    });
}

void TestServerSession::doRunTest()
{
    terminate();

    qDebug() << "Running test" << mTest.key << ":" << mTest.command.toStdString().c_str() << mTest.arguments.join(" ").toStdString().c_str();

    auto prc = new QProcess(this);
    prc->setArguments(mTest.arguments);
    prc->setProgram(mTest.command);
    prc->setWorkingDirectory(mTest.rootPath);

    connect(prc, &QProcess::readyReadStandardError, this, [this, prc](){
        qDebug() << prc->readAllStandardError();
    });
    connect(prc, &QProcess::readyReadStandardOutput, this, [this, prc](){
        qInfo() << prc->readAllStandardOutput();
    });
    connect(prc, &QProcess::stateChanged, this, [this, prc](QProcess::ProcessState status){
        if (prc != mProcess)
            return;
        switch (static_cast<int>(status))
        {
        case QProcess::ProcessState::NotRunning:
            qDebug() << "FINISHED SERVER" << mStartedTime.msecsTo(QDateTime::currentDateTime());
            break;
        case QProcess::ProcessState::Running:
            mStartedTime = QDateTime::currentDateTime();
            break;
        case QProcess::ProcessState::Starting:
            break;
        }
    });
    connect(prc, static_cast<void(QProcess::*)(int, QProcess::ExitStatus)>(&QProcess::finished), this, [this, prc](){
        prc->deleteLater();
    });

    mProcess = prc;

    mStartedTime = QDateTime::currentDateTime();
    prc->start();
}

void TestServerSession::terminate()
{
    if (!mProcess)
        return;

    mProcess->terminate();
    mProcess->waitForFinished(5000);
    mProcess = nullptr;
}
