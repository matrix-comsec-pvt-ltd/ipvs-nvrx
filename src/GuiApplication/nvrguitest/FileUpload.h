#ifndef FILEUPLOAD_H
#define FILEUPLOAD_H

#include <QObject>
#include <QNetworkReply>
#include <QFile>
#include <QEventLoop>
#include <QTimer>

class FileUpload: public QObject
{
    Q_OBJECT
public:
    FileUpload();
    ~FileUpload();

    void upLoadFileToUsb();
    void upLoadFileToFtp(QString url);

signals:
    void usbBkpRespRcvd(bool status);
    void ftpWriteStatus(bool status);

public slots:
    void dataError(QNetworkReply::NetworkError);
    void dataSent();

private:
    QNetworkAccessManager       m_networkManger;
    QNetworkReply*              m_networkNetRepsonse;
    QNetworkReply::NetworkError m_networkError;
    QFile*                      m_dataUpload;
    QEventLoop                  loop;
    QTimer                      timer;
};

#endif // USBFILEUPLOAD_H
