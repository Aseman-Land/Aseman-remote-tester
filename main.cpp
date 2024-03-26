#include <QCoreApplication>
#include <QFileInfo>
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QDebug>

#include "testclient.h"
#include "testserver.h"

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);

    const auto args = app.arguments();
    if (args.count() != 2)
    {
        qDebug() << "Please enter config file path.";
        qDebug() << QString(QFileInfo(app.applicationFilePath()).fileName() + " /path/to/config.json").toStdString().c_str();
        return 0;
    }

    QFile f(args.at(1));
    if (!f.open(QFile::ReadOnly))
    {
        qDebug() << "Could not open config file:" << f.errorString().toStdString().c_str();
        return 1;
    }

    const auto json = QJsonDocument::fromJson(f.readAll());
    f.close();

    if (json.isNull() || !json.isObject())
    {
        qDebug() << "Invalid json.";
        return 2;
    }

    const auto root = json.object();
    const auto instances = root.value("instances");
    if (!instances.isArray())
    {
        qDebug() << "Invalid json's test property.";
        return 3;
    }

    QSet<QSharedPointer<AbstractTest>> testsList;
    for (const auto &item: instances.toArray())
    {
        if (!item.isObject())
            continue;

        AbstractTestPtr t;

        const auto testConfigs = item.toObject();
        const auto type = testConfigs.value(QStringLiteral("type")).toString();
        if (type == QStringLiteral("client"))
            t = AbstractTestPtr(new TestClient(testConfigs));
        else if (type == QStringLiteral("server"))
            t = AbstractTestPtr(new TestServer(testConfigs));
        else
            continue;

        t->start();
        testsList.insert(t);
    }

    const auto retCode = app.exec();

    for (const auto &t: testsList)
        t->stop();

    return retCode;
}
