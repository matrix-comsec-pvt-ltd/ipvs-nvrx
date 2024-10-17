#ifndef DATASTRUCTURE_H
#define DATASTRUCTURE_H

#include <QRegExp>

class TextboxParam
{
public:
    QString     textStr;
    QString     labelStr;
    QString     suffixStr;
    bool        isEmailAddrType;
    bool        isCentre;
    quint16     leftMargin;
    quint16     maxChar;
    quint16     minChar;
    bool        isTotalBlankStrAllow;
    bool        isNumEntry;
    quint32     minNumValue;
    quint32     maxNumValue;
    int         extraNumValue;
    QRegExp     validation;
    QRegExp     startCharVal;
    QRegExp     middelCharVal;
    QRegExp     endCharVal;

    TextboxParam()
    {
        textStr = labelStr = suffixStr = "";
        validation = startCharVal = middelCharVal = endCharVal = QRegExp("");
        isCentre = true;
        isNumEntry = isEmailAddrType = isTotalBlankStrAllow = false;
        leftMargin = minChar = maxChar = minNumValue = maxNumValue = 0;
        extraNumValue = -1;
    }
};

#endif // DATASTRUCTURE_H
