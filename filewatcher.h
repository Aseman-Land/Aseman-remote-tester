#ifndef FILEWATCHER_H
#define FILEWATCHER_H

#include <QObject>
#include <QFileSystemWatcher>
#include <QDateTime>
#include <QTimer>

class FileWatcher : public QObject
{
    Q_OBJECT
public:
    explicit FileWatcher(const QStringList &list, QObject *parent = nullptr);
    virtual ~FileWatcher();

Q_SIGNALS:
    void changed(const QString &path);

protected:
    bool add(const QString &path);
    void checkChanged(const QString &path);

private:
    struct FileDetails {
        QString path;
        qint64 size;
        QDateTime modifyTime;
        QDateTime createTime;
    };

    QStringList mList;
    QHash<QString, QList<FileDetails>> mDirFiles;
    QStringList mChangedListCache;

    QFileSystemWatcher *mWatcher = nullptr;
    QTimer *mChangeTimer;
};

#endif // FILEWATCHER_H
