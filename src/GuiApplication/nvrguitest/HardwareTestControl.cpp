#include "HardwareTestControl.h"

#define PLAY_IMAGE_PATH     IMAGE_PATH"/PlayBtn/"
#define STOP_IMAGE_PATH     IMAGE_PATH"/StopBtn/"

bool HardwareTestControl::exitTest=true;
QString HardwareTestControl::ntpServerIpAddr="192.168.1.100";
QString HardwareTestControl::ftpServerIpAddr="192.168.1.100";
QString HardwareTestControl::ftpPort="21";
QString HardwareTestControl::ftpUsername="matrix";
QString HardwareTestControl::ftpPassword="matrix";
QString HardwareTestControl::deviceSerialNumber="";
QString HardwareTestControl::testEmpId="";
QString HardwareTestControl::macAdd1="";
QString HardwareTestControl::macAdd2="";
quint8 HardwareTestControl::reTestCount=0;
QString HardwareTestControl::ntpDate="";
QString HardwareTestControl::ntpTime="";
TEST_RESULT_e HardwareTestControl::testResult[];
QString HardwareTestControl::boardTypeString="";
struct tm HardwareTestControl::testStartTime;
QString HardwareTestControl::testStartEndDiff;


HardwareTestControl::HardwareTestControl(CONDUNCT_TEST_e hwIndex,QWidget *parent, quint8 numOfSubDev) :
   QWidget(parent), hwType(hwIndex), m_testHeader(NULL), numOfSubdevices(numOfSubDev), m_isCtrlBtnNeeded(false), m_isPlayBtnNeeded(false)
{
    quint8 subDeviceCnt;
    for(subDeviceCnt=0;subDeviceCnt<MAX_SUB_DEVICE;subDeviceCnt++)
    {
        testCondunctResult[subDeviceCnt] = MAX_HW_TEST_RESULT;
    }

    for(quint8 row = 0; row < MAX_ROWS; row++)
    {
        for(quint8 col = 0; col < MAX_COLUMNS; col++)
        {
            INIT_OBJ(m_statusIndicator[row][col]);
            INIT_OBJ(m_statusLable[row][col]);
            INIT_OBJ(m_playBtn[row][col]);
            INIT_OBJ(m_failBtn[row][col]);
            m_playBtnState[row][col] = PLAY_STATE;
        }
    }
}

HardwareTestControl::~HardwareTestControl()
{
    DELETE_OBJ(m_testHeader);

    for(quint8 row = 0; row < MAX_ROWS; row++)
    {
        for(quint8 col = 0; col < MAX_COLUMNS; col++)
        {
            DELETE_OBJ(m_statusIndicator[row][col]);
            DELETE_OBJ(m_statusLable[row][col]);
            DELETE_OBJ(m_playBtn[row][col]);            
            DELETE_OBJ(m_failBtn[row][col]);
        }
    }
}

void HardwareTestControl::createDefaultComponent(quint16 xPos, quint16 yPos,
                                                 quint8 totalRow, quint8 totalCol,
                                                 bool isCtrlBtnNeeded,
                                                 bool isPlayBtnNeeded,
                                                 quint8 fontSize,
                                                 QStringList &lableList,
                                                 QString testHeader)
{    
    quint16 xParam, yParam;
    quint8 row = 0, col = 0;
    bool isTestHeader = ((testHeader == "") ? (false) : (true));
    quint16 headerXoffset = 0, headerYoffset = 0;

    m_totalElements = numOfSubdevices;
    m_isCtrlBtnNeeded = isCtrlBtnNeeded;
    m_isPlayBtnNeeded = isPlayBtnNeeded;

    m_totalColumns = totalCol;

    if(isTestHeader)
    {
        m_testHeader = new TextLabel(0,
                                     0,
                                     TEST_HEADING_FONT_SIZE,
                                     testHeader,
                                     this,
                                     QString("#757575"),
                                     NORMAL_FONT_FAMILY,
                                     ALIGN_START_X_START_Y);
        m_testHeader->SetBold(true);

        headerXoffset = 15;
        headerYoffset = m_testHeader->height() + 5;
    }


    for(quint8 loop = 0; loop < m_totalElements; loop++)
    {
        row = loop / totalCol;
        col = loop % totalCol;

        if(row == 0)
        {
            yParam = 10 + headerYoffset;
        }
        else
        {
            if(isCtrlBtnNeeded)
            {
                yParam = (m_failBtn[row-1][0]->y() + m_failBtn[row-1][0]->height() + 15);
            }
            else
            {
                yParam = (m_statusLable[row-1][0]->y() + m_statusLable[row-1][0]->height() + 15);
            }
        }

        if(col == 0)
        {
            xParam = 10 + headerXoffset;
        }
        else
        {
            if(isCtrlBtnNeeded)
            {
                xParam = (m_failBtn[0][col-1]->x() + m_failBtn[0][col-1]->width() + 30);
            }
            else if(isPlayBtnNeeded)
            {
                xParam = (m_playBtn[0][col-1]->x() + m_playBtn[0][col-1]->width() + 30);
            }
            else
            {
                xParam = (m_statusLable[0][col-1]->x() + m_statusLable[0][col-1]->width() + 30);
            }
        }

        m_statusIndicator[row][col] = new Rect(xParam,
                                               yParam,
                                               10,
                                               10,
                                               QString(HEADER_BG_COLOR),
                                               this);

        m_statusLable[row][col] = new TextLabel((m_statusIndicator[row][col]->x() + m_statusIndicator[row][col]->width() + 10),
                                                (m_statusIndicator[row][col]->y() + (m_statusIndicator[row][col]->height()/2)),
                                                (int)fontSize,
                                                lableList.at(loop),
                                                this,
                                                QString("#757575"),
                                                NORMAL_FONT_FAMILY,
                                                ALIGN_START_X_CENTRE_Y);
        if(fontSize == 16)
        {
            m_statusLable[row][col]->SetBold(true);
        }

        if(isPlayBtnNeeded)
        {
            m_playBtn[row][col] = new Image((m_statusLable[row][col]->x() + 150),
                                  (m_statusLable[row][col]->y() + (m_statusLable[row][col]->height()/2)),
                                  QString(PLAY_IMAGE_PATH),
                                  this,
                                  CENTER_X_CENTER_Y,
                                  PLAY_STOP_CLICKED,
                                  true);

            connect(m_playBtn[row][col],
                    SIGNAL(sigImageClicked(int)),
                    this,
                    SLOT(slotUIClicked(int)),
                    Qt::DirectConnection);

            connect(m_playBtn[row][col],
                    SIGNAL(sigImageClicked(int)),
                    this,
                    SLOT(slotImageClicked(int)));

            m_playBtnState[row][col] = PLAY_STATE;

        }

        if(isCtrlBtnNeeded)
        {
            m_failBtn[row][col] = new CnfgButton(CNFGBUTTON_MEDIAM,
                                                 (m_statusLable[row][col]->x() + 60),
                                                 (m_statusLable[row][col]->y() + m_statusLable[row][col]->height() + 35),
                                                 QString("Fail"),
                                                 this,
                                                 FAIL_CLICKED,
                                                 false);

            connect(m_failBtn[row][col],
                    SIGNAL(sigButtonClick(int)),
                    this,
                    SLOT(slotUIClicked(int)),
                    Qt::DirectConnection);
        }
    }

    int width, height;

    if(isCtrlBtnNeeded)
    {
        width = m_failBtn[row][col]->x() + (m_failBtn[row][col]->width()) - m_statusIndicator[row][0]->x() + 20 + headerXoffset;
        height = m_failBtn[row][col]->y() + m_failBtn[row][col]->height() - m_statusIndicator[0][col]->y() + 10 + headerYoffset;
    }
    else if(isPlayBtnNeeded)
    {
        width = m_playBtn[row][col]->x() + (m_playBtn[row][col]->width()) - m_statusIndicator[row][0]->x() + 20 + headerXoffset;
        height = m_statusLable[row][col]->y() + m_statusLable[row][col]->height() - m_statusIndicator[0][col]->y() + 10 + headerYoffset;
    }
    else
    {
        width =  m_statusLable[row][col]->x() + m_statusLable[row][col]->width() - m_statusIndicator[row][0]->x() + 20 + headerXoffset;
        height = m_statusLable[row][col]->y() + m_statusLable[row][col]->height() - m_statusIndicator[0][col]->y() + 10 + headerYoffset;

        if((isTestHeader) && (width < m_testHeader->width()))
        {
            width = m_testHeader->width() + 20;
        }
    }

    this->setGeometry((int)xPos, (int)yPos, width, height);
    Q_UNUSED(totalRow);
}


void HardwareTestControl::enabledisableControls(bool isEnable)
{
    for(quint8 loop = 0; loop < m_totalElements; loop++)
    {
        quint8 row,col;
        row = loop / m_totalColumns;
        col = loop % m_totalColumns;

        if(IS_VALID_OBJ(m_playBtn[row][col]))
        {
            m_playBtn[row][col]->setIsEnabled(isEnable);
        }
        if(IS_VALID_OBJ(m_failBtn[row][col]))
        {
            m_failBtn[row][col]->setIsEnabled(isEnable);
        }
    }
}

void HardwareTestControl::passFailTest(TEST_RESULT_e testResult, bool isApplyToIndicator)
{
    for(quint8 loop = 0; loop < m_totalElements; loop++)
    {
        quint8 row,col;
        row = loop / m_totalColumns;
        col = loop % m_totalColumns;

        if(loop < MAX_SUB_DEVICE)
        {
            testCondunctResult[loop] = (testResult == MAX_HW_TEST_RESULT) ? HW_TEST_FAIL : testResult;
        }

        if((isApplyToIndicator) && (IS_VALID_OBJ(m_statusIndicator[row][col])))
        {
            if(testResult == HW_TEST_PASS)
            {
                if(m_statusIndicator[row][col]->getBgColor() != RED_COLOR)
                {
                    m_statusIndicator[row][col]->changeBgColor(
                                QString((testResult != MAX_HW_TEST_RESULT) ? ((testResult == HW_TEST_PASS) ? (GREEN_COLOR) : (RED_COLOR))
                                                                           : (HEADER_BG_COLOR)));
                }
            }
            else
            {
                m_statusIndicator[row][col]->changeBgColor(
                            QString((testResult != MAX_HW_TEST_RESULT) ? ((testResult == HW_TEST_PASS) ? (GREEN_COLOR) : (RED_COLOR))
                                                                       : (HEADER_BG_COLOR)));
            }
        }
    }

    if((testResult == MAX_HW_TEST_RESULT) && (m_isCtrlBtnNeeded) && (IS_VALID_OBJ(m_failBtn[0][0])))
    {
        m_failBtn[0][0]->changeColor(DISABLE_FONT_COLOR);
        m_failBtn[0][0]->update();
    }
}

TEST_RESULT_e HardwareTestControl::getStatusOfHwTest()
{
    quint8 subTypeDevCnt;
    TEST_RESULT_e retVal = HW_TEST_PASS;
    TEST_RESULT_e testRetVal = HW_TEST_PASS;

    for(subTypeDevCnt = 0; subTypeDevCnt < numOfSubdevices; subTypeDevCnt++)
    {
        retVal = testCondunctResult[subTypeDevCnt];
        quint8 row, col;

        row = subTypeDevCnt / m_totalColumns;
        col = subTypeDevCnt % m_totalColumns;

        if (retVal != HW_TEST_PASS)
        {
            m_statusIndicator[row][col]->changeBgColor(QString(RED_COLOR));
        }
        else
        {
            m_statusIndicator[row][col]->changeBgColor(QString(GREEN_COLOR));
        }

        if (retVal != HW_TEST_PASS)
        {
            testRetVal = HW_TEST_FAIL;
        }
    }

    return testRetVal;
}

void HardwareTestControl::slotUIClicked(int ctrlBtnIndex)
{
    if(ctrlBtnIndex == FAIL_CLICKED)
    {
        if(IS_VALID_OBJ(m_failBtn[0][0]))
        {
            m_failBtn[0][0]->setIsEnabled(false);
            m_failBtn[0][0]->changeColor(QString(RED_COLOR));
            m_failBtn[0][0]->update();
        }
        passFailTest(HW_TEST_FAIL, true);
    }
    emit sigCtrlBtnClicked((int)hwType, ctrlBtnIndex);
}

void HardwareTestControl::slotImageClicked(int indexInPage)
{
    quint8 row, col;
    row = indexInPage / m_totalColumns;
    col = indexInPage % m_totalColumns;

    if (m_playBtnState[row][col] == PLAY_STATE)
    {
        m_playBtnState[row][col] = STOP_STATE;
        m_playBtn[row][col]->updateImageSource(STOP_IMAGE_PATH, true);
    }
    else
    {
        m_playBtnState[row][col] = PLAY_STATE;
        m_playBtn[row][col]->updateImageSource(PLAY_IMAGE_PATH, true);
    }
}
