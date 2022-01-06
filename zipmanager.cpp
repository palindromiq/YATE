#include <QFile>
#include <QDebug>
#include <QDir>
#include <QFileInfo>
#include "zipmanager.h"
#include "miniz.h"

namespace Yate {

ZipManager::ZipManager(QObject *parent)
    : QObject{parent}
{

}

bool ZipManager::unzip(QString sourceArchive, QString destDir)
{
    mz_zip_archive zipArchive;
    memset(&zipArchive, 0, sizeof(zipArchive));
    mz_bool status;
    auto sourceArchiveBA = sourceArchive.toUtf8 ();
    const char *sourceArchiveFileName =  sourceArchiveBA.data();
    status = mz_zip_reader_init_file(&zipArchive, sourceArchiveFileName, 0);
    if (!status)
    {
        qDebug() << "mz_zip_reader_init_file() failed!";
        qDebug() << mz_zip_get_error_string(zipArchive.m_last_error);
        return false;
    }
    for (int i = 0; i < (int)mz_zip_reader_get_num_files(&zipArchive); i++)
     {
       mz_zip_archive_file_stat file_stat;
       if (!mz_zip_reader_file_stat(&zipArchive, i, &file_stat))
       {
           qDebug() << "mz_zip_reader_file_stat() failed!";
           mz_zip_reader_end(&zipArchive);
           return false;
       }

//       qDebug() << file_stat.m_filename << file_stat.m_comment << file_stat.m_uncomp_size << file_stat.m_comp_size << mz_zip_reader_is_file_a_directory(&zipArchive, i);
       if (mz_zip_reader_is_file_a_directory(&zipArchive, i)) {
           continue;
       }
       QString strippedFileName = QFileInfo(file_stat.m_filename).fileName();
       auto extractionFileNameBA = QString(destDir + QDir::separator() + strippedFileName).toUtf8();
       const char * extractionFileName = extractionFileNameBA.data();
       status = mz_zip_reader_extract_file_to_file(&zipArchive, file_stat.m_filename, extractionFileName, 0);
       if (!status)
       {
           qDebug() << "mz_zip_reader_extract_file_to_file() failed!";
           qDebug() << mz_zip_get_error_string(zipArchive.m_last_error);
           mz_zip_reader_end(&zipArchive);
           return false;
       }
    }

     mz_zip_reader_end(&zipArchive);
     return true;

}

}
