#ifndef GENRATEREPORT_H
#define GENRATEREPORT_H

#include "Controls/TextLabel.h"
#include "FileIO.h"

class GenrateReport : public QWidget
{
    Q_OBJECT
public:
    explicit GenrateReport(QWidget *parent = 0);
    ~GenrateReport();

    void valueFillInReport();
    void generatePdfFile();

private:
    FileIO*         m_fileIO;
    QString         m_versionNumber;

    void createDefaultElements();
};

#endif // GENRATEREPORT_H
