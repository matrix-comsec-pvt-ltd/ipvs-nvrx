#include "IpTextBox.h"
#include <QMouseEvent>
#include <QPainter>

#define IPTEXTBOX_FOLDER_PATH               IMAGE_PATH"Textbox/"
#define MAX_COLON_IN_IPV6_ADDR              7
#define MAX_IPV6_ADDR_LEN                   39

const QString ipTextboxBtnFolder[MAX_IP_TEXTBOX_SIZE] =
{
    "Small/",
    "Medium/",
    "UltraMedium/",
    "Large/",
    "UltraLarge/",
    "ExtraSmall/",
    "ExtraLarge/",
    "TableTypeTextbox/"
};

IpTextBox::IpTextBox(quint32 startX, quint32 startY,
                     quint32 width, quint32 height,
                     quint16 controlIndex,
                     QString labelstr,
                     IP_ADDR_TYPE_e ipAddressType,
                     QWidget *parent,
                     BGTILE_TYPE_e bgType,
                     bool isBoxInCentre,
                     quint16 leftMarginVal,
                     bool isNavigationEnable,
                     IP_FIELD_TYPE_e fieldType,
                     IP_TEXTBOX_SIZE_e size,
                     quint32 leftMarginFromCenter)
    : BgTile(startX, startY, width, height, bgType, parent),
      NavigationControl(controlIndex, isNavigationEnable),
      m_label(labelstr), m_isInCentre(isBoxInCentre), entryByRemote(false), editMode(false),
      m_leftMargin(leftMarginVal), virtualKeypad(NULL), currCurPos(0), m_fieldType(fieldType), textBoxSize(size),
      m_leftMarginFromCenter(leftMarginFromCenter)
{
    INIT_OBJ(m_inVisibleWidget);
    INIT_OBJ(m_physicalKeyboardInVisibleWidget);
    m_ipAddressType = ipAddressType;

    if(m_ipAddressType == IP_ADDR_TYPE_IPV4_ONLY)
    {
        alphaNumRegExp = QRegExp(QString("[0-9.]"));
    }
    else if(m_ipAddressType == IP_ADDR_TYPE_IPV6_ONLY)
    {
        alphaNumRegExp = QRegExp(QString("[a-fA-F0-9:]"));
    }
    else
    {
        alphaNumRegExp = QRegExp(QString("[a-fA-F0-9.:]"));
    }

    m_currentImageType = (m_isEnabled) ? IMAGE_TYPE_NORMAL : IMAGE_TYPE_DISABLE;
    m_image = QPixmap(QString(IPTEXTBOX_FOLDER_PATH + ipTextboxBtnFolder[textBoxSize] + imgTypePath[m_currentImageType]));
    SCALE_IMAGE(m_image);

    this->setEnabled(m_isEnabled);
    this->setMouseTracking(true);
    createDefaultComponent();
    this->show();
}

IpTextBox::~IpTextBox()
{
    slotInvisibleCtrlMouseClick();

    if(m_label != "")
    {
        delete m_labelTextLabel;
    }

    delete m_palette;

    delete m_lineEdit;

    /* PARASOFT: To make it happy */
    DELETE_OBJ(virtualKeypad);
    DELETE_OBJ(m_inVisibleWidget);
    DELETE_OBJ(m_physicalKeyboardInVisibleWidget);
}

void IpTextBox::changeImage(IMAGE_TYPE_e imageType)
{
    QString imgPath;
    m_currentImageType = imageType;
    imgPath = IPTEXTBOX_FOLDER_PATH + ipTextboxBtnFolder[textBoxSize] + imgTypePath[m_currentImageType];
    m_image = QPixmap(imgPath);
    SCALE_IMAGE(m_image);
    update();
}

void IpTextBox::paintEvent(QPaintEvent *event)
{
    BgTile::paintEvent(event);
    QPainter painter(this);
    painter.drawPixmap(m_imgRect, m_image);
}

void IpTextBox::forceActiveFocus()
{
    if(m_isEnabled)
    {
        this->setFocus();
    }
}

void IpTextBox::selectControl()
{
    if(m_currentImageType != IMAGE_TYPE_MOUSE_HOVER)
    {
        changeImage(IMAGE_TYPE_MOUSE_HOVER);
    }
}

void IpTextBox::deSelectControl()
{
    m_palette->setColor(QPalette::Text, QColor(NORMAL_FONT_COLOR).lighter(200));
    if(IS_VALID_OBJ(m_lineEdit))
    {
        m_lineEdit->setPalette(*m_palette);
    }

    if(m_currentImageType != IMAGE_TYPE_NORMAL)
    {
        changeImage(IMAGE_TYPE_NORMAL);
    }
}

void IpTextBox::focusInEvent(QFocusEvent *)
{
    if((virtualKeypad != NULL) || (entryByRemote == true))
    {
        m_lineEdit->setActiveFocus();
    }
    else
    {
        selectControl();
    }
}

void IpTextBox::focusOutEvent(QFocusEvent *)
{
    if(editMode == false)
    {
        deSelectControl();
    }
}

void IpTextBox::createDefaultComponent()
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

    m_lineEdit = new LineEdit(m_imgRect.x() + SCALE_WIDTH(10),
                              m_imgRect.y(),
                              m_imgRect.width() - SCALE_WIDTH(20),
                              m_imgRect.height(), this);

    m_lineEdit->setFrame(false);
    m_lineEdit->setAlignment(Qt::AlignLeft);
    m_lineEdit->setMaxLength(MAX_IPV6_ADDR_LEN);
    m_lineEdit->setPalette(*m_palette);
    m_lineEdit->setFont(labelFont);
    m_lineEdit->setText("");
    currTextValue = "";
}

void IpTextBox::setIsEnabled(bool isEnable)
{
    if (m_isEnabled == isEnable)
    {
        return;
    }

    m_isEnabled = isEnable;
    this->setEnabled(m_isEnabled);

    if(isEnable == true)
    {
        m_palette->setColor(QPalette::Text, QColor(NORMAL_FONT_COLOR).lighter(200));
        changeImage(IMAGE_TYPE_NORMAL);
    }
    else
    {
        m_palette->setColor(QPalette::Text, QColor(QString(DISABLE_FONT_COLOR)));
        changeImage(IMAGE_TYPE_DISABLE);
    }

    m_lineEdit->setEnabled(m_isEnabled);
    m_lineEdit->setPalette(*m_palette);
    update();
}

void IpTextBox::setIpaddress(QString str)
{
    currTextValue = str;
    m_lineEdit->setText(currTextValue);
    m_lineEdit->setCursorPosition(0);
    update();
}

void IpTextBox::getIpaddress(QString &str)
{
    str = currTextValue;
}

QString IpTextBox::getIpaddress()
{
    return currTextValue;
}

bool IpTextBox::operator > (const IpTextBox &other) const
{
    Q_IPV6ADDR ipv6Address1, ipv6Address2;

    if (QHostAddress(currTextValue).protocol() == QAbstractSocket::IPv6Protocol)
    {
        ipv6Address1 = QHostAddress(currTextValue).toIPv6Address();
        ipv6Address2 = QHostAddress(other.currTextValue).toIPv6Address();

        for (quint8 index = 0; index < 16; index++)
        {
            if (ipv6Address1[index] > ipv6Address2[index])
            {
                return true;
            }

            if (ipv6Address1[index] < ipv6Address2[index])
            {
                return false;
            }
        }

        return false;
    }
    else
    {
        return (QHostAddress(currTextValue).toIPv4Address() > QHostAddress(other.currTextValue).toIPv4Address());
    }
}

bool IpTextBox::operator < (const IpTextBox &other) const
{
    Q_IPV6ADDR ipv6Address1, ipv6Address2;

    if (QHostAddress(currTextValue).protocol() == QAbstractSocket::IPv6Protocol)
    {
        ipv6Address1 = QHostAddress(currTextValue).toIPv6Address();
        ipv6Address2 = QHostAddress(other.currTextValue).toIPv6Address();

        for (quint8 index = 0; index < 16; index++)
        {
            if (ipv6Address1[index] < ipv6Address2[index])
            {
                return true;
            }

            if (ipv6Address1[index] > ipv6Address2[index])
            {
                return false;
            }
        }

        return false;
    }
    else
    {
        return (QHostAddress(currTextValue).toIPv4Address() < QHostAddress(other.currTextValue).toIPv4Address());
    }
}

void IpTextBox::navigationKeyPressed(QKeyEvent *event)
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

void IpTextBox::backspaceKeyPressed(QKeyEvent *event)
{
    event->accept();
    if((m_catchKey) && (entryByRemote == true))
    {
        slotTextBoxKeyPressed(KEY_BACKSPACE, "");
    }
}

void IpTextBox::enterKeyPressed(QKeyEvent *event)
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

void IpTextBox::deleteKeyPressed(QKeyEvent *event)
{
    event->accept();
    if((m_catchKey) && (entryByRemote == true))
    {
        slotTextBoxKeyPressed(KEY_CLEAR, "");
    }
}

void IpTextBox::escKeyPressed(QKeyEvent *event)
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

void IpTextBox::asciiCharKeyPressed(QKeyEvent *event)
{
    if((m_catchKey) && (entryByRemote == true))
    {
        if((event->key() == Qt::Key_Space))
        {
            return;
        }

        event->accept();
        slotTextBoxKeyPressed(KEY_ALPHANUM, event->text());
    }
}

void IpTextBox::mouseMoveEvent(QMouseEvent *event)
{
    if(!((m_imgRect.contains(event->pos())) && (m_isControlActivated) && (editMode == false)))
    {
        return;
    }

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

void IpTextBox::hideEvent(QHideEvent *event)
{
    QWidget::hideEvent(event);
    if(virtualKeypad != NULL)
    {
        m_inVisibleWidget->setVisible(false);
        virtualKeypad->setVisible(false);
    }
}

void IpTextBox::showEvent(QShowEvent * event)
{
    QWidget::showEvent(event);
    if(virtualKeypad != NULL)
    {
        m_inVisibleWidget->setVisible(true);
        virtualKeypad->setVisible(true);
    }
}

void IpTextBox::mousePressEvent(QMouseEvent *event)
{
    if((m_imgRect.contains(event->pos())) && (event->button() == m_leftMouseButton))
    {
        mouseClickOnBox();
    }
}

void IpTextBox::unloadVirtualKeyboard()
{
    if(m_catchKey == false)
    {
        unloadVirtualKeypad();
        entryByRemote = true;
        editMode = true;
        createPhysicalKeyboardWidget();
    }
}

void IpTextBox::createPhysicalKeyboardWidget(void)
{
    if (false == IS_VALID_OBJ(m_physicalKeyboardInVisibleWidget))
    {
        m_physicalKeyboardInVisibleWidget = new InvisibleWidgetCntrl(this->window());
        connect(m_physicalKeyboardInVisibleWidget,
                SIGNAL(sigMouseClick()),
                this,
                SLOT(slotInvisibleCtrlMouseClick()));
    }
}

void IpTextBox::deletePhysicalKeyboardWidget(void)
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

void IpTextBox::mouseClickOnBox(bool isByRemote)
{
    if(m_currentImageType != IMAGE_TYPE_MOUSE_HOVER)
    {
        changeImage(IMAGE_TYPE_MOUSE_HOVER);
    }

    oldTextValue = currTextValue;

    if(isByRemote == true)
    {
        entryByRemote = true;
        m_lineEdit->setActiveFocus();
    }
    else
    {
        if(virtualKeypad == NULL)
        {
            m_inVisibleWidget = new InvisibleWidgetCntrl(this->window());
            connect(m_inVisibleWidget,
                    SIGNAL(sigMouseClick()),
                    this,
                    SLOT(slotInvisibleCtrlMouseClick()));

            virtualKeypad = new VirtualKeypad(SCALE_WIDTH(618), SCALE_HEIGHT(800), this->window());
            connect(virtualKeypad,
                    SIGNAL(sigKeyDetected(KEY_TYPE_e,QString)),
                    this,
                    SLOT(slotTextBoxKeyPressed(KEY_TYPE_e,QString)));
            setCatchKey(false);
        }
        m_lineEdit->setActiveFocus();
    }
    currCurPos = m_lineEdit->cursorPosition();
}

void IpTextBox::unloadVirtualKeypad()
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

void IpTextBox::doneKeyValidation(void)
{
    bool isIpv4Valid = false;
    bool isIpv6Valid = false;

    if ((m_ipAddressType == IP_ADDR_TYPE_IPV4_ONLY) || (m_ipAddressType == IP_ADDR_TYPE_IPV4_AND_IPV6))
    {
        struct sockaddr_in  tServerAddr4;
        char                tCurrentAddr4[INET_ADDRSTRLEN];

        do
        {
            // Validation using inet_pton
            if (inet_pton(AF_INET, currTextValue.toStdString().c_str(), &tServerAddr4.sin_addr) <= 0)
            {
                break;
            }

            inet_ntop(AF_INET, &tServerAddr4.sin_addr, tCurrentAddr4, sizeof(tCurrentAddr4));
            currTextValue = QString::fromUtf8(tCurrentAddr4);
            isIpv4Valid = true;

        }while(0);
    }

    if ((m_ipAddressType == IP_ADDR_TYPE_IPV6_ONLY) || (m_ipAddressType == IP_ADDR_TYPE_IPV4_AND_IPV6))
    {
        struct sockaddr_in6 tServerAddr6;
        char                tCurrentAddr6[INET6_ADDRSTRLEN];

        do
        {
            // Allowing empty ipv6 address for DNSv6
            if((m_fieldType == IP_FIELD_TYPE_DNSV6) && (currTextValue == ""))
            {
                isIpv6Valid = true;
                break;
            }

            // Validation using inet_pton
            if (inet_pton(AF_INET6, currTextValue.toStdString().c_str(), &tServerAddr6.sin6_addr) <= 0)
            {
                break;
            }

            inet_ntop(AF_INET6, &tServerAddr6.sin6_addr, tCurrentAddr6, sizeof(tCurrentAddr6));
            currTextValue = QString::fromUtf8(tCurrentAddr6);

            // Skip Local ipv6 validation for Static Route Entry
            if (m_fieldType == IP_FIELD_TYPE_STATIC_ROUTE_NW_ADDR)
            {
                isIpv6Valid = true;
                break;
            }

            // Check start and end of ipv6 address
            if ((currTextValue.startsWith(':')) || (currTextValue.endsWith(':')) || (currTextValue.startsWith("ff")) || (currTextValue.startsWith("fe")))
            {
                break;
            }

            // Check if first or last octet is zero
            if ((currTextValue.length() >= 2) && (((currTextValue.at(currTextValue.length() - 2) == ':') && (currTextValue.endsWith('0'))) ||
                                                  ((currTextValue.at(1) == ':') && (currTextValue.startsWith('0')))))
            {
                break;
            }

            isIpv6Valid = true;

        }while(0);
    }

    if((isIpv4Valid == true) || (isIpv6Valid == true))
    {
        oldTextValue = currTextValue;
        emit sigEntryDone(m_indexInPage);
    }
    else
    {
        currTextValue = oldTextValue;
        emit sigLoadInfopage(m_indexInPage);
    }
}

void IpTextBox::slotTextBoxKeyPressed(KEY_TYPE_e keyType, QString str)
{
    QString newStr = currTextValue;
    m_lineEdit->setActiveFocus();
	
    switch(keyType)
    {
        case KEY_ALPHANUM:
            if(str.contains(alphaNumRegExp))
            {
                newStr = newStr.insert(currCurPos, str);
                currTextValue = newStr;
                currCurPos++;
                m_lineEdit->setText(currTextValue);
                m_lineEdit->setCursorPosition(currCurPos);
            }
            break;

        case KEY_CLEAR:
            currCurPos = 0;
            currTextValue= "";
            m_lineEdit->setText(currTextValue);
            m_lineEdit->setCursorPosition(currCurPos);
            break;

        case KEY_BACKSPACE:
            if(currCurPos > 0)
            {
                currCurPos--;
                currTextValue = currTextValue.remove(currCurPos, 1);
                m_lineEdit->setText(currTextValue);
            }
            m_lineEdit->setCursorPosition(currCurPos);
            break;

        case KEY_LEFT_ARROW:
            if(currCurPos > 0)
            {
                currCurPos--;
            }
            m_lineEdit->setCursorPosition(currCurPos);
            break;

        case KEY_RIGHT_ARROW:
            if(currCurPos < currTextValue.length())
            {
                currCurPos++;
            }
            m_lineEdit->setCursorPosition(currCurPos);
            break;

        case KEY_DONE:
            doneKeyValidation();
            if(IS_VALID_OBJ(m_lineEdit))
            {
                m_lineEdit->setText(currTextValue);
                m_lineEdit->setCursorPosition(0);
            }

            editMode = false;
            unloadVirtualKeypad();
            entryByRemote = false;
            forceActiveFocus();
            break;

        case KEY_CLOSE:
            currTextValue = oldTextValue;
            m_lineEdit->setText(currTextValue);
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

void IpTextBox::slotInvisibleCtrlMouseClick()
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
