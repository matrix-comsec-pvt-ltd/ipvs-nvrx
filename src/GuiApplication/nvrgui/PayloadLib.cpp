#include "PayloadLib.h"

PayloadLib :: PayloadLib()
{
    cnfgArray.reserve(CNFG_ARRAY_SIZE);
    for(qint32 index = 0; index < CNFG_ARRAY_SIZE; index++)
    {
        cnfgArray.insert(index, 0);
    }
    cnfgTableIndex = 0;

    for(qint8 index = 0; index < MAX_TABLE; index++)
    {
        m_configRecordForTable[index] = 0;
        m_configTableIndex[index] = 0;
    }
    m_totalCmdFields = 0;
    m_totalReplyFields = 0;
    m_totalTable = 0;
}

PayloadLib::~PayloadLib()
{
    cnfgArray.clear();
}

QString PayloadLib :: createDevCmdPayload(qint32 totalFields)
{
    QString payloadString = "";

    for(qint32 index = 0; index < totalFields; index++)
    {
        payloadString = payloadString + cnfgArray.at(index).toString();
        payloadString += QString(QChar(FSP));
    }
    return payloadString;
}

QString PayloadLib :: createDevCnfgPayload(REQ_MSG_ID_e requestType,
                                           qint32 tableId,
                                           qint32 fromIndex,
                                           qint32 toIndex,
                                           qint32 fromField,
                                           qint32 toField,
                                           qint32 totalField,
                                           QString payloadString,
                                           qint32 startingCnfgArrayIndex)
{
    switch(requestType)
    {
    case MSG_GET_CFG :
    {
        payloadString.append (SOT);
        payloadString.append (QString("%1").arg(tableId));      // tableId
        payloadString.append (FSP);
        payloadString.append (QString("%1").arg(fromIndex));    // from index
        payloadString.append (FSP);
        payloadString.append (QString("%1").arg(toIndex));      // to index
        payloadString.append (FSP);
        payloadString.append (QString("%1").arg(fromField));    // from field
        payloadString.append (FSP);
        payloadString.append (QString("%1").arg(toField));      // to field
        payloadString.append (FSP);
        payloadString.append (EOT);
    }
        break;

    case MSG_DEF_CFG:
    {
        payloadString.append(SOT);
        payloadString.append(QString("%1").arg(tableId));      // tableId
        payloadString.append(FSP);
        payloadString.append(QString("%1").arg(fromIndex));    // from index
        payloadString.append(FSP);
        payloadString.append(QString("%1").arg(toIndex));      // to index
        payloadString.append(FSP);
        payloadString.append(EOT);
    }
        break;

    case MSG_SET_CFG:
    {
        quint8 fieldDiff = (toField - fromField) + 1;
        qint16 totalRecords = totalField / fieldDiff;
        payloadString.append(SOT);
        payloadString.append(QString("%1").arg(tableId));      // tableId
        payloadString.append(FSP);
        for(qint16 recordIndex = 0; recordIndex < totalRecords; recordIndex++)
        {
            payloadString.append(SOI);
            payloadString.append(QString("%1").arg(fromIndex++));            //record Index
            payloadString.append(FSP);
            for(qint16 fieldIndex = 0, fieldNumber = fromField; fieldIndex < fieldDiff; fieldIndex++)
            {
                payloadString.append(QString("%1").arg(fieldNumber++));        //field Index
                payloadString.append(FVS);
                payloadString.append(cnfgArray.at((recordIndex * fieldDiff) + fieldIndex + startingCnfgArrayIndex).toString());   //value
                payloadString.append(FSP);
            }
            payloadString.append(EOI);
        }
        payloadString.append(EOT);
    }
        break;

    default:
        break;
    }
    return payloadString;
}

void PayloadLib::parsePayload(REQ_MSG_ID_e requestType, QString devPayload)
{
    QStringList recordsRowList;
    QStringList recordColList;
    QStringList recordList;
    QString oneRecordRow, oneRecordCol;
    qint32 colIndex, rowIndex, arrayIndex;
    for(arrayIndex = 0; arrayIndex < CNFG_ARRAY_SIZE ; arrayIndex++)
    {
        cnfgArray.replace(arrayIndex, "");
    }
    arrayIndex = 0;

    QString regularExpressionStringTable = "";
    regularExpressionStringTable.append('[');
    regularExpressionStringTable.append(SOT);
    regularExpressionStringTable.append(EOT);
    regularExpressionStringTable.append(']');
    QRegExp regExpForTable(regularExpressionStringTable);

    QString regularExpressionStringRecord = "";
    regularExpressionStringRecord.append('[');
    regularExpressionStringRecord.append(SOI);
    regularExpressionStringRecord.append(EOI);
    regularExpressionStringRecord.append(']');
    QRegExp regExpForRecord(regularExpressionStringRecord);

    QString regularExpressionStringField = "";
    regularExpressionStringField.append('[');
    regularExpressionStringField.append(FSP);
    regularExpressionStringField.append(']');
    QRegExp regExpForField(regularExpressionStringField);

    QString regularExpressionStringFieldValue = "";
    regularExpressionStringFieldValue.append('[');
    regularExpressionStringFieldValue.append(FVS);
    regularExpressionStringFieldValue.append(']');
    QRegExp regExpForFieldValue(regularExpressionStringFieldValue);


    QStringList devicePayloadList = devPayload.split(regExpForTable, QString::SkipEmptyParts);
    m_totalTable = devicePayloadList.length();

    memset (&m_configTableIndex,0,sizeof(m_configTableIndex));

    for(qint32 tableIndex = 0; tableIndex < m_totalTable; tableIndex++)
    {
        QString devicePayload = devicePayloadList.at(tableIndex);
        switch(requestType)
        {
        case MSG_GET_CFG :
            m_configTableIndex[tableIndex] = devicePayload.mid(0, devicePayload.indexOf(QChar(FSP))).toInt();
            devicePayload = devicePayload.mid((devicePayload.indexOf(QChar(FSP)) + 1));
            recordsRowList = devicePayload.split(regExpForRecord, QString::SkipEmptyParts);
            for(rowIndex = 0; rowIndex < recordsRowList.size(); rowIndex++)
            {
                oneRecordRow = recordsRowList.at(rowIndex);
                recordColList = oneRecordRow.split(regExpForField, QString::SkipEmptyParts);
                recordColList.removeFirst();
                for(colIndex = 0; colIndex < recordColList.size(); colIndex++)
                {
                    oneRecordCol = recordColList.at(colIndex);
                    recordList = oneRecordCol.split(regExpForFieldValue, QString::KeepEmptyParts);
                    if(arrayIndex >= CNFG_ARRAY_SIZE)
                    {
                        EPRINT(GUI_SYS,"ERROR: Parsing Failed,Buffer limit reached [Max=%d]",CNFG_ARRAY_SIZE);
                        break;
                    }
                    cnfgArray.replace(arrayIndex++, recordList.at(1));
                }
            }
            break;

        default:
            break;
        }
        m_configRecordForTable[tableIndex] = recordsRowList.size() * recordColList.size();
        recordsRowList.clear();
        recordColList.clear();
    }
}

void PayloadLib :: parseDevCmdReply(bool hasIndex, QString devCmdReply)
{
    m_totalCmdFields = 0;
    m_totalReplyFields = 0;
    QString regularExpressionStringRecord = "";
    regularExpressionStringRecord.append('[');
    regularExpressionStringRecord.append(SOI);
    regularExpressionStringRecord.append(EOI);
    regularExpressionStringRecord.append(']');
    QRegExp regExpForRecord(regularExpressionStringRecord);

    QString regularExpressionStringField = "";
    regularExpressionStringField.append('[');
    regularExpressionStringField.append(FSP);
    regularExpressionStringField.append(']');
    QRegExp regExpForField(regularExpressionStringField);

    for(qint32 index = 0; index < CNFG_ARRAY_SIZE ; index++)
    {
        cnfgArray.replace(index, "");
    }
    QStringList deviceReplyList = devCmdReply.split(regExpForRecord, QString::SkipEmptyParts);
    for(qint32 recordIndex = 0; recordIndex < deviceReplyList.length(); recordIndex++)
    {
        QString devicePayload = deviceReplyList.at(recordIndex);
        QStringList replyList = devicePayload.split(regExpForField, QString::KeepEmptyParts);
        replyList.removeLast();
        if(hasIndex)
        {
            replyList.removeFirst();
        }
        m_totalReplyFields = replyList.length();
        for(qint32 feildIndex = 0; feildIndex < replyList.length(); feildIndex++)
        {
            if(m_totalCmdFields >= CNFG_ARRAY_SIZE)
            {
                EPRINT(GUI_SYS,"ERROR: Parsing Failed,Buffer limit reached [Max=%d]",CNFG_ARRAY_SIZE);
                break;
            }
            cnfgArray.replace(m_totalCmdFields++, replyList.at(feildIndex));
        }
    }
}

void PayloadLib :: setCnfgArrayAtIndex(qint32 index, QVariant value)
{
    cnfgArray.replace(index, value.toString());
}

QVariant PayloadLib :: getCnfgArrayAtIndex(qint32 index)
{
    return cnfgArray.at(index);
}

qint32 PayloadLib :: getcnfgTableIndex(qint32 tableNumber)
{
    return m_configTableIndex[tableNumber];
}

void PayloadLib :: setcnfgTableIndex(qint32 tableIndex)
{
    cnfgTableIndex = tableIndex;
}

qint32 PayloadLib::getTotalCmdFields()
{
    return m_totalCmdFields;
}

qint32 PayloadLib::getTotalReplyFields()
{
    return m_totalReplyFields;
}

qint32 PayloadLib::getTotalConfigFieldOfTable(qint32 tableIndex)
{
    return m_configRecordForTable[tableIndex];
}

void PayloadLib::clearPayloadLib()
{
    cnfgArray.clear();
}
