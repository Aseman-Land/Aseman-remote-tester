#ifndef ABSTRACTTEST_H
#define ABSTRACTTEST_H

#include <QObject>
#include <QJsonObject>
#include <QSharedPointer>

class AbstractTest: public QObject
{
public:
    explicit AbstractTest(const QJsonObject &configs, QObject *parent = nullptr);
    virtual ~AbstractTest();

    QJsonObject configs() const;

public Q_SLOTS:
    virtual void start() = 0;
    virtual void stop() = 0;

private:
    QJsonObject mConfigs;
};

typedef QSharedPointer<AbstractTest> AbstractTestPtr;

#endif // ABSTRACTTEST_H
