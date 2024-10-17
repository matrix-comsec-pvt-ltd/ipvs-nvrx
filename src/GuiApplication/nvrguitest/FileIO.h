#ifndef FILEIO_H
#define FILEIO_H

#include <QObject>

class FileIO : public QObject
{
    Q_OBJECT
public:
    explicit FileIO(QObject *parent = 0, QString source = "");

    Q_INVOKABLE QString read();
    Q_INVOKABLE bool write(const QString& data);
    void changeSourcePath(QString source);

private:
    QString mSource;
};

#endif // FILEIO_H
