#include "Ipv4TextBox.h"
#include <QMouseEvent>
#include <QPainter>

#define IPTEXTBOX_FOLDER_PATH               IMAGE_PATH"Textbox/Large/"
#define IP_SEPARATOR                        "."
#define SUBNETMASK_FIRSTOCTAL_MAX_VALUE     255
#define FIRST_OCTAL_MAX_VALUE               223
#define OCTAL_MAX_VALUE                     255
#define MAX_SUBNET_MASK_VALUES              32

static const QString subnetMaskList[MAX_SUBNET_MASK_VALUES] =
{
    "255.255.255.255",  "255.255.255.254",  "255.255.255.252",
    "255.255.255.248",  "255.255.255.240",  "255.255.255.224",
    "255.255.255.192",  "255.255.255.128",  "255.255.255.0",
    "255.255.254.0",    "255.255.252.0",    "255.255.248.0",
    "255.255.240.0",    "255.255.224.0",    "255.255.192.0",
    "255.255.128.0",    "255.255.0.0",      "255.254.0.0",
    "255.252.0.0",      "255.248.0.0",      "255.240.0.0",
    "255.224.0.0",      "255.192.0.0",      "255.128.0.0",
    "255.0.0.0",        "254.0.0.0",        "252.0.0.0",
    "248.0.0.0",        "240.0.0.0",        "224.0.0.0",
    "192.0.0.0",        "128.0.0.0"
};

Ipv4TextBox::Ipv4TextBox(quint32 startX,
                         quint32 startY,
                         quint32 width,
                         quint32 height,
                         quint16 controlIndex,
                         QString labelstr,
                         QWidget *parent,
                         BGTILE_TYPE_e bgType,
                         bool isBoxInCentre,
                         quint16 leftMarginVal,
                         bool isNavigationEnable,
                         bool isSubnetMask,
                         quint32 leftMarginFromCenter)
    : BgTile(startX, startY, width, height, bgType, parent),
      NavigationControl(controlIndex, isNavigationEnable),
      m_label(labelstr), m_isSubnetMask(isSubnetMask), m_isInCentre(isBoxInCentre), entryByRemote(false),
      editMode(false), isAllAddrEmpty(false), m_leftMargin(leftMarginVal), m_leftMarginFromCenter(leftMarginFromCenter),
      m_currSelectedOctal(0), virtualKeypad(NULL), currCurPos(0)
{
    INIT_OBJ(m_inVisibleWidget);
    INIT_OBJ(m_physicalKeyboardInVisibleWidget);

    m_currentImageType = (m_isEnabled) ? IMAGE_TYPE_NORMAL : IMAGE_TYPE_DISABLE;
    m_image = QPixmap(QString(IPTEXTBOX_FOLDER_PATH + imgTypePath[m_currentImageType]));
    SCALE_IMAGE(m_image);

    this->setEnabled(m_isEnabled);
    this->setMouseTracking(true);
    createDefaultComponent();

    this->show();
}

Ipv4TextBox::~Ipv4TextBox()
{
    slotInvisibleCtrlMouseClick();

    if(m_label != "")
    {
        delete m_labelTextLabel;
    }

    delete m_palette;

    for(quint8 index = 0; index < MAX_OCTAL; index++)
    {
        disconnect(m_octalLineEdit[index],
                   SIGNAL(sigFocusChange(quint8,bool,bool)),
                   this,
                   SLOT(slotLineEditFocusChange(quint8,bool,bool)));
        delete m_octalLineEdit[index];
    }

    for(quint8 index = 0; index < (MAX_OCTAL - 1); index++)
    {
        delete m_dotTextLabels[index];
    }

    /* PARASOFT: To make it happy */
    DELETE_OBJ(virtualKeypad);
    DELETE_OBJ(m_inVisibleWidget);
    DELETE_OBJ(m_physicalKeyboardInVisibleWidget);
}

void Ipv4TextBox::changeImage(IMAGE_TYPE_e imageType)
{
    QString imgPath;
    m_currentImageType = imageType;
    imgPath = IPTEXTBOX_FOLDER_PATH + imgTypePath[m_currentImageType];
    m_image = QPixmap(imgPath);
    SCALE_IMAGE(m_image);
    update();
}

void Ipv4TextBox::paintEvent(QPaintEvent *event)
{
    BgTile::paintEvent(event);
    QPainter painter(this);
    painter.drawPixmap(m_imgRect, m_image);
}

void Ipv4TextBox::forceActiveFocus()
{
    if(m_isEnabled)
    {
        this->setFocus();
    }
}

void Ipv4TextBox::selectControl()
{
    if(m_currentImageType != IMAGE_TYPE_MOUSE_HOVER)
    {
        changeImage(IMAGE_TYPE_MOUSE_HOVER);
    }
}

void Ipv4TextBox::deSelectControl()
{
    for(quint8 index = 0; index < MAX_OCTAL; index++)
    {
        m_palette->setColor(QPalette::Text, QColor(NORMAL_FONT_COLOR).lighter(200));
        if(IS_VALID_OBJ(m_octalLineEdit[index]))
        {
            m_octalLineEdit[index]->setPalette(*m_palette);
        }
    }

    if(m_currentImageType != IMAGE_TYPE_NORMAL)
    {
        changeImage(IMAGE_TYPE_NORMAL);
    }
}

void Ipv4TextBox::focusInEvent(QFocusEvent *)
{
    if((virtualKeypad != NULL) || (entryByRemote == true))
    {
        m_octalLineEdit[m_currSelectedOctal]->setActiveFocus();
    }
    else
    {
        selectControl();
    }
}

void Ipv4TextBox::focusOutEvent(QFocusEvent *)
{
    if(editMode == false)
    {
        deSelectControl();
    }
}

void Ipv4TextBox::createDefaultComponent()
{
    quint16 labelWidth = 0, labelHeight = 0, translatedlabelWidth=0;
    quint16 width = 0;
    qint8   verticalOffset = 0;
    QFont   labelFont;

    if(m_label != "")
    {
        labelFont = TextLabel::getFont(NORMAL_FONT_FAMILY, NORMAL_FONT_SIZE);
        translatedlabelWidth = QFontMetrics(labelFont).width(QApplication::translate(QT_TRANSLATE_STR, m_label.toUtf8().constData()));
        labelWidth = QFontMetrics(labelFont).width(m_label);
        labelHeight = QFontMetrics(labelFont).height();
        width += SCALE_WIDTH(10);
    }

    width += m_image.width() + labelWidth;
    switch(m_bgTileType)
    {
        case NO_LAYER:
            m_width = width + m_leftMargin;
            this->setGeometry(m_startX, m_startY, m_width, m_height);
            m_isInCentre = false;
            break;

        case TOP_TABLE_LAYER:
            verticalOffset = (TOP_MARGIN / 2);
            break;

        case BOTTOM_TABLE_LAYER:
            verticalOffset = -(TOP_MARGIN / 2);
            break;

        default:
            break;
    }

    if(m_isInCentre == true)
    {
        m_imgRect.setRect(this->width()/2 - m_leftMarginFromCenter,
                          (this->height() - m_image.height())/2 + verticalOffset,
                          m_image.width(),
                          m_image.height());
        if(m_label != "")
        {
            labelWidth = (translatedlabelWidth > ((this->width()/2) - SCALE_WIDTH(20))) ? ((this->width()/2) - SCALE_WIDTH(20)) : translatedlabelWidth;
            m_labelTextLabel = new TextLabel(((this->width()/2) - SCALE_WIDTH(10) - labelWidth) - m_leftMarginFromCenter,
                                             (this->height() - labelHeight)/2 + verticalOffset,
                                             NORMAL_FONT_SIZE, m_label,
                                             this, NORMAL_FONT_COLOR, NORMAL_FONT_FAMILY, ALIGN_START_X_START_Y,
                                             0, 0, labelWidth, 0, 0, Qt::AlignRight);
        }
    }
    else
    {
        if(m_label != "")
        {
            translatedlabelWidth = (translatedlabelWidth > ((m_leftMargin + labelWidth) - SCALE_WIDTH(17))) ? ((m_leftMargin + labelWidth) - SCALE_WIDTH(17)) : (translatedlabelWidth);
            m_labelTextLabel = new TextLabel(abs((abs(translatedlabelWidth - (m_leftMargin + labelWidth))) - SCALE_WIDTH(5)),
                                             (this->height() - labelHeight)/2+ verticalOffset,
                                             NORMAL_FONT_SIZE, m_label,
                                             this, NORMAL_FONT_COLOR, NORMAL_FONT_FAMILY, ALIGN_START_X_START_Y,
                                             0, 0, translatedlabelWidth, 0, 0, Qt::AlignRight);
            labelWidth += SCALE_WIDTH(10);
        }

        m_imgRect.setRect(m_leftMargin + labelWidth,
                          (this->height() - m_image.height())/2 + verticalOffset,
                          m_image.width(),
                          m_image.height());
    }


    labelFont = TextLabel::getFont(NORMAL_FONT_FAMILY, NORMAL_FONT_SIZE);
    labelWidth = QFontMetrics(labelFont).width(IP_SEPARATOR);
    labelHeight = QFontMetrics(labelFont).height();

    m_palette = new QPalette();
    if(m_isEnabled)
    {
        m_palette->setColor(QPalette::Text, QColor(NORMAL_FONT_COLOR).lighter(200));
    }
    else
    {
        m_palette->setColor(QPalette::Text, QColor(QString(DISABLE_FONT_COLOR)));
    }
    m_palette->setColor(QPalette::Base, QColor(Qt::transparent));

    for(quint8 index = 0; index < MAX_OCTAL; index++)
    {
        m_octalLineEdit[index] = new LineEdit(m_imgRect.x() + SCALE_WIDTH(10) + ((SCALE_WIDTH(40)) *index),
                                              m_imgRect.y() + (m_imgRect.height() - SCALE_HEIGHT(20))/2,
                                              SCALE_WIDTH(40), SCALE_HEIGHT(20), this, index);
        m_octalLineEdit[index]->setFrame(false);
        m_octalLineEdit[index]->setAlignment(Qt::AlignLeft);
        m_octalLineEdit[index]->setPalette(*m_palette);
        m_octalLineEdit[index]->setFont(labelFont);
        m_octalLineEdit[index]->setText("");
        currTextValue[index] = "";

        connect(m_octalLineEdit[index],
                SIGNAL(sigFocusChange(quint8,bool,bool)),
                this,
                SLOT(slotLineEditFocusChange(quint8,bool,bool)));
    }

    for(quint8 index = 0; index < (MAX_OCTAL - 1); index++)
    {
        m_dotTextLabels[index] = new TextLabel(m_octalLineEdit[index]->x() + m_octalLineEdit[index]->width()- SCALE_WIDTH(3),
                                               (m_imgRect.y() + (m_imgRect.height()/2)),
                                               NORMAL_FONT_SIZE,
                                               IP_SEPARATOR, this,
                                               NORMAL_FONT_COLOR,
                                               NORMAL_FONT_FAMILY,
                                               ALIGN_START_X_CENTRE_Y,
                                               200);
    }
}

void Ipv4TextBox::setIsEnabled(bool isEnable)
{
    if(m_isEnabled == isEnable)
    {
        return;
    }

    m_isEnabled = isEnable;
    this->setEnabled(m_isEnabled);

    if(isEnable == true)
    {
        m_palette->setColor(QPalette::Text, QColor(NORMAL_FONT_COLOR).lighter(200));
        for(quint8 index = 0; index < (MAX_OCTAL - 1); index++)
        {
            m_dotTextLabels[index]->changeColor(NORMAL_FONT_COLOR, 200);
        }
        changeImage(IMAGE_TYPE_NORMAL);
    }
    else
    {
        m_palette->setColor(QPalette::Text, QColor(QString(DISABLE_FONT_COLOR)));
        for(quint8 index = 0; index < (MAX_OCTAL - 1); index++)
        {
            m_dotTextLabels[index]->changeColor(DISABLE_FONT_COLOR);
        }
        changeImage(IMAGE_TYPE_DISABLE);
    }

    for(quint8 index = 0; index < MAX_OCTAL; index++)
    {
        m_octalLineEdit[index]->setEnabled(m_isEnabled);
        m_octalLineEdit[index]->setPalette(*m_palette);
    }

    update();
}
void Ipv4TextBox::setIpaddress(QString str)
{
    QStringList tempStrList;

    do
    {
        if(str != "")
        {
            tempStrList = str.split(IP_SEPARATOR);
            if(tempStrList.length() != MAX_OCTAL)
            {
                isAllAddrEmpty = true;
                for(quint8 index = 0; index < MAX_OCTAL; index++)
                {
                    currTextValue[index] = "";
                    m_octalLineEdit[index]->setText("");
                }
                break;
            }

            for(quint8 index = 0; index < MAX_OCTAL; index++)
            {
                quint8 tempVal = tempStrList.at(index).toUInt();
                currTextValue[index] = QString("%1").arg(tempVal);
                m_octalLineEdit[index]->setText(currTextValue[index]);
                isAllAddrEmpty = false;
            }
        }
        else
        {
            isAllAddrEmpty = true;
            for(quint8 index = 0; index < MAX_OCTAL; index++)
            {
                currTextValue[index] = "";
                m_octalLineEdit[index]->setText("");
            }
        }
    }while(0);

    update();
}

void Ipv4TextBox::getIpaddress(QString &str)
{
    if(isAllAddrEmpty == true)
    {
        str = "";
    }
    else
    {
        QStringList tempStrList;
        for(quint8 index = 0; index < MAX_OCTAL; index++)
        {
            tempStrList.append(currTextValue[index]);
        }
        str = tempStrList.join(".");
    }
}

QString Ipv4TextBox::getIpaddress()
{
    return QString((isAllAddrEmpty == true) ? "" : getCurrentIpAddress());
}

QString Ipv4TextBox::getCurrentIpAddress()
{
    QStringList tempStrList;
    for(quint8 index = 0; index < MAX_OCTAL; index++)
    {
        // if multiple zeros are there, then make it single zero
        if(currTextValue[index].toInt() == 0)
        {
            currTextValue[index] = "0";
        }
        tempStrList.append(currTextValue[index]);
    }
    return tempStrList.join(".");
}

QString Ipv4TextBox::getSubnetOfIpaddress()
{
    QStringList tempStrList;
    for(quint8 index = 0; index < (MAX_OCTAL-1); index++)
    {
        // if multiple zeros are there, then make it single zero
        tempStrList.append(currTextValue[index]);
    }
    return tempStrList.join(".");
}

QString Ipv4TextBox::getlastOctalOfIpaddress()
{
    return currTextValue[(MAX_OCTAL-1)];
}

QString Ipv4TextBox::getSubnetToPrefixLength()
{
    QHostAddress subnetMask(getCurrentIpAddress());
    quint32 maskInt = subnetMask.toIPv4Address();
    return INT_TO_QSTRING(std::bitset<32>(maskInt).count());
}

void Ipv4TextBox::navigationKeyPressed(QKeyEvent *event)
{
    if((m_catchKey == true) && (entryByRemote == true))
    {
        switch(event->key())
        {
            case Qt::Key_Left:
                event->accept();
                slotTextBoxKeyPressed(KEY_LEFT_ARROW, "");
                break;

            case Qt::Key_Right:
                event->accept();
                slotTextBoxKeyPressed(KEY_RIGHT_ARROW, "");
                break;

            default:
                QWidget::keyPressEvent(event);
                break;
        }
    }
    else
    {
        QWidget::keyPressEvent(event);
    }
}

void Ipv4TextBox::backspaceKeyPressed(QKeyEvent *event)
{
    event->accept();
    if((m_catchKey) && (entryByRemote == true))
    {
        slotTextBoxKeyPressed(KEY_BACKSPACE, "");
    }
}

void Ipv4TextBox::enterKeyPressed(QKeyEvent *event)
{
    event->accept();
    if(m_catchKey)
    {
        if(entryByRemote == false)
        {
            editMode = true;
            m_isControlActivated = false;
            mouseClickOnBox(true);
            createPhysicalKeyboardWidget();
        }
        else
        {
            m_isControlActivated = true;
            slotTextBoxKeyPressed(KEY_DONE, "");
        }
    }
}

void Ipv4TextBox::deleteKeyPressed(QKeyEvent *event)
{
    event->accept();
    if((m_catchKey) && (entryByRemote == true))
    {
        slotTextBoxKeyPressed(KEY_CLEAR, "");
    }
}

void Ipv4TextBox::escKeyPressed(QKeyEvent *event)
{
    event->accept();
    if((m_catchKey) && (entryByRemote == true))
    {
        m_isControlActivated = true;
        slotTextBoxKeyPressed(KEY_CLOSE, "");
    }
    else
    {
        QWidget::keyPressEvent(event);
    }
}

void Ipv4TextBox::asciiCharKeyPressed(QKeyEvent *event)
{
    if((m_catchKey) && (entryByRemote == true))
    {
        switch(event->key())
        {
            case Qt::Key_0:
            case Qt::Key_1:
            case Qt::Key_2:
            case Qt::Key_3:
            case Qt::Key_4:
            case Qt::Key_5:
            case Qt::Key_6:
            case Qt::Key_7:
            case Qt::Key_8:
            case Qt::Key_9:
                event->accept();
                slotTextBoxKeyPressed(KEY_ALPHANUM, QString("%1").arg(event->key() - Qt::Key_0));
                break;

            default:
                event->accept();
                break;
        }
    }

}

void Ipv4TextBox::mouseMoveEvent(QMouseEvent *event)
{
    if((m_imgRect.contains(event->pos())) && (m_isControlActivated))
    {
        if(editMode == false)
        {
            if(this->hasFocus())
            {
                selectControl();
            }
            else
            {
                forceActiveFocus();
                emit sigUpdateCurrentElement(m_indexInPage);
            }
        }
    }
}

void Ipv4TextBox::hideEvent(QHideEvent *event)
{
    QWidget::hideEvent(event);
    if(virtualKeypad != NULL)
    {
        m_inVisibleWidget->setVisible(false);
        virtualKeypad->setVisible(false);
    }
}

void Ipv4TextBox::showEvent(QShowEvent * event)
{
    QWidget::showEvent(event);
    if(virtualKeypad != NULL)
    {
        m_inVisibleWidget->setVisible(true);
        virtualKeypad->setVisible(true);
    }
}

void Ipv4TextBox::mousePressEvent(QMouseEvent *event)
{
    if((m_imgRect.contains(event->pos())) && (event->button() == m_leftMouseButton))
    {
        mouseClickOnBox();
    }
}

void Ipv4TextBox::unloadVirtualKeyboard()
{
    if(m_catchKey == false)
    {
        unloadVirtualKeypad();
        entryByRemote = true;
        editMode = true;
        createPhysicalKeyboardWidget();
    }
}

void Ipv4TextBox::createPhysicalKeyboardWidget(void)
{
    if (false == IS_VALID_OBJ(m_physicalKeyboardInVisibleWidget))
    {
        /* Memory Deallocated in unloadVirtualKeypad function */
        m_physicalKeyboardInVisibleWidget = new InvisibleWidgetCntrl(this->window());
        connect(m_physicalKeyboardInVisibleWidget,
                SIGNAL(sigMouseClick()),
                this,
                SLOT(slotInvisibleCtrlMouseClick()));
    }
}

void Ipv4TextBox::deletePhysicalKeyboardWidget(void)
{
    if (IS_VALID_OBJ(m_physicalKeyboardInVisibleWidget))
    {
        disconnect(m_physicalKeyboardInVisibleWidget,
                   SIGNAL(sigMouseClick()),
                   this,
                   SLOT(slotInvisibleCtrlMouseClick()));
        DELETE_OBJ(m_physicalKeyboardInVisibleWidget);
    }
}

void Ipv4TextBox::mouseClickOnBox(bool isByRemote)
{
    if(m_currentImageType != IMAGE_TYPE_MOUSE_HOVER)
    {
        changeImage(IMAGE_TYPE_MOUSE_HOVER);
    }

    for(quint8 index =0; index < MAX_OCTAL; index++)
    {
        oldTextValue[index] = currTextValue[index];
    }

    if(isByRemote == true)
    {
        entryByRemote = true;
        m_octalLineEdit[0]->setActiveFocus();
    }
    else
    {
        if(virtualKeypad == NULL)
        {
            /* Memory Deallocated in unload Virtual Keypad */
            m_inVisibleWidget = new InvisibleWidgetCntrl(this->window());
            connect(m_inVisibleWidget,
                    SIGNAL(sigMouseClick()),
                    this,
                    SLOT(slotInvisibleCtrlMouseClick()));

            /* Memory Deallocated in unload Virtual Keypad */
            virtualKeypad = new VirtualKeypad(SCALE_WIDTH(618), SCALE_HEIGHT(800), this->window ());
            connect(virtualKeypad,
                    SIGNAL(sigKeyDetected(KEY_TYPE_e,QString)),
                    this,
                    SLOT(slotTextBoxKeyPressed(KEY_TYPE_e,QString)));
            setCatchKey(false);
        }
        m_octalLineEdit[m_currSelectedOctal]->setActiveFocus();
    }

    currCurPos = m_octalLineEdit[m_currSelectedOctal]->cursorPosition();
}

void Ipv4TextBox::slotLineEditFocusChange(quint8 index, bool isFocusIn, bool forceFocus)
{
    editMode = isFocusIn;

    if(!((currTextValue[0] == "") || (currTextValue[1] == "") || (currTextValue[2] == "") || (currTextValue[3] == "")))
    {
        isAllAddrEmpty = false;
    }

    if(editMode == false)
    {
        deSelectControl();
    }
    else
    {
        m_currSelectedOctal = index;
        selectControl();
        updateAllOctal();
    }
    Q_UNUSED(forceFocus);
}

void Ipv4TextBox::updateAllOctal()
{
    for(quint8 index = 0; index < MAX_OCTAL; index++)
    {
        m_palette->setColor(QPalette::Text, QColor(NORMAL_FONT_COLOR).lighter(200));
        if(IS_VALID_OBJ(m_octalLineEdit[index]))
        {
            m_octalLineEdit[index]->setPalette(*m_palette);
        }
    }

    m_palette->setColor(QPalette::Text, QColor(QString(HIGHLITED_FONT_COLOR)));
    if(IS_VALID_OBJ(m_octalLineEdit[m_currSelectedOctal]))
    {
        m_octalLineEdit[m_currSelectedOctal]->setPalette(*m_palette);
        m_octalLineEdit[m_currSelectedOctal]->setActiveFocus();
    }

    update();
}

void Ipv4TextBox::unloadVirtualKeypad()
{
    if(virtualKeypad != NULL)
    {
        disconnect(virtualKeypad,
                   SIGNAL(sigKeyDetected(KEY_TYPE_e,QString)),
                   this,
                   SLOT(slotTextBoxKeyPressed(KEY_TYPE_e,QString)));
        virtualKeypad->deleteLater();
        virtualKeypad = NULL;

        disconnect(m_inVisibleWidget,
                   SIGNAL(sigMouseClick()),
                   this,
                   SLOT(slotInvisibleCtrlMouseClick()));
        DELETE_OBJ(m_inVisibleWidget);
        setCatchKey(true);
    }

    deletePhysicalKeyboardWidget();
}

void Ipv4TextBox::doneKeyValidation(void)
{
    QString tempIp;
    quint8 index = 0;

    if(m_isSubnetMask)
    {
        tempIp = getCurrentIpAddress();
        for(index = 0; index < MAX_SUBNET_MASK_VALUES; index++)
        {
            if(subnetMaskList[index] == tempIp)
            {
                break;
            }
        }

        if(index == MAX_SUBNET_MASK_VALUES)
        {
            for(quint8 index = 0; index < MAX_OCTAL; index++)
            {
                currTextValue[index] = oldTextValue[index];
            }
        }
    }

    for(quint8 index = 0; index < MAX_OCTAL; index++)
    {
        if(oldTextValue[index] == "")
        {
            oldTextValue[index] = "0";
        }
    }

    if((currTextValue[0] == "") && (currTextValue[1] == "") && (currTextValue[2] == "") && (currTextValue[3] == ""))
    {
        isAllAddrEmpty = true;
        emit sigLoadInfopage(m_indexInPage);
    }
    else if((currTextValue[0] == "") || (currTextValue[0].toUInt() == 0))
    {
        // load info page
        // "Please enter complete IP address"
        currTextValue[0] = currTextValue[1] = currTextValue[2] = currTextValue[3] = "";
        isAllAddrEmpty = true;
        emit sigLoadInfopage(m_indexInPage);
    }
    else
    {
        if(currTextValue[1] == "")
        {
            currTextValue[1] = oldTextValue[1];
        }

        if(currTextValue[2] == "")
        {
            currTextValue[2] = oldTextValue[2];
        }

        if(currTextValue[3] == "")
        {
            currTextValue[3] = oldTextValue[3];
        }
        isAllAddrEmpty = false;
    }

    for(quint8 index = 0; index < MAX_OCTAL; index++)
    {
        quint8 tempVal = currTextValue[index].toUInt();
        currTextValue[index] = QString("%1").arg(tempVal);
        oldTextValue[index] = currTextValue[index];
        m_octalLineEdit[index]->setText(currTextValue[index]);
    }

    emit sigEntryDone(m_indexInPage);
}

void Ipv4TextBox::slotTextBoxKeyPressed(KEY_TYPE_e keyType, QString str)
{
    QString newStr = currTextValue[m_currSelectedOctal];
    bool    isAlphaNumKeyAccept = false;
    quint16 intValue = 0;
    m_octalLineEdit[m_currSelectedOctal]->setActiveFocus();

    switch(keyType)
    {
        case KEY_ALPHANUM:
            // check for key to validator
            if(str.contains(QRegExp("[0-9]")))
            {
                newStr = newStr.insert(currCurPos, str);
                if((currCurPos <= 2) && (currTextValue[m_currSelectedOctal].length() < 3))
                {
                    intValue = newStr.toUInt();
                    if(m_currSelectedOctal == 0)
                    {
                        if((m_isSubnetMask) && (intValue <= SUBNETMASK_FIRSTOCTAL_MAX_VALUE))
                        {
                            isAlphaNumKeyAccept = true;
                        }
                        else if(intValue <= FIRST_OCTAL_MAX_VALUE)
                        {
                            isAlphaNumKeyAccept = true;
                        }
                    }
                    else if(intValue <= OCTAL_MAX_VALUE)
                    {
                        isAlphaNumKeyAccept = true;
                    }
                }
            }

            if(isAlphaNumKeyAccept == true)
            {
                currTextValue[m_currSelectedOctal] = newStr;
                currCurPos++;
                m_octalLineEdit[m_currSelectedOctal]->setText(currTextValue[m_currSelectedOctal]);

                if((currCurPos == 3) || (currTextValue[m_currSelectedOctal].length() == 3))
                {
                    currCurPos = 0;
                    m_currSelectedOctal = ((m_currSelectedOctal + 1)%MAX_OCTAL);
                    updateAllOctal();
                }
                m_octalLineEdit[m_currSelectedOctal]->setCursorPosition(currCurPos);
            }
            break;

        case KEY_CLEAR:
            currCurPos = 0;
            currTextValue[m_currSelectedOctal] = "";
            m_octalLineEdit[m_currSelectedOctal]->setText(currTextValue[m_currSelectedOctal]);
            m_octalLineEdit[m_currSelectedOctal]->setCursorPosition(currCurPos);
            break;

        case KEY_BACKSPACE:
            if(currCurPos > 0)
            {
                currCurPos--;
                currTextValue[m_currSelectedOctal] = currTextValue[m_currSelectedOctal].remove(currCurPos, 1);
                m_octalLineEdit[m_currSelectedOctal]->setText(currTextValue[m_currSelectedOctal]);
            }
            else
            {
                m_currSelectedOctal = ((m_currSelectedOctal - 1 + MAX_OCTAL)%MAX_OCTAL);
                currCurPos = currTextValue[m_currSelectedOctal].length();
                updateAllOctal();
            }
            m_octalLineEdit[m_currSelectedOctal]->setCursorPosition(currCurPos);
            break;

        case KEY_LEFT_ARROW:
            if(currCurPos > 0)
            {
                currCurPos--;
            }
            else
            {
                m_currSelectedOctal = ((m_currSelectedOctal - 1 + MAX_OCTAL)%MAX_OCTAL);
                currCurPos = currTextValue[m_currSelectedOctal].length();
                updateAllOctal();
            }
            m_octalLineEdit[m_currSelectedOctal]->setCursorPosition(currCurPos);
            break;

        case KEY_RIGHT_ARROW:
            currCurPos++;
            if(currCurPos > currTextValue[m_currSelectedOctal].length())
            {
                currCurPos = 0;
                m_currSelectedOctal = ((m_currSelectedOctal + 1)%MAX_OCTAL);
                updateAllOctal();
            }
            m_octalLineEdit[m_currSelectedOctal]->setCursorPosition(currCurPos);
            break;

        case KEY_DONE:
            if(IS_VALID_OBJ(m_octalLineEdit[m_currSelectedOctal]))
            {
                m_octalLineEdit[m_currSelectedOctal]->setText(currTextValue[m_currSelectedOctal]);
                m_octalLineEdit[m_currSelectedOctal]->setCursorPosition(currCurPos);
            }

            editMode= false;
            unloadVirtualKeypad();
            entryByRemote = false;
            forceActiveFocus();
            doneKeyValidation();
            break;

        case KEY_CLOSE:
            for(quint8 index = 0; index < MAX_OCTAL; index++)
            {
                currTextValue[index] = oldTextValue[index];
                m_octalLineEdit[index]->setText(currTextValue[index]);
            }

            editMode = false;
            unloadVirtualKeypad();
            entryByRemote = false;
            forceActiveFocus();
            break;

        default:
            break;
    }

    update();
}

void Ipv4TextBox::slotInvisibleCtrlMouseClick()
{
    if (virtualKeypad != NULL)
    {
        virtualKeypad->slotKeyDeteceted(KEY_DONE, 0);
    }

    if (m_physicalKeyboardInVisibleWidget != NULL)
    {
        slotTextBoxKeyPressed(KEY_DONE, "");
    }
}
