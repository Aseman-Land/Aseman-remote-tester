#include "testclientsession.h"

TestClientSession::TestClientSession(const TestClient::TestObject &test, const QString &sessionId, TestClient *client)
    : QObject()
    , mClient(client)
    , mTest(test)
    , mSessionId(sessionId)
{
    mRunTimer = new QTimer(this);
    mRunTimer->setInterval(500);
    mRunTimer->setSingleShot(true);

    connect(mRunTimer, &QTimer::timeout, this, &TestClientSession::doRunTest);
}

TestClientSession::~TestClientSession()
{
}

void TestClientSession::updateFileRequest(const QString &path, qint32 port, const QString &token)
{
    terminate();
    Q_EMIT statusChanged(ARTClient::Status::Installing);

    auto s = new QTcpSocket(this);

    connect(s, &QTcpSocket::connected, this, [this, s, token](){
        s->write(token.toUtf8());
    });
    connect(s, &QTcpSocket::readyRead, this, [this, s](){
        QFile *f = nullptr;
        if (!mSocketFiles.contains(s))
        {
            auto fileName = QString::fromUtf8(s->readLine()).trimmed();
            f = new QFile(mTest.rootPath + '/' + fileName);
            f->open(QFile::WriteOnly);

            mSocketFiles[s] = f;
        }
        else
            f = mSocketFiles.value(s);

        f->write(s->readAll());
    });
    connect(s, &QTcpSocket::disconnected, this, [this, s](){
        if (mSocketFiles.contains(s))
        {
            auto f = mSocketFiles.take(s);
            f->setPermissions(QFile::WriteOwner | QFile::WriteGroup | QFile::ReadOwner | QFile::ReadGroup | QFile::ExeOwner | QFile::ExeGroup);
            f->close();
            f->deleteLater();
            runTest();
        }
        s->deleteLater();
    });

    s->connectToHost(mClient->host(), port);
    mRunTimer->stop();
}

void TestClientSession::runTest()
{
    mRunTimer->stop();
    mRunTimer->start();
}

void TestClientSession::doRunTest()
{
    terminate();
    Q_EMIT statusChanged(ARTClient::Status::Starting);

    qDebug() << "Running test" << mTest.key << ":" << mTest.command.toStdString().c_str() << mTest.arguments.join(" ").toStdString().c_str();

    auto prc = new QProcess(this);
    prc->setArguments(mTest.arguments);
    prc->setProgram(mTest.command);
    prc->setWorkingDirectory(mTest.rootPath);

    connect(prc, &QProcess::readyReadStandardError, this, [this, prc](){
        const auto data = prc->readAllStandardError();
        qDebug() << data;
        if (prc == mProcess) Q_EMIT submitLog(data, true);
    });
    connect(prc, &QProcess::readyReadStandardOutput, this, [this, prc](){
        const auto data = prc->readAllStandardOutput();
        qDebug() << data;
        if (prc == mProcess) Q_EMIT submitLog(data, false);
    });
    connect(prc, &QProcess::stateChanged, this, [this, prc](QProcess::ProcessState status){
        if (prc != mProcess)
            return;
        switch (static_cast<int>(status))
        {
        case QProcess::ProcessState::NotRunning:
            Q_EMIT statusChanged(ARTClient::Status::Finished);
            Q_EMIT finalize(mStartedTime.msecsTo(QDateTime::currentDateTime()), QString());
            break;
        case QProcess::ProcessState::Running:
            mStartedTime = QDateTime::currentDateTime();
            Q_EMIT statusChanged(ARTClient::Status::Started);
            break;
        case QProcess::ProcessState::Starting:
            Q_EMIT statusChanged(ARTClient::Status::Starting);
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

void TestClientSession::terminate()
{
    if (!mProcess)
        return;

    qDebug() << "Terminate";
    Q_EMIT statusChanged(ARTClient::Status::Closing);
    mProcess->terminate();
    mProcess->waitForFinished(5000);
    mProcess = nullptr;
}
