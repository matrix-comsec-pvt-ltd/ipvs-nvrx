#include "GenrateReport.h"
#include "Enumfile.h"
#include <sys/sysinfo.h>

#include <QTextDocument>
#include <QPrinter>
#include <QMargins>
#include "MainWindow.h"

QStringList passFailString = (QStringList() <<"Failed" << "Passed");

GenrateReport::GenrateReport(QWidget *parent) : QWidget(parent)
{
    INIT_OBJ(m_fileIO);
    m_versionNumber = QString("V%1R%2").arg(SOFTWARE_VERSION, 2, 10, QLatin1Char('0')).arg(SOFTWARE_REVISION, 2, 10, QLatin1Char('0'));
    if (PRODUCT_SUB_REVISION)
    {
        m_versionNumber.append(QString(".%1").arg(PRODUCT_SUB_REVISION));
    }
}

GenrateReport::~GenrateReport()
{
    DELETE_OBJ(m_fileIO);
}

void GenrateReport::valueFillInReport()
{
    QString filePath = HW_TEST_REPORT_PATH + QString(HardwareTestControl::boardTypeString).replace(' ', '_')
            + QString("_") + QString(HardwareTestControl::deviceSerialNumber) + QString(".txt");

    m_fileIO = new FileIO(this,filePath);
    if(false == IS_VALID_OBJ(m_fileIO))
    {
        EPRINT(GUI_SYS, "FileIO() failed: [filePath=%s]", filePath.toUtf8().constData());
        return;
    }

    QString data;
    QVector <QStringRef> reportList;

    data = m_fileIO->read();
    reportList = data.splitRef("*************************");

    quint8 m_maxAvailableReport = reportList.length() - 1;
    QString date = QString("%1").arg(HardwareTestControl::testStartTime.tm_mday,2,10,QLatin1Char('0')) + QString("/") +
            QString("%1").arg(HardwareTestControl::testStartTime.tm_mon,2,10,QLatin1Char('0')) + QString("/") +
            QString("%1").arg(HardwareTestControl::testStartTime.tm_year);
    QString time = QString("%1").arg(HardwareTestControl::testStartTime.tm_hour,2,10,QLatin1Char('0')) + QString(":") +
            QString("%1").arg(HardwareTestControl::testStartTime.tm_min,2,10,QLatin1Char('0')) + QString(":") +
            QString("%1").arg(HardwareTestControl::testStartTime.tm_sec,2,10,QLatin1Char('0'));

    QString reportBuff = QString("\n\n              " ENTERPRISE_STRING " ") + HardwareTestControl::boardTypeString;

    reportBuff.append(QString(" Test Report - ") + m_versionNumber + QString("\n\n"));
    reportBuff.append(QString("Product Name :                 ")  + QString(ENTERPRISE_STRING " ") + HardwareTestControl::boardTypeString + QString("\n"));
    reportBuff.append(QString("Device Serial Number :     ") + HardwareTestControl::deviceSerialNumber + QString("\n"));
    reportBuff.append(QString("Software Version :             ") + m_versionNumber + QString("\n"));
    reportBuff.append(QString("MAC Address 1 :               ") + HardwareTestControl::macAdd1 + QString("\n"));
    if (BoardTypeWiseInfo::noOfLan == 2)
    {
        reportBuff.append(QString("MAC Address 2 :               ") + HardwareTestControl::macAdd2 + QString("\n"));
    }

    reportBuff.append(QString("\n"));
    reportBuff.append(QString("Tested By :              ") + HardwareTestControl::testEmpId + QString("\n"));
    reportBuff.append(QString("Date :                      ") + date + QString("\n"));
    reportBuff.append(QString("Time :                      ") + time + QString("\n"));
    reportBuff.append(QString("Time Taken :           ") + HardwareTestControl::testStartEndDiff + QString("\n"));
    reportBuff.append(QString("Re-test Number :    ") + QString("%1").arg(m_maxAvailableReport) + QString("\n\n"));
    reportBuff.append(QString("Voltage :                                 ") + QString("Passed") + QString("\n"));
    reportBuff.append(QString("BUZZER :                                  ") + passFailString.at(HardwareTestControl::testResult[BUZZER_HW_TEST]) + QString("\n"));
    reportBuff.append(QString("LED :                                       ") + passFailString.at(HardwareTestControl::testResult[LED_HW_TEST]) + QString("\n"));
    reportBuff.append(QString("HDMI Video :                          ") + passFailString.at(HardwareTestControl::testResult[HDMI_VIDEO_HW_TEST]) + QString("\n"));

    if (true == BoardTypeWiseInfo::isAudioInOutSupport)
    {
        reportBuff.append(QString("Audio IN :                               ") + passFailString.at(HardwareTestControl::testResult[AUDIO_IN_HW_TEST]) + QString("\n"));
        reportBuff.append(QString("Audio OUT :                            ") + passFailString.at(HardwareTestControl::testResult[AUDIO_OUT_HW_TEST]) + QString("\n"));
        reportBuff.append(QString("HDMI Audio :                          ") + passFailString.at(HardwareTestControl::testResult[HDMI_AUDIO_HW_TEST]) + QString("\n"));
    }

    reportBuff.append(QString("RTC :                                       ") + passFailString.at(HardwareTestControl::testResult[RTC_HW_TEST]) + QString("\n"));
    reportBuff.append(QString("Ethernet :                                ") + passFailString.at(HardwareTestControl::testResult[ETHERNET_HW_TEST]) + QString("\n"));

    if (BoardTypeWiseInfo::noOfSensorInOut)
    {
        reportBuff.append(QString("Sensor IN and Alarm OUT :    ") + passFailString.at(HardwareTestControl::testResult[SENSOR_IN_ALARM_OUT_HW_TEST]) + QString("\n"));
    }

    reportBuff.append(QString("HDD :                                      ") + passFailString.at(HardwareTestControl::testResult[HDD_HW_TEST]) + QString("\n"));
    reportBuff.append(QString("USB :                                       ") + passFailString.at(HardwareTestControl::testResult[USB_HW_TEST]) + QString("\n\n"));
    reportBuff.append(QString("               *************************           "));

    if(false == m_fileIO->write(reportBuff))
    {
        EPRINT(GUI_SYS, "FileIO() write failed: [filePath=%s]", filePath.toUtf8().constData());
    }
    DELETE_OBJ(m_fileIO);
    DPRINT(GUI_SYS, "Hardware test report generated: [txtFilePath=%s]", filePath.toUtf8().constData());
}

void GenrateReport::generatePdfFile()
{
    QString filePath = HW_TEST_REPORT_PATH + QString(HardwareTestControl::boardTypeString).replace(' ', '_')
            + QString("_") + QString(HardwareTestControl::deviceSerialNumber) + QString(".txt");

    m_fileIO = new FileIO(this,filePath);
    if(false == IS_VALID_OBJ(m_fileIO))
    {
        EPRINT(GUI_SYS, "FileIO() failed: [filePath=%s]", filePath.toUtf8().constData());
        return;
    }

    QString fileData = m_fileIO->read ();
    QString pdfFilePath = HW_TEST_REPORT_PATH + QString(HardwareTestControl::boardTypeString).replace(' ', '_')
            + QString("_") + QString(HardwareTestControl::deviceSerialNumber) + QString(".pdf");

    QTextDocument *document = new QTextDocument(fileData);

    QPrinter pdfDoc(QPrinter::PrinterResolution);
    pdfDoc.setOutputFormat(QPrinter::PdfFormat);
    pdfDoc.setPaperSize(QPrinter::A4);
    pdfDoc.setPageMargins(QMarginsF(15,15,15,15));
    pdfDoc.setOutputFileName(pdfFilePath);

    document->print(&pdfDoc);

    DELETE_OBJ(m_fileIO);
    DPRINT(GUI_SYS, "Hardware test report generated: [pdfFilePath=%s]", pdfFilePath.toUtf8().constData());
}
