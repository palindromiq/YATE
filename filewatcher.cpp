#include "filewatcher.h"
#include <stdlib.h>
#include <stdio.h>
#include <windows.h>
#include <Winbase.h>
#include <QFileInfo>
#include <QDir>

#define MAX_DIRS 25
#define MAX_FILES 255
#define MAX_BUFFER 4096

namespace Yate {

FileWatcher::FileWatcher(QObject *parent, QString file): QObject(parent)
{
    setFilePath(file);
}

const QString &FileWatcher::filePath() const
{
    return filePath_;
}

void FileWatcher::setFilePath(const QString &newFilePath)
{
    QString trimmedPath = newFilePath.trimmed();
    if (trimmedPath.length()) {
        filePath_ = trimmedPath;
    }

}


void FileWatcher::start()
{
    running_.storeRelaxed(1);
    QString parentPath = QFileInfo(filePath_).dir().path();
    QString fileBaseName = QFileInfo(filePath_).fileName();
    HANDLE dirHandler = CreateFile(parentPath.toStdWString().c_str(), FILE_LIST_DIRECTORY, FILE_SHARE_READ | FILE_SHARE_WRITE |FILE_SHARE_DELETE,
                                   NULL, OPEN_EXISTING, FILE_FLAG_BACKUP_SEMANTICS, NULL);
    wchar_t filename[MAX_PATH];
    FILE_NOTIFY_INFORMATION dirBuffer[1024];
    DWORD bytesRead;

    while (ReadDirectoryChangesW(dirHandler, &dirBuffer, sizeof(dirBuffer),
               FALSE,
               FILE_NOTIFY_CHANGE_SECURITY |
               FILE_NOTIFY_CHANGE_CREATION |
               FILE_NOTIFY_CHANGE_LAST_ACCESS |
               FILE_NOTIFY_CHANGE_LAST_WRITE |
               FILE_NOTIFY_CHANGE_SIZE |
               FILE_NOTIFY_CHANGE_ATTRIBUTES |
               FILE_NOTIFY_CHANGE_DIR_NAME |
               FILE_NOTIFY_CHANGE_FILE_NAME,
               &bytesRead, NULL, NULL) && running_.loadRelaxed()
           )
    {
        int offset = 0;
        FILE_NOTIFY_INFORMATION* fileBuffer;
        fileBuffer = (FILE_NOTIFY_INFORMATION*) ((char*)dirBuffer + offset);
        wcscpy(filename, L"");
        wcsncpy(filename, fileBuffer->FileName, fileBuffer->FileNameLength / 2);

#pragma GCC diagnostic ignored "-Wconversion-null"
        filename[fileBuffer->FileNameLength / 2] = NULL;

        bool filter = QString::fromWCharArray(filename) == fileBaseName;
        qDebug() << QString::fromWCharArray(filename) << filter;
        if (filter) {
            if (dirBuffer[0].Action == FILE_ACTION_MODIFIED) {
                emit fileChanged(true);
            } else if (dirBuffer[0].Action == FILE_ACTION_ADDED) {
                emit fileChanged(true);
            } else if (dirBuffer[0].Action == FILE_ACTION_REMOVED) {
                emit fileChanged(false);
            }
        }
        if (dirBuffer[0].Action == FILE_ACTION_RENAMED_OLD_NAME) {
            emit fileChanged(QFileInfo::exists(filePath_));
        } else if (dirBuffer[0].Action == FILE_ACTION_RENAMED_NEW_NAME) {
            emit fileChanged(QFileInfo::exists(filePath_));
        }


    }

    CloseHandle(dirHandler);
}

void FileWatcher::stop()
{
    running_.storeRelaxed(0);
}

bool FileWatcher::running() const
{
    return running_.loadRelaxed();
}

}
