#include "AdvanceCameraSearch.h"
#include <QPainter>
#include <QKeyEvent>
#include "ValidationMessage.h"

#define ADVC_CAM_SRCH_BGWIDTH        BGTILE_ULTRAMEDIUM_SIZE_WIDTH
#define ADVC_CAM_SRCH_WIDTH          (ADVC_CAM_SRCH_BGWIDTH + SCALE_WIDTH(40))
#define ADVC_CAM_SRCH_HEIGHT         SCALE_HEIGHT(330)
#define ADVC_ANY_STR                 "Any"
#define LEFT_MARGIN_FROM_CENTER      SCALE_WIDTH(165)

static const QString advCamSrchStr[] =
{
    "Advance Camera Search",
    "IP Address Range",
    "to",
    "Camera Brand",
    "HTTP Port",
    "Username",
    "Password",
    "Search",
    "Cancel"
};

AdvanceCameraSearch::AdvanceCameraSearch(QString ipAddr1, QString ipAddr2, QString httpPort, PayloadLib *payloadlib, QWidget *parent)
    : KeyBoard(parent)
{
    this->setGeometry (0, 0, parent->width(), parent->height());
    payloadLib = payloadlib;
    createDefaultElements();
    m_ipAddressRangeBox1->setIpaddress (ipAddr1);
    m_ipAddressRangeBox2->setIpaddress (ipAddr2);
    m_httpPortTextBox->setInputText (httpPort);
    this->show ();
}

AdvanceCameraSearch::~AdvanceCameraSearch ()
{
    delete m_backGround;
    disconnect (m_closeButton,
                SIGNAL(sigButtonClick(int)),
                this,
                SLOT(slotButtonClick(int)));
    disconnect (m_closeButton,
                SIGNAL(sigUpdateCurrentElement(int)),
                this,
                SLOT(slotUpdateCurrentElement(int)));
    delete m_closeButton;

    delete m_heading;
    disconnect(m_ipAddressRangeBox1,
               SIGNAL(sigUpdateCurrentElement(int)),
               this,
               SLOT(slotUpdateCurrentElement(int)));
    disconnect (m_ipAddressRangeBox1,
                SIGNAL(sigEntryDone(quint32)),
                this,
                SLOT(slotIpStartDone(quint32)));
    disconnect (m_ipAddressRangeBox1,
                SIGNAL(sigLoadInfopage(quint32)),
                this,
                SLOT(slotIpTextLoadInfoPage(quint32)));
    delete m_ipAddressRangeBox1;

    disconnect(m_ipAddressRangeBox2,
               SIGNAL(sigUpdateCurrentElement(int)),
               this,
               SLOT(slotUpdateCurrentElement(int)));
    disconnect (m_ipAddressRangeBox2,
             SIGNAL(sigEntryDone(quint32)),
             this,
             SLOT(slotIpEndDone(quint32)));
    disconnect(m_ipAddressRangeBox2,
               SIGNAL(sigLoadInfopage(quint32)),
               this,
               SLOT(slotIpTextLoadInfoPage(quint32)));
    delete m_ipAddressRangeBox2;

    disconnect(m_cameraBrandNamePicklist,
               SIGNAL(sigUpdateCurrentElement(int)),
               this,
               SLOT(slotUpdateCurrentElement(int)));
    disconnect(m_cameraBrandNamePicklist,
               SIGNAL(sigButtonClick(int)),
               this,
               SLOT(slotButtonClick(int)));
    disconnect(m_cameraBrandNamePicklist,
               SIGNAL(sigValueChanged(quint8,QString,int)),
               this,
               SLOT(slotValueChanged(quint8,QString,int)));
    delete m_cameraBrandNamePicklist;

    disconnect(m_httpPortTextBox,
               SIGNAL(sigUpdateCurrentElement(int)),
               this,
               SLOT(slotUpdateCurrentElement(int)));
    delete m_httpPortTextBox;
    delete m_httpPortTextBoxParam;

    disconnect(m_usernameTextBox,
               SIGNAL(sigUpdateCurrentElement(int)),
               this,
               SLOT(slotUpdateCurrentElement(int)));
    delete m_usernameTextBox;
    delete m_usernameTextBoxParam;

    disconnect(m_passwordTextBox,
               SIGNAL(sigUpdateCurrentElement(int)),
               this,
               SLOT(slotUpdateCurrentElement(int)));
    delete m_passwordTextBox;
    delete m_passwordTextBoxParam;

    disconnect (m_searchBtn,
                SIGNAL(sigButtonClick(int)),
                this,
                SLOT(slotButtonClick(int)));
    disconnect (m_searchBtn,
                SIGNAL(sigUpdateCurrentElement(int)),
                this,
                SLOT(slotUpdateCurrentElement(int)));
    delete m_searchBtn;

    disconnect (m_cancleBtn,
                SIGNAL(sigButtonClick(int)),
                this,
                SLOT(slotButtonClick(int)));
    disconnect (m_cancleBtn,
                SIGNAL(sigUpdateCurrentElement(int)),
                this,
                SLOT(slotUpdateCurrentElement(int)));
    delete m_cancleBtn;

    disconnect (infoPage,
                SIGNAL(sigInfoPageCnfgBtnClick(int)),
                this,
                SLOT(slotInfoPageBtnclick(int)));
    delete infoPage;
}

void AdvanceCameraSearch::paintEvent (QPaintEvent *)
{
    QPainter painter(this);
    QColor color;

    color.setAlpha (0);
    painter.setBrush (QBrush(color));
    painter.setPen (Qt::NoPen);

    painter.drawRoundedRect (QRect(0,
                                   0,
                                   SCALE_WIDTH(SETTING_LEFT_PANEL_WIDTH),
                                   SCALE_HEIGHT(SETTING_LEFT_PANEL_HEIGHT)),
                                   SCALE_WIDTH(RECT_RADIUS), SCALE_HEIGHT(RECT_RADIUS));

    color.setAlpha (150);
    painter.setBrush (QBrush(color));
    painter.setPen (Qt::NoPen);

    painter.drawRoundedRect (QRect(SCALE_WIDTH(SETTING_LEFT_PANEL_WIDTH),
                                   SCALE_HEIGHT(SETTING_LEFT_PANEL_HEIGHT) - SCALE_HEIGHT(SETTING_RIGHT_PANEL_HEIGHT),
                                   SCALE_WIDTH(SETTING_RIGHT_PANEL_WIDTH),
                                   SCALE_HEIGHT(SETTING_RIGHT_PANEL_HEIGHT)),
                                   SCALE_WIDTH(RECT_RADIUS), SCALE_HEIGHT(RECT_RADIUS));
}

void AdvanceCameraSearch:: createDefaultElements ()
{
    m_backGround = new Rectangle((SCALE_WIDTH(SETTING_LEFT_PANEL_WIDTH) + SCALE_WIDTH(20) + ((SCALE_WIDTH(SETTING_RIGHT_PANEL_WIDTH) - ADVC_CAM_SRCH_WIDTH) / 2)),
                                 (SCALE_HEIGHT(SETTING_LEFT_PANEL_HEIGHT) - SCALE_HEIGHT(SETTING_RIGHT_PANEL_HEIGHT) + ((SCALE_HEIGHT(SETTING_RIGHT_PANEL_HEIGHT)- ADVC_CAM_SRCH_HEIGHT) / 2)),
                                 ADVC_CAM_SRCH_WIDTH,
                                 ADVC_CAM_SRCH_HEIGHT,
                                 0,
                                 NORMAL_BKG_COLOR,
                                 NORMAL_BKG_COLOR,
                                 this);

    m_closeButton = new CloseButtton ((m_backGround->x () + m_backGround->width () - SCALE_WIDTH(20)),
                                      (m_backGround->y () + SCALE_HEIGHT(30)),
                                      this,
                                      CLOSE_BTN_TYPE_1,
                                      ADVNC_SERCH_CAM_CLS_BUTN);

    m_elementlist[ADVNC_SERCH_CAM_CLS_BUTN] = m_closeButton;

    connect (m_closeButton,
             SIGNAL(sigButtonClick(int)),
             this,
             SLOT(slotButtonClick(int)));

    connect (m_closeButton,
             SIGNAL(sigUpdateCurrentElement(int)),
             this,
             SLOT(slotUpdateCurrentElement(int)));


    m_heading = new Heading((m_backGround->x () + (m_backGround->width () / 2)),
                            (m_backGround->y () + SCALE_HEIGHT(30)),
                            advCamSrchStr[0],
                            this,
                            HEADING_TYPE_2);

    m_ipAddressRangeBox1 = new Ipv4TextBox(m_backGround->x () + SCALE_WIDTH(20),
                                           m_backGround->y () + SCALE_HEIGHT(60),
                                           ADVC_CAM_SRCH_BGWIDTH,
                                           BGTILE_HEIGHT,
                                           ADVNC_SERCH_CAM_IP_RANGE1_TXTBOX,
                                           advCamSrchStr[ADVNC_SERCH_CAM_IP_RANGE1_TXTBOX],
                                           this,
                                           COMMON_LAYER,
                                           true, 0, true, false,
                                           LEFT_MARGIN_FROM_CENTER);

    m_elementlist[ADVNC_SERCH_CAM_IP_RANGE1_TXTBOX] = m_ipAddressRangeBox1;
    connect(m_ipAddressRangeBox1,
            SIGNAL(sigUpdateCurrentElement(int)),
            this,
            SLOT(slotUpdateCurrentElement(int)));
    connect (m_ipAddressRangeBox1,
             SIGNAL(sigEntryDone(quint32)),
             this,
             SLOT(slotIpStartDone(quint32)));
    connect (m_ipAddressRangeBox1,
             SIGNAL(sigLoadInfopage(quint32)),
             this,
             SLOT(slotIpTextLoadInfoPage(quint32)));

    m_ipAddressRangeBox2 = new Ipv4TextBox(m_ipAddressRangeBox1->x () + SCALE_WIDTH(358),
                                           m_ipAddressRangeBox1->y (),
                                           ADVC_CAM_SRCH_BGWIDTH,
                                           BGTILE_HEIGHT,
                                           ADVNC_SERCH_CAM_IP_RANGE2_TXTBOX,
                                           advCamSrchStr[ADVNC_SERCH_CAM_IP_RANGE2_TXTBOX],
                                           this,
                                           NO_LAYER,
                                           false,
                                           SCALE_WIDTH(18));
    m_elementlist[ADVNC_SERCH_CAM_IP_RANGE2_TXTBOX] = m_ipAddressRangeBox2;
    connect(m_ipAddressRangeBox2,
            SIGNAL(sigUpdateCurrentElement(int)),
            this,
            SLOT(slotUpdateCurrentElement(int)));
    connect (m_ipAddressRangeBox2,
             SIGNAL(sigEntryDone(quint32)),
             this,
             SLOT(slotIpEndDone(quint32)));
    connect (m_ipAddressRangeBox2,
             SIGNAL(sigLoadInfopage(quint32)),
             this,
             SLOT(slotIpTextLoadInfoPage(quint32)));

    QMap<quint8, QString> brandNameList;
    brandNameList.clear ();
    brandNameList.insert (0,ADVC_ANY_STR);
    m_cameraBrandNamePicklist = new PickList(m_ipAddressRangeBox1->x() ,
                                             m_ipAddressRangeBox2->y() +
                                             m_ipAddressRangeBox2->height (),
                                             ADVC_CAM_SRCH_BGWIDTH,
                                             BGTILE_HEIGHT,
                                             SCALE_WIDTH(180), SCALE_HEIGHT(30),
                                             advCamSrchStr[ADVNC_SERCH_CAM_BRAND_PICKLIST],
                                             brandNameList, 0,
                                             "Select Brand",
                                             this, COMMON_LAYER, -1,
                                             ADVNC_SERCH_CAM_BRAND_PICKLIST,
                                             true, true, true,
                                             LEFT_MARGIN_FROM_CENTER);

    m_elementlist[ADVNC_SERCH_CAM_BRAND_PICKLIST] = m_cameraBrandNamePicklist;
    connect(m_cameraBrandNamePicklist,
            SIGNAL(sigUpdateCurrentElement(int)),
            this,
            SLOT(slotUpdateCurrentElement(int)));
    connect(m_cameraBrandNamePicklist,
            SIGNAL(sigButtonClick(int)),
            this,
            SLOT(slotButtonClick(int)));
    connect(m_cameraBrandNamePicklist,
            SIGNAL(sigValueChanged(quint8,QString,int)),
            this,
            SLOT(slotValueChanged(quint8,QString,int)));

    m_httpPortTextBoxParam = new TextboxParam();
    m_httpPortTextBoxParam->labelStr = advCamSrchStr[ADVNC_SERCH_CAM_HTTP_TXTBOX];
    m_httpPortTextBoxParam->suffixStr = "(1-65535)";
    m_httpPortTextBoxParam->isNumEntry = true;
    m_httpPortTextBoxParam->minNumValue = 1;
    m_httpPortTextBoxParam->maxNumValue = 65535;
    m_httpPortTextBoxParam->maxChar = 5;

    m_httpPortTextBox = new TextBox(m_cameraBrandNamePicklist->x (),
                                    (m_cameraBrandNamePicklist->y() +
                                     m_cameraBrandNamePicklist->height()),
                                    ADVC_CAM_SRCH_BGWIDTH,
                                    BGTILE_HEIGHT,
                                    ADVNC_SERCH_CAM_HTTP_TXTBOX,
                                    TEXTBOX_SMALL,
                                    this,
                                    m_httpPortTextBoxParam,
                                    COMMON_LAYER,
                                    false, false, false,
                                    LEFT_MARGIN_FROM_CENTER);
    m_elementlist[ADVNC_SERCH_CAM_HTTP_TXTBOX] = m_httpPortTextBox;
    connect(m_httpPortTextBox,
            SIGNAL(sigUpdateCurrentElement(int)),
            this,
            SLOT(slotUpdateCurrentElement(int)));

    m_usernameTextBoxParam = new TextboxParam();
    m_usernameTextBoxParam->maxChar = 24;
    m_usernameTextBoxParam->labelStr = advCamSrchStr[ADVNC_SERCH_CAM_USERNAME_TXTBOX];

    m_usernameTextBox =  new TextBox(m_httpPortTextBox->x(),
                                     m_httpPortTextBox->y () + m_httpPortTextBox->height (),
                                     ADVC_CAM_SRCH_BGWIDTH,
                                     BGTILE_HEIGHT,
                                     ADVNC_SERCH_CAM_USERNAME_TXTBOX,
                                     TEXTBOX_LARGE,
                                     this,
                                     m_usernameTextBoxParam,
                                     COMMON_LAYER,
                                     false, false, false,
                                     LEFT_MARGIN_FROM_CENTER);
    m_elementlist[ADVNC_SERCH_CAM_USERNAME_TXTBOX] = m_usernameTextBox;
    connect(m_usernameTextBox,
            SIGNAL(sigUpdateCurrentElement(int)),
            this,
            SLOT(slotUpdateCurrentElement(int)));

    m_passwordTextBoxParam = new TextboxParam();
    m_passwordTextBoxParam->maxChar = 20;
    m_passwordTextBoxParam->labelStr = advCamSrchStr[ADVNC_SERCH_CAM_PASSWORD_TXTBOX];
    m_passwordTextBoxParam->suffixStr = "(Max 20 chars)";

    m_passwordTextBox = new PasswordTextbox(m_usernameTextBox->x (),
                                            m_usernameTextBox->y () + m_usernameTextBox->height (),
                                            ADVC_CAM_SRCH_BGWIDTH,
                                            BGTILE_HEIGHT,
                                            ADVNC_SERCH_CAM_PASSWORD_TXTBOX,
                                            TEXTBOX_LARGE,
                                            this,
                                            m_passwordTextBoxParam,
                                            COMMON_LAYER,
                                            false, LEFT_MARGIN_FROM_CENTER);
    m_elementlist[ADVNC_SERCH_CAM_PASSWORD_TXTBOX] = m_passwordTextBox;
    connect(m_passwordTextBox,
            SIGNAL(sigUpdateCurrentElement(int)),
            this,
            SLOT(slotUpdateCurrentElement(int)));

    m_searchBtn = new CnfgButton(CNFGBUTTON_MEDIAM,
                                 m_passwordTextBox->x () + m_passwordTextBox->width ()/2 - SCALE_WIDTH(70),
                                 m_passwordTextBox->y () +
                                 m_passwordTextBox->height () + SCALE_HEIGHT(30),
                                 advCamSrchStr[ADVNC_SERCH_CAM_SEARCH_BUTN],
                                 this,
                                 ADVNC_SERCH_CAM_SEARCH_BUTN);
    m_elementlist[ADVNC_SERCH_CAM_SEARCH_BUTN] = m_searchBtn;
    connect (m_searchBtn,
             SIGNAL(sigButtonClick(int)),
             this,
             SLOT(slotButtonClick(int)));
    connect (m_searchBtn,
             SIGNAL(sigUpdateCurrentElement(int)),
             this,
             SLOT(slotUpdateCurrentElement(int)));

    m_cancleBtn = new CnfgButton(CNFGBUTTON_MEDIAM,
                                 m_passwordTextBox->x () + m_passwordTextBox->width ()/2 + SCALE_WIDTH(70),
                                 m_passwordTextBox->y () +
                                 m_passwordTextBox->height () + SCALE_HEIGHT(30),
                                 advCamSrchStr[ADVNC_SERCH_CAM_CANCEL_BUTN],
                                 this,
                                 ADVNC_SERCH_CAM_CANCEL_BUTN);
    m_elementlist[ADVNC_SERCH_CAM_CANCEL_BUTN] = m_cancleBtn;
    connect (m_cancleBtn,
             SIGNAL(sigButtonClick(int)),
             this,
             SLOT(slotButtonClick(int)));
    connect (m_cancleBtn,
             SIGNAL(sigUpdateCurrentElement(int)),
             this,
             SLOT(slotUpdateCurrentElement(int)));

    infoPage = new InfoPage (0, 0,
                             SCALE_WIDTH(SETTING_RIGHT_PANEL_WIDTH) + SCALE_WIDTH(SETTING_LEFT_PANEL_WIDTH),
                             SCALE_HEIGHT(SETTING_LEFT_PANEL_HEIGHT),
                             INFO_CONFIG_PAGE,
                             parentWidget());
    connect (infoPage,
             SIGNAL(sigInfoPageCnfgBtnClick(int)),
             this,
             SLOT(slotInfoPageBtnclick(int)));

    m_currElement = ADVNC_SERCH_CAM_IP_RANGE1_TXTBOX;
    m_elementlist[m_currElement]->forceActiveFocus ();
}

void AdvanceCameraSearch::processDeviceResponse (DevCommParam *param, QString)
{
    QMap<quint8, QString> brandNameList;

    if (param->msgType != MSG_SET_CMD)
    {
        return;
    }

    payloadLib->parseDevCmdReply(true, param->payload);
    switch(param->cmdType)
    {
        case BRND_NAME:
        {
            brandNameList.insert(0,ADVC_ANY_STR);
            for (quint8 index = 1; index < payloadLib->getTotalCmdFields(); index++)
            {
                brandNameList.insert(index, payloadLib->getCnfgArrayAtIndex(index).toString ());
            }
            m_cameraBrandNamePicklist->loadPickListOnResponse(brandNameList);
        }
        break;

        case GET_USER_DETAIL:
        {
            m_usernameTextBox->setInputText(payloadLib->getCnfgArrayAtIndex(0).toString());
            m_passwordTextBox->setInputText(payloadLib->getCnfgArrayAtIndex(1).toString());
        }
        break;

        default:
        {
            /* Nothing to do */
        }
        break;
    }
}

bool AdvanceCameraSearch::validationDone ()
{
    if(m_ipAddressRangeBox1->getIpaddress () == "")
    {
        infoPage->loadInfoPage (ValidationMessage::getValidationMessage(ENT_IP_ADDR_RANGE));
        return false;
    }

    if(m_ipAddressRangeBox2->getIpaddress () == "")
    {
        infoPage->loadInfoPage (ValidationMessage::getValidationMessage(ENT_IP_ADDR_RANGE));
        return false;
    }

    if(m_ipAddressRangeBox1->getIpaddress () == m_ipAddressRangeBox2->getIpaddress ())
    {
        infoPage->loadInfoPage (ValidationMessage::getValidationMessage(ADV_CAM_SEARCH_STR_END_DIFF));
        return false;
    }

    if(m_ipAddressRangeBox1->getSubnetOfIpaddress () != m_ipAddressRangeBox2->getSubnetOfIpaddress ())
    {
        infoPage->loadInfoPage (ValidationMessage::getValidationMessage(ADV_CAM_SEARCH_STR_END_SAME));
        return false;
    }

    if(m_ipAddressRangeBox1->getlastOctalOfIpaddress ().toUInt () > m_ipAddressRangeBox2->getlastOctalOfIpaddress ().toUInt () )
    {
        infoPage->loadInfoPage (ValidationMessage::getValidationMessage(START_IP_ADDR_LESS_END_IP_ADDR));
        return false;
    }

    if((m_cameraBrandNamePicklist->getCurrentPickStr () != ADVC_ANY_STR) && (m_httpPortTextBox->getInputText () == ""))
    {
        infoPage->loadInfoPage (ValidationMessage::getValidationMessage(HTTP_PORT_RANGE));
        return false;
    }

    if((m_cameraBrandNamePicklist->getCurrentPickStr () != ADVC_ANY_STR) && (m_usernameTextBox->getInputText () == ""))
    {
        infoPage->loadInfoPage (ValidationMessage::getValidationMessage(ENT_USER_NM));
        return false;
    }

    if((m_cameraBrandNamePicklist->getCurrentPickStr () != ADVC_ANY_STR) && (m_passwordTextBox->getInputText () == ""))
    {
        infoPage->loadInfoPage (ValidationMessage::getValidationMessage(ENT_PASSWORD));
        return false;
    }

    QString brandStr = m_cameraBrandNamePicklist->getCurrentPickStr() == ADVC_ANY_STR ? "GENERIC" : m_cameraBrandNamePicklist->getCurrentPickStr();
    payloadLib->setCnfgArrayAtIndex(0, m_ipAddressRangeBox1->getIpaddress());
    payloadLib->setCnfgArrayAtIndex(1, m_ipAddressRangeBox2->getIpaddress());
    payloadLib->setCnfgArrayAtIndex(2, brandStr);
    payloadLib->setCnfgArrayAtIndex(3, m_httpPortTextBox->getInputText ());
    payloadLib->setCnfgArrayAtIndex(4, m_usernameTextBox->getInputText ());
    payloadLib->setCnfgArrayAtIndex(5, m_passwordTextBox->getInputText ());
    return true;
}

void AdvanceCameraSearch::takeLeftKeyAction()
{
    do
    {
        m_currElement = (m_currElement - 1 + MAX_ADVNC_SERCH_CTRL) % MAX_ADVNC_SERCH_CTRL;
    }while(!m_elementlist[m_currElement]->getIsEnabled());

    m_elementlist[m_currElement]->forceActiveFocus();
}

void AdvanceCameraSearch::takeRightKeyAction()
{
    do
    {
        m_currElement = (m_currElement + 1) % MAX_ADVNC_SERCH_CTRL;
    }while(!m_elementlist[m_currElement]->getIsEnabled());

    m_elementlist[m_currElement]->forceActiveFocus();
}

void AdvanceCameraSearch::showEvent (QShowEvent *event)
{
    QWidget::showEvent (event);
    if(m_elementlist[m_currElement] != NULL)
    {
        m_elementlist[m_currElement]->forceActiveFocus ();
    }
}

void AdvanceCameraSearch::backTab_KeyPressed(QKeyEvent *event)
{
    event->accept();
    takeLeftKeyAction();
}

void AdvanceCameraSearch::tabKeyPressed(QKeyEvent *event)
{
    event->accept();
    takeRightKeyAction();
}

void AdvanceCameraSearch::navigationKeyPressed(QKeyEvent *event)
{
    event->accept();
}

void AdvanceCameraSearch::escKeyPressed(QKeyEvent *event)
{
    event->accept();
    m_currElement = ADVNC_SERCH_CAM_CLS_BUTN;
    m_elementlist[m_currElement]->forceActiveFocus ();
}

void AdvanceCameraSearch::slotButtonClick (int indexInPage)
{
    switch(indexInPage)
    {
        case ADVNC_SERCH_CAM_BRAND_PICKLIST:
        {
            emit sigCreateCMDRequest(BRND_NAME,0);
        }
        break;

        case ADVNC_SERCH_CAM_SEARCH_BUTN:
        {
            if(validationDone())
            {
                emit sigAdvanceSearchRequest (m_ipAddressRangeBox1->getIpaddress(), m_ipAddressRangeBox2->getIpaddress(), m_httpPortTextBox->getInputText());
                emit sigCreateCMDRequest(ADV_CAM_SEARCH, 7);
                emit sigObjectDelete (true);
            }
        }
        break;

        default:
        {
            emit sigObjectDelete (false);
        }
        break;
    }
}

void AdvanceCameraSearch::slotValueChanged(quint8,QString value,int indexInPage)
{
    if((indexInPage == ADVNC_SERCH_CAM_BRAND_PICKLIST) && (value != ""))
    {
        if(value == ADVC_ANY_STR)
        {
            m_usernameTextBox->setIsEnabled (false);
            m_passwordTextBox->setIsEnabled (false);
            m_httpPortTextBox->setIsEnabled (false);
            m_usernameTextBox->setInputText ("");
            m_passwordTextBox->setInputText ("");
        }
        else
        {
            m_usernameTextBox->setIsEnabled (true);
            m_passwordTextBox->setIsEnabled (true);
            m_httpPortTextBox->setIsEnabled (true);
            payloadLib->setCnfgArrayAtIndex(0, m_cameraBrandNamePicklist->getCurrentPickStr());
            emit sigCreateCMDRequest(GET_USER_DETAIL, 1);
        }
    }
}

void AdvanceCameraSearch::slotUpdateCurrentElement (int indexInPage)
{
    m_currElement = indexInPage;
}

void AdvanceCameraSearch::slotInfoPageBtnclick (int)
{
    m_elementlist[m_currElement]->forceActiveFocus ();
}

void AdvanceCameraSearch::slotIpStartDone (quint32)
{
    QString ipAddr2 = m_ipAddressRangeBox1->getSubnetOfIpaddress ();

    if(m_ipAddressRangeBox1->getlastOctalOfIpaddress () == "0")
    {
        ipAddr2.append (".").append ("1");
        m_ipAddressRangeBox1->setIpaddress (ipAddr2);
        ipAddr2 = m_ipAddressRangeBox1->getSubnetOfIpaddress ();
    }

    ipAddr2.append (".").append(m_ipAddressRangeBox2->getlastOctalOfIpaddress());
    m_ipAddressRangeBox2->setIpaddress (ipAddr2);
}

void AdvanceCameraSearch::slotIpEndDone (quint32)
{
    QString ipAddr2 = m_ipAddressRangeBox2->getSubnetOfIpaddress ();

    if(m_ipAddressRangeBox2->getlastOctalOfIpaddress () == "0")
    {
        ipAddr2.append (".").append ("1");
        m_ipAddressRangeBox2->setIpaddress (ipAddr2);
    }
}

void AdvanceCameraSearch::slotIpTextLoadInfoPage(quint32)
{
    infoPage->loadInfoPage (ValidationMessage::getValidationMessage(ENT_IP_ADDR_RANGE));
}

void AdvanceCameraSearch::insertKeyPressed(QKeyEvent *event)
{
    event->accept();
}
