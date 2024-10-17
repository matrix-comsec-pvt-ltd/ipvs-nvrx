#include "InfoPage.h"

#include <QPainter>
#include <QPaintEvent>
#include <QKeyEvent>
#include <QMouseEvent>

#define INFO_PAGE_WIDTH         SCALE_WIDTH(449)
#define INFO_PAGE_HEIGHT        SCALE_HEIGHT(225)
#define WARNING_IMG_PATH        ":/Images_Nvrx/Warning.png"

static QString imageTypeString[] = {
    "AdvancedDetails.png",
    "ConfigSettings.png",
    "EventLog.png",
    "LiveView.png",
    "Manage.png",
    "PlaybackSearchInfopage.png",
    "PresetTourSchedule.png",
    "Loginpage.png",
};

InfoPage::InfoPage(quint16 startX,
                   quint16 startY,
                   quint16 width,
                   quint16 height,
                   INFO_PAGE_TYPE_e infoPageType,
                   QWidget *parent,
                   bool canceBtnNeed,
                   bool isBackGroundVisible,
                   quint32 msgWidthMax) :
    KeyBoard(parent), m_startX(startX), m_startY(startY), m_height(height),
    m_width(width),   truncateWidth(msgWidthMax), m_isBackGroundVisible(isBackGroundVisible), label(""), isCancelBtnIn(canceBtnNeed),
    infoPageImage(NULL), m_firstBtnStr(CONFORMATION_BTN_OK), m_secondBtnStr(CONFORMATION_BTN_CANCEL)
{
    if (infoPageType == MAX_INFO_PAGE_TYPE)
    {
        /* Avoid invalid image type access for invalid index */
        m_isBackGroundVisible = false;
    }

    if(m_isBackGroundVisible)
    {
        imageSource = QString(IMAGE_PATH) + QString("InfoPageImages/") + imageTypeString[infoPageType];
    }

    bgType = BACKGROUND_TYPE_1;
    INIT_OBJ(m_backGround);

    createDefaultComponent ();

    m_currElement = INFO_OK_BTN;
    m_elementList[m_currElement]->forceActiveFocus();

    QWidget::setVisible(false);
}

InfoPage::~InfoPage()
{
    destroyDefaultComponent();
}

void InfoPage::createDefaultComponent ()
{
    this->setGeometry(m_startX, m_startY, m_width, m_height);

    if((m_isBackGroundVisible == true) && (infoPageImage == NULL))
    {
        infoPageImage = new Image (0,0,
                                   imageSource,
                                   this,
                                   START_X_START_Y,
                                   0,
                                   false,
                                   true);
    }

    bgRect = new BackGround(((this->width() - INFO_PAGE_WIDTH) / 2),
                            ((this->height() - INFO_PAGE_HEIGHT) / 2),
                            INFO_PAGE_WIDTH,
                            INFO_PAGE_HEIGHT,
                            BACKGROUND_TYPE_4,
                            MAX_TOOLBAR_BUTTON,
                            this,
                            false);

    okButton = new CnfgButton(CNFGBUTTON_MEDIAM,
                              (bgRect->x () + (bgRect->width ()/2)),
                              (bgRect->y () + bgRect->height () - SCALE_HEIGHT(60)),
                              CONFORMATION_BTN_OK,
                              this,
                              INFO_OK_BTN);
    connect (okButton,
             SIGNAL(sigUpdateCurrentElement(int)),
             this,
             SLOT(slotUpdateCurrentElement(int)));

    connect (okButton,
             SIGNAL(sigButtonClick(int)),
             this,
             SLOT(slotCnfgBtnClicked(int)));
    m_elementList[INFO_OK_BTN] = okButton;

    cancelButton = new CnfgButton(CNFGBUTTON_MEDIAM,
                                  (bgRect->x () + (bgRect->width () / 2) + SCALE_WIDTH(60)),
                                  (bgRect->y () + bgRect->height () - SCALE_HEIGHT(60)),
                                  CONFORMATION_BTN_CANCEL,
                                  this,
                                  INFO_CANCEL_BTN);

    m_elementList[INFO_CANCEL_BTN] = cancelButton;
    cancelButton->setVisible (false);

    connect (cancelButton,
             SIGNAL(sigUpdateCurrentElement(int)),
             this,
             SLOT(slotUpdateCurrentElement(int)));

    connect (cancelButton,
             SIGNAL(sigButtonClick(int)),
             this,
             SLOT(slotCnfgBtnClicked(int)));

    labelText = new TextLabel((this->width() / 2),
                              (this->height() / 2 - SCALE_HEIGHT(20)),
                              NORMAL_FONT_SIZE,
                              label,
                              this,
                              NORMAL_FONT_COLOR,
                              NORMAL_FONT_FAMILY,
                              ALIGN_CENTRE_X_CENTER_Y);

    warningImage = new Image((bgRect->x() + SCALE_WIDTH(15)),
                             (bgRect->y() + bgRect->height() - SCALE_HEIGHT(35)),
                             QString(WARNING_IMG_PATH),
                             this, START_X_START_Y,
                             0, false, true);
    warningImage->setVisible(false);

    warningText = new TextLabel((warningImage->x() + warningImage->width() + SCALE_WIDTH(10)),
                                (bgRect->y() + bgRect->height () - SCALE_HEIGHT(35)),
                                SCALE_FONT(13), "", this, NORMAL_FONT_COLOR, NORMAL_FONT_FAMILY, ALIGN_START_X_START_Y,
                                0, false);
    warningText->setVisible(false);
}

void InfoPage::destroyDefaultComponent()
{
    disconnect (okButton,
                SIGNAL(sigButtonClick(int)),
                this,
                SLOT(slotCnfgBtnClicked(int)));
    disconnect (okButton,
                SIGNAL(sigUpdateCurrentElement(int)),
                this,
                SLOT(slotUpdateCurrentElement(int)));
    delete okButton;

    disconnect (cancelButton,
                SIGNAL(sigButtonClick(int)),
                this,
                SLOT(slotCnfgBtnClicked(int)));

    disconnect (cancelButton,
                SIGNAL(sigUpdateCurrentElement(int)),
                this,
                SLOT(slotUpdateCurrentElement(int)));
    delete cancelButton;

    delete bgRect;
    delete labelText;

    if((m_isBackGroundVisible == true) && (infoPageImage != NULL))
    {
        delete infoPageImage;
        infoPageImage = NULL;
    }

    delete warningText;
    delete warningImage;
}

void InfoPage::changeText(QString str)
{
	label = str;
    str = QApplication::translate(QT_TRANSLATE_STR, str.toUtf8().data());
    str = manipulateValidationMsg(str);
    labelText->changeText(str, truncateWidth);
    labelText->update();
}

QString InfoPage::manipulateValidationMsg(QString str)
{
    quint16     width;
    QString     tmpstr(str);
    QString     ourText;
    QStringList list;
    quint32     referenceWidth = 0;
    bool        isStrHaveNewLine = str.contains('\n');
    QFont       font = getFont(NORMAL_FONT_FAMILY, NORMAL_FONT_SIZE, false, false);

    list.clear();
    list = tmpstr.split(" ");

    for(quint8 index = 0; index < list.size(); index++)
    {
        QString temp = list.at(index);
        temp += QString(" ");
        width = QFontMetrics(font).width(temp);

        if(width > truncateWidth)
        {
            temp += QString("\n");
            ourText += temp;
            continue;
        }

        referenceWidth += width;
        if(referenceWidth > truncateWidth)
        {
            ourText += QString("\n");
            referenceWidth = width;
        }
        else if ((isStrHaveNewLine == true) && (temp.contains('\n') == true))
        {
            /* If string contains '\n' itself then reset the width calculation by considering string width after '\n' */
            referenceWidth = QFontMetrics(font).width(temp.section('\n', -1, -1).trimmed());
        }

        ourText += temp;
    }

    return ourText;

}

QFont InfoPage::getFont(QString fontFamily, quint8 fontSize, bool isBold, bool isSetUnderLine)
{
    float letterSpacing, wordSpacing;
    QFont font = QFont();

    font.setFamily(NORMAL_FONT_FAMILY);
    if(isBold == true)
        font.setBold (true);
    if(isSetUnderLine)
        font.setUnderline (true);

    if(fontFamily == NORMAL_FONT_FAMILY)
    {
        letterSpacing = 0.75;
        wordSpacing = SCALE_WIDTH(2);
        if(isBold == true)
            font.setWeight(QFont::Bold);
    }
    else
    {
        letterSpacing = 1;
        wordSpacing = SCALE_WIDTH(2);
        font.setWeight(QFont::Bold);
    }

    font.setLetterSpacing(QFont::AbsoluteSpacing, letterSpacing);
    font.setWordSpacing(wordSpacing);
    font.setPixelSize(fontSize);
    return font;
}

QString InfoPage::getText()
{
    return label;
}

void InfoPage::showCancelBtn(bool flag)
{
    if(flag == true)
    {
        okButton->resetGeometry((bgRect->x () + (bgRect->width () / 2)) - SCALE_WIDTH(60),
                                (bgRect->y () + bgRect->height () - SCALE_HEIGHT(60)));
    }
    else
    {
        okButton->resetGeometry((bgRect->x () + (bgRect->width () / 2)),
                                (bgRect->y () + bgRect->height () - SCALE_HEIGHT(60)));
    }
    cancelButton->setVisible(flag);
    cancelButton->setIsEnabled(flag);
}

void InfoPage::loadInfoPage(QString infoMsg, bool isCancelButton, bool isFocusToCancelButton, QString warningMsg,QString firstBtnStr, QString SecondBtnStr)
{
    if(infoMsg != "")
    {
        showCancelBtn(isCancelButton);
        changeText(infoMsg);
        if(isCancelButton && isFocusToCancelButton)
        {
            m_currElement = INFO_CANCEL_BTN;
        }
        else
        {
            m_currElement = INFO_OK_BTN;
        }

        if(warningMsg != "")
        {
            warningText->changeText(warningMsg);
            warningText->setVisible(true);
            warningImage->setVisible(true);
        }
        changeButtonsString(firstBtnStr,SecondBtnStr);
        m_elementList[m_currElement]->forceActiveFocus();
        setVisible(true);
    }
}

void InfoPage::loadInfoPageNoButton(QString infoMsg, bool isOkButton, bool isCancelButton)
{
    if(infoMsg != "")
    {
        showCancelBtn(isCancelButton);
        changeText(infoMsg);
        okButton->setVisible(isOkButton);
        okButton->setIsEnabled(isOkButton);
        setVisible(true);
    }
}

void InfoPage::unloadInfoPage()
{
    m_firstBtnStr = CONFORMATION_BTN_OK;
    m_secondBtnStr = CONFORMATION_BTN_CANCEL;
    setVisible(false);
    resetButtonString();
    if(warningText->isVisible())
    {
        warningText->changeText("");
        warningText->setVisible(false);
        warningImage->setVisible(false);
    }
}

void InfoPage::changeButtonsString(QString firstBtnStr, QString secondBtnStr)
{

    if(m_firstBtnStr != firstBtnStr)
    {
        m_firstBtnStr = firstBtnStr;
        okButton->changeText (firstBtnStr);
        okButton->repaint ();
    }

    if(m_secondBtnStr != secondBtnStr)
    {
        m_secondBtnStr = secondBtnStr;
        cancelButton->changeText (secondBtnStr);
        cancelButton->repaint ();
    }
}

void InfoPage::resetButtonString()
{
    okButton->changeText ("OK");
    okButton->update ();
    cancelButton->changeText ("Cancel");
    cancelButton->update ();
}

void InfoPage::takeLeftKeyAction()
{
    do
    {
        m_currElement = (m_currElement - 1 + MAX_INFO_ELEMENTS)
                % MAX_INFO_ELEMENTS;
    }while(!m_elementList[m_currElement]->getIsEnabled());
    m_elementList[m_currElement]->forceActiveFocus();
}


void InfoPage::takeRightKeyAction()
{
    do
    {
        m_currElement = (m_currElement + 1)
                % MAX_INFO_ELEMENTS;
    }while(!m_elementList[m_currElement]->getIsEnabled());

    m_elementList[m_currElement]->forceActiveFocus();
}

void InfoPage::updateGeometry()
{
    m_startX = ApplController::getXPosOfScreen();
    m_startY = ApplController::getYPosOfScreen();
    m_width = ApplController::getWidthOfScreen();
    m_height = ApplController::getHeightOfScreen();

    bool okBtnState = okButton->isVisible();
    QString okButtonStr = okButton->getText();
    bool cancelBtnState = cancelButton->isVisible();
    QString cancelButtonStr = cancelButton->getText();
    bool warningImageState = warningImage->isVisible();
    bool warningTextState = warningText->isVisible();
    QString warningTextStr = warningText->getText();
    QString lableTextStr = labelText->getText();

    destroyDefaultComponent();
    createDefaultComponent();

    okButton->changeText(okButtonStr);
    okButton->setVisible(okBtnState);
    cancelButton->changeText(cancelButtonStr);
    cancelButton->setVisible(cancelBtnState);
    warningText->changeText(warningTextStr);
    warningText->setVisible(warningTextState);
    warningImage->setVisible(warningImageState);
    labelText->changeText(lableTextStr);

    update();
}
void InfoPage::showEvent (QShowEvent *event)
{
    QWidget::showEvent (event);
    m_elementList[m_currElement]->forceActiveFocus ();
}

void InfoPage::forceActiveFocus()
{
    m_elementList[m_currElement]->forceActiveFocus ();
}

void InfoPage::navigationKeyPressed(QKeyEvent *event)
{
    event->accept();
}

void InfoPage::escKeyPressed(QKeyEvent *event)
{
    event->accept();
    unloadInfoPage();
    emit sigInfoPageCnfgBtnClick(INFO_CANCEL_BTN);
}

void InfoPage::mouseMoveEvent(QMouseEvent *event)
{
    event->accept();
}

void InfoPage::mousePressEvent(QMouseEvent *event)
{
    event->accept();
}

void InfoPage::slotUpdateCurrentElement(int index)
{
    m_currElement = index;
}

void InfoPage::slotCnfgBtnClicked(int index)
{
    // send sig of cnfg btn click
    unloadInfoPage();
    emit sigInfoPageCnfgBtnClick(index);
}

void InfoPage::backTab_KeyPressed(QKeyEvent *event)
{
    event->accept();
    takeLeftKeyAction ();
}

void InfoPage::tabKeyPressed(QKeyEvent *event)
{
    event->accept();
    takeRightKeyAction();
}

void InfoPage::insertKeyPressed(QKeyEvent *event)
{
    event->accept();
}
