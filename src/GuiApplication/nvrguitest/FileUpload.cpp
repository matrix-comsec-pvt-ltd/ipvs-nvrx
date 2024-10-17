#include <unistd.h>
#include <sys/stat.h>

#include "FileUpload.h"
#include "BoardTypeWiseInfo.h"
#include "HardwareTestControl.h"
#include "CommonApi.h"

#define	FOLDER_PERMISSION   (S_IRWXU | S_IRGRP | S_IXGRP | S_IROTH | S_IXOTH)
#define UPLOAD_PATH         MEDIA_DIR_PATH "/USB"

FileUpload::FileUpload()
{
    m_networkError = QNetworkReply::NoError;
    INIT_OBJ(m_networkNetRepsonse);
    INIT_OBJ(m_dataUpload);
}

FileUpload::~FileUpload()
{
    disconnect(m_networkNetRepsonse,
            SIGNAL(finished()),
            this,
            SLOT(dataSent()));

    if(IS_VALID_OBJ(m_networkNetRepsonse))
    {
       m_networkNetRepsonse->deleteLater();
       m_networkNetRepsonse = NULL;
    }

    if(IS_VALID_OBJ(m_dataUpload))
    {
        m_dataUpload->deleteLater();
        m_dataUpload = NULL;
    }
}

void FileUpload::upLoadFileToUsb()
{
    QString     uploadPath = "";
    bool        result = false;
    QString     readPdfPath = HW_TEST_REPORT_PATH + QString(HardwareTestControl::boardTypeString).replace(' ', '_')
                            + QString("_") + QString(HardwareTestControl::deviceSerialNumber) + QString(".pdf");
    CHAR        sysCmd[200];

    if(access (readPdfPath.toUtf8().constData(), F_OK) != STATUS_OK)
    {
        emit usbBkpRespRcvd (false);
        EPRINT(GUI_SYS, "Hardware test resport not present: [path=%s]", readPdfPath.toUtf8().constData());
    }
    else
    {
        for(quint8 index = 0; index< BoardTypeWiseInfo::noOfUsb; index++)
        {
            uploadPath = UPLOAD_PATH + QString("%1").arg (index+1);
            if(access (uploadPath.toLatin1(), F_OK) != STATUS_OK )
            {
                continue;
            }

            uploadPath = uploadPath + QString("/") + TEST_REPORT_FOLDER;
            if(access (uploadPath.toLatin1(), F_OK) != STATUS_OK )
            {
                if(mkdir(uploadPath.toLatin1 (), FOLDER_PERMISSION) != STATUS_OK)
                {
                    emit usbBkpRespRcvd (false);
                    EPRINT(GUI_SYS, "upload path not present: [uploadPath=%s]", uploadPath.toUtf8().constData());
                    continue;
                }
            }

            uploadPath = UPLOAD_PATH + QString("%1").arg (index+1) + QString("/") + TEST_REPORT_FOLDER + QString("/");
            uploadPath = uploadPath.trimmed();

            snprintf(sysCmd,sizeof(sysCmd), "cp %s %s", readPdfPath.toUtf8().constData(), uploadPath.toUtf8().constData());
            result = Utils_ExeCmd(sysCmd);
        }
    }

    /* Remove PDF File after upload */
    snprintf(sysCmd,sizeof(sysCmd), "rm -rf %s", readPdfPath.toUtf8().constData());
    Utils_ExeCmd(sysCmd);

    emit usbBkpRespRcvd (result);
}

void FileUpload::upLoadFileToFtp(QString url)
{
    QString readPdfPath = HW_TEST_REPORT_PATH + QString(HardwareTestControl::boardTypeString).replace(' ', '_')
            + QString("_") + QString(HardwareTestControl::deviceSerialNumber) + QString(".pdf");

    m_networkError = QNetworkReply::NoError;

    timer.setSingleShot(true);

    if (false == IS_VALID_OBJ(m_networkNetRepsonse) && (false == IS_VALID_OBJ(m_dataUpload)))
    {
        if (access(HW_TEST_REPORT_PATH, F_OK) != STATUS_OK)
        {
            emit ftpWriteStatus (false);
            EPRINT(GUI_SYS, "source path not present: [source=%s]", HW_TEST_REPORT_PATH);
        }
        if( access (readPdfPath.toUtf8().constData(), F_OK) != STATUS_OK )
        {
            emit usbBkpRespRcvd (false);
            EPRINT(GUI_SYS, "Hardware test resport not present: [path=%s]", readPdfPath.toUtf8().constData());
        }

        url= url.trimmed ();

        QUrl uploadurl(url);

        uploadurl.setPort(HardwareTestControl::ftpPort.toInt());

        m_dataUpload  = new QFile(readPdfPath, this);
        if(IS_VALID_OBJ(m_dataUpload))
        {
            if(m_dataUpload->open(QIODevice::ReadOnly))
            {
                m_networkNetRepsonse = m_networkManger.put (QNetworkRequest(uploadurl),m_dataUpload);
                connect(&timer, SIGNAL(timeout()), &loop,
                        SLOT(quit()));
                connect(m_networkNetRepsonse, SIGNAL(finished()),
                        &loop, SLOT(quit()));
                timer.start(30000);
                loop.exec();
            }
            else
            {
                EPRINT(GUI_SYS, "failed to open file: [readPath=%s]", readPdfPath.toUtf8().constData());
            }
        }
        else
        {
            EPRINT(GUI_SYS, "QFile failed: [readPath=%s]", readPdfPath.toUtf8().constData());
        }
     }
    else
    {
        EPRINT(GUI_SYS, "upload file to ftp server: in progress");
    }

    if(timer.isActive())
    {
        timer.stop();
        if(m_networkNetRepsonse->error() > 0)
        {
            disconnect(m_networkNetRepsonse, SIGNAL(finished()),
                       &loop, SLOT(quit()));
            m_networkNetRepsonse->abort();
            dataError(m_networkNetRepsonse->error());
        }
        else
        {
            dataSent();
        }
    }
    else
    {
        EPRINT(GUI_SYS, "failed to upload file to ftp server: timeout");
        disconnect(m_networkNetRepsonse, SIGNAL(finished()),
                   &loop, SLOT(quit()));
        m_networkNetRepsonse->abort();
        this->dataError(m_networkNetRepsonse->error());
    }

    /* Remove PDF File after upload */
    CHAR sysCmd[200];

    snprintf(sysCmd,sizeof(sysCmd), "rm -rf %s", readPdfPath.toUtf8().constData());
    Utils_ExeCmd(sysCmd);
}

void FileUpload::dataError(QNetworkReply::NetworkError)
{
    if(false == IS_VALID_OBJ(m_networkNetRepsonse) || (false == IS_VALID_OBJ(m_dataUpload)))
    {
        EPRINT(GUI_SYS, "invld objects: [m_networkNetRepsonse=%p] [m_dataUpload=%p] ", m_networkNetRepsonse, m_dataUpload);
    }

    m_networkError = m_networkNetRepsonse->error();

    EPRINT(GUI_SYS, "failed to upload file to ftp server: [m_networkError=%d]", m_networkError);
    emit ftpWriteStatus (false);

    m_dataUpload->close();
    m_dataUpload->deleteLater();
    m_networkNetRepsonse->deleteLater();

    m_dataUpload = NULL;
    m_networkNetRepsonse = NULL;
}

void FileUpload::dataSent()
{
    if(false == IS_VALID_OBJ(m_networkNetRepsonse) || (false == IS_VALID_OBJ(m_dataUpload)))
    {
        EPRINT(GUI_SYS, "invld objects: [m_networkNetRepsonse=%p] [m_dataUpload=%p] ", m_networkNetRepsonse, m_dataUpload);
    }

    m_networkError = m_networkNetRepsonse->error();
    if (m_networkError == QNetworkReply::NoError)
    {
        DPRINT(GUI_SYS, "file uploaded to ftp server sucessfully");
        emit ftpWriteStatus (true);
    }
    else
    {
        EPRINT(GUI_SYS, "failed to upload file to ftp server: [m_networkError=%d]", m_networkError);
        emit ftpWriteStatus (false);
    }

    m_dataUpload->close();
    m_dataUpload->deleteLater();
    m_networkNetRepsonse->deleteLater();

    m_dataUpload = NULL;
    m_networkNetRepsonse = NULL;
}
