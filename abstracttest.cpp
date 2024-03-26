#include "abstracttest.h"

AbstractTest::AbstractTest(const QJsonObject &configs, QObject *parent)
    : QObject(parent)
    , mConfigs(configs)
{
}

AbstractTest::~AbstractTest()
{
}

QJsonObject AbstractTest::configs() const
{
    return mConfigs;
}
