#include "filewatcher.h"

#include <QDir>
#include <QFileInfo>
#include <QDebug>
#include <QTimer>

FileWatcher::FileWatcher(const QStringList &list, QObject *parent)
    : QObject(parent)
    , mList(list)
{
    mWatcher = new QFileSystemWatcher(this);

    connect(mWatcher, &QFileSystemWatcher::fileChanged, this, &FileWatcher::checkChanged);
    connect(mWatcher, &QFileSystemWatcher::directoryChanged, this, &FileWatcher::checkChanged);

    mChangeTimer = new QTimer(this);
    mChangeTimer->setInterval(300);
    mChangeTimer->setSingleShot(false);

    connect(mChangeTimer, &QTimer::timeout, this, [this](){
        while (!mChangedListCache.isEmpty())
        {
            const auto path = mChangedListCache.takeFirst();
            if (!QFileInfo::exists(path))
                continue;

            qDebug() << "FILE CHANGED" << path;
            Q_EMIT changed(path);
        }
    });

    for (const auto &l: list)
        add(l);
}

FileWatcher::~FileWatcher()
{

}

bool FileWatcher::add(const QString &path)
{
    if (path.isEmpty() || path.at(0) != '/')
        return false;

    QFileInfo f(path);
    if (f.isFile())
    {
        qDebug() << "Watching" << path;
        FileDetails details;
        details.path = f.filePath();
        details.size = f.size();
        details.modifyTime = f.fileTime(QFile::FileBirthTime);
        details.createTime = f.fileTime(QFile::FileBirthTime);

        const auto parentPath = f.path();

        mDirFiles[parentPath] << details;
        mWatcher->addPath(parentPath);

        checkChanged(parentPath);
    }
    else
    {
        mWatcher->addPath(path);
        const auto dirs = QDir(path).entryList(QDir::Dirs | QDir::NoDotAndDotDot);
        for (const auto &d: dirs)
            add(path + '/' + d);
    }

    return true;
}

void FileWatcher::checkChanged(const QString &path)
{
    for (const auto &d: mDirFiles.value(path))
    {
        if (!mList.contains(d.path))
            return;

        QFileInfo inf(d.path);
        if (!inf.exists())
            return;

        mChangedListCache.removeAll(d.path);
        mChangedListCache << d.path;
        mChangeTimer->stop();
        mChangeTimer->start();
    }
}
