#ifndef PAYLOADLIB_H
#define PAYLOADLIB_H
#include "EnumFile.h"
#include <QtCore>

#define CNFG_ARRAY_SIZE         (2251) // payload size increased to incorporate (250*9) camera search result
#define MAX_TABLE               5


class PayloadLib : public QObject
{
private:
    QList<QVariant> cnfgArray;
    qint32 cnfgTableIndex;
    qint32 m_configTableIndex[MAX_TABLE];
    qint32 m_configRecordForTable[MAX_TABLE];
    qint32 m_totalCmdFields;
    qint32 m_totalReplyFields;
    qint32 m_totalTable;

public :
    PayloadLib();
    ~PayloadLib();

    qint32 getcnfgTableIndex(qint32 tableNumber = 0);

    void setcnfgTableIndex(qint32 tableIndex);

    QString createDevCnfgPayload(REQ_MSG_ID_e requestType,
                                 qint32 tableId,
                                 qint32 fromIndex,
                                 qint32 toIndex,
                                 qint32 fromField,
                                 qint32 toField,
                                 qint32 totalField,
                                 QString payloadString = "",
                                 qint32 startingCnfgArrayIndex = 0);

    QString createDevCmdPayload(qint32 totalFields);

    void parsePayload(REQ_MSG_ID_e requestType, QString devPayload);

    void parseDevCmdReply(bool hasIndex, QString devCmdReply);

    void setCnfgArrayAtIndex(qint32 index, QVariant value);

    QVariant getCnfgArrayAtIndex(qint32 index);

    qint32 getTotalCmdFields();

    qint32 getTotalReplyFields();

    qint32 getTotalConfigFieldOfTable(qint32 tableIndex = 0);

    void clearPayloadLib();
};
#endif // PAYLOADLIB_H
