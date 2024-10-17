#include "FileIO.h"
#include "DebugLog.h"

#include <QFile>
#include <QTextStream>
#include <unistd.h>

FileIO::FileIO(QObject *parent, QString source):
    QObject(parent)
{
    mSource = source;
}

QString FileIO::read()
{
    if (mSource.isEmpty())
    {
        EPRINT(GUI_SYS, "File path not set");
        return QString();
    }

    QFile file(mSource);

    QString fileContent;
    if (false == file.open(QIODevice::ReadOnly))
    {
        EPRINT(GUI_SYS, "Unable to open the file[%s]", mSource.toUtf8().data());
        return QString();
    }

    QString line;
    QTextStream t( &file );
    do
    {
        line = t.readLine();
        fileContent += line+"\n";
    }while (!line.isNull());

    file.close();
    return fileContent;
}

bool FileIO::write(const QString& data)
{
    if (mSource.isEmpty())
    {
        EPRINT(GUI_SYS, "File path not set");
        return false;
    }

    QFile file(mSource);

    if (false == file.open(QIODevice::WriteOnly|QIODevice::Append))
    {
        EPRINT(GUI_SYS, "Unable to open the file[%s]", mSource.toUtf8().data());
        return false;
    }

    QTextStream out(&file);
    QString str;
    str = data;
    out << str;
    file.flush ();
    file.close();
    sync();
    return true;
}

void FileIO:: changeSourcePath(QString source)
{
    mSource = source;
}
