#include "MacTextBox.h"
#include "ValidationMessage.h"

#include "Ipv4TextBox.h"
#include <QMouseEvent>
#include <QTimer>
#include <QPaintEvent>
#include <QPainter>

#define MACTEXTBOX_FOLDER_PATH      IMAGE_PATH"Textbox/Large/"
#define MAC_SEPARATOR                ":"

const quint8 maxNumKeyRepeat[10] =
{ 1, 1, 4, 4, 1, 1, 1, 1, 1, 1};

const QString numKeyLoweCaseRep [10] =
{
    {"0"},
    {"1"},
    {"2"},
    {"3"},
    {"4"},
    {"5"},
    {"6"},
    {"7"},
    {"8"},
    {"9"}
};

const QString numKeyAToF[6] =
{
    {"A"},
    {"B"},
    {"C"},
    {"D"},
    {"E"},
    {"F"}
};

const QString numKeyaTof[6] =
{
    {"a"},
    {"b"},
    {"c"},
    {"d"},
    {"e"},
    {"f"}
};

static const QRegExp macRegExp = QRegExp(QString("[0-9abcdefABCDEF]"));

MacTextBox::MacTextBox(quint32 startX,
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
                       quint32 leftMarginFromCenter)
    :BgTile(startX,
            startY,
            width,
            height,
            bgType,
            parent), NavigationControl(controlIndex, isNavigationEnable),
      m_label(labelstr), m_isInCentre(isBoxInCentre), entryByRemote(false), remoteCapsLock(false),
      editMode(false), isAllAddrEmpty(false), m_leftMargin(leftMarginVal), m_currSelectedOctal(0),
      virtualKeypad(NULL),currCurPos(0), repeatKeyCnt(0), m_leftMarginFromCenter(leftMarginFromCenter)
{
    QString imgPath;
    m_inVisibleWidget = NULL;
    prevRemoteKey = Qt::Key_unknown;

    if(m_isEnabled)
    {
        m_currentImageType = IMAGE_TYPE_NORMAL;
    }
    else
    {
        m_currentImageType = IMAGE_TYPE_DISABLE;
    }

    imgPath = MACTEXTBOX_FOLDER_PATH + imgTypePath[m_currentImageType];
    m_image = QPixmap(imgPath);
    SCALE_IMAGE(m_image);

    keyRepTimer = new QTimer(this);
    connect (keyRepTimer,
             SIGNAL(timeout()),
             this,
             SLOT(slotkeyRepTimerTimeout()));
    keyRepTimer->setInterval (1000);
    keyRepTimer->setSingleShot (true);

    this->setEnabled (m_isEnabled);
    this->setMouseTracking (true);
    createDefaultComponent();

    this->show ();
}

MacTextBox::~MacTextBox()
{
    slotInvisibleCtrlMouseClick ();
    if(m_label != "")
    {
        delete m_labelTextLabel;
    }

    delete m_palette;

    for(quint8 index = 0; index < MAX_MAC_OCTAL; index++)
    {
        disconnect (m_octalLineEdit[index],
                 SIGNAL(sigFocusChange(quint8,bool,bool)),
                 this,
                 SLOT(slotLineEditFocusChange(quint8,bool,bool)));
        delete m_octalLineEdit[index];
    }

    for(quint8 index = 0; index < (MAX_MAC_OCTAL-1); index++)
    {
        delete m_dotTextLabels[index];
    }

    disconnect (keyRepTimer,
             SIGNAL(timeout()),
             this,
             SLOT(slotkeyRepTimerTimeout()));
    DELETE_OBJ(keyRepTimer);

}
//*****************************************************************************
// changeImage
//      Param:
//          IN : Not Applicable
//          OUT: Not Applicable
//      Returns:
//          Not Applicable
//      Description:
//*****************************************************************************
void MacTextBox::changeImage (IMAGE_TYPE_e imageType)
{
    QString imgPath;
    m_currentImageType = imageType;
    imgPath = MACTEXTBOX_FOLDER_PATH + imgTypePath[m_currentImageType];
    m_image = QPixmap(imgPath);
    SCALE_IMAGE(m_image);
    update ();
}

//*****************************************************************************
// changeImage
//      Param:
//          IN : Not Applicable
//          OUT: Not Applicable
//      Returns:
//          Not Applicable
//      Description:
//*****************************************************************************
void MacTextBox::paintEvent (QPaintEvent *event)
{
    BgTile::paintEvent (event);

    QPainter painter(this);
    painter.drawPixmap (m_imgRect, m_image);
}

//*****************************************************************************
// changeImage
//      Param:
//          IN : Not Applicable
//          OUT: Not Applicable
//      Returns:
//          Not Applicable
//      Description:
//*****************************************************************************
void MacTextBox::forceActiveFocus()
{
    if(m_isEnabled)
    {
        this->setFocus();
    }
}
//*****************************************************************************
// changeImage
//      Param:
//          IN : Not Applicable
//          OUT: Not Applicable
//      Returns:
//          Not Applicable
//      Description:
//*****************************************************************************
void MacTextBox::selectControl()
{
    if(m_currentImageType != IMAGE_TYPE_MOUSE_HOVER)
    {
        changeImage(IMAGE_TYPE_MOUSE_HOVER);
    }
}

void MacTextBox::deSelectControl()
{
    for(quint8 index = 0; index < MAX_MAC_OCTAL; index++)
    {
        m_palette->setColor(QPalette::Text, QColor(NORMAL_FONT_COLOR).lighter (200));
        m_octalLineEdit[index]->setPalette(*m_palette);
    }
    if(m_currentImageType != IMAGE_TYPE_NORMAL)
    {
        changeImage(IMAGE_TYPE_NORMAL);
    }
}

void MacTextBox::focusInEvent(QFocusEvent *)
{
    if((virtualKeypad != NULL) || (entryByRemote == true))
    {
        m_octalLineEdit[m_currSelectedOctal]->setActiveFocus ();
    }
    else
    {
        selectControl();
    }
}

void MacTextBox::focusOutEvent(QFocusEvent *)
{
    if(editMode == false)
    {
        deSelectControl();
    }
}
//*****************************************************************************
// changeImage
//      Param:
//          IN : Not Applicable
//          OUT: Not Applicable
//      Returns:
//          Not Applicable
//      Description:
//*****************************************************************************
void MacTextBox::createDefaultComponent()
{
    quint16 labelWidth = 0, labelHeight = 0, translatedlabelWidth = 0;
    quint16 width = 0;
    qint8 verticalOffset = 0;
    QFont labelFont;

    if(m_label != "")
    {
        labelFont = TextLabel::getFont (NORMAL_FONT_FAMILY, NORMAL_FONT_SIZE);
        translatedlabelWidth = QFontMetrics(labelFont).width (QApplication::translate(QT_TRANSLATE_STR, m_label.toUtf8().constData()));
        labelWidth = QFontMetrics(labelFont).width (m_label);
        labelHeight = QFontMetrics(labelFont).height ();
        width += SCALE_WIDTH(10);
    }

    width += m_image.width () + labelWidth;

    switch(m_bgTileType)
    {
    case NO_LAYER:
        m_width = width;
        this->setGeometry(m_startX, m_startY, m_width, m_height);
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
        if(m_label != "")
        {
            labelWidth = (translatedlabelWidth > ((getWidth()/2) - SCALE_WIDTH(20)))? ((getWidth()/2) - SCALE_WIDTH(20)) : translatedlabelWidth;
            m_labelTextLabel = new TextLabel(((this->width ()/2) - SCALE_WIDTH(10) - labelWidth) - m_leftMarginFromCenter,
                                             (this->height () - labelHeight)/2 + verticalOffset,
                                             NORMAL_FONT_SIZE, m_label,
                                             this, NORMAL_FONT_COLOR, NORMAL_FONT_FAMILY, ALIGN_START_X_START_Y,
                                             0, 0, labelWidth, 0, 0, Qt::AlignRight);
        }

        m_imgRect.setRect (this->width ()/2 - m_leftMarginFromCenter,
                           (this->height () - m_image.height ())/2 + verticalOffset,
                           m_image.width (),
                           m_image.height ());

    }
    else
    {

        if(m_label != "")
        {
            translatedlabelWidth = (translatedlabelWidth > ((m_leftMargin + labelWidth) - SCALE_WIDTH(17))) ? ((m_leftMargin + labelWidth) - SCALE_WIDTH(17)) : (translatedlabelWidth);
            m_labelTextLabel = new TextLabel(abs((abs(translatedlabelWidth - (m_leftMargin + labelWidth))) - SCALE_WIDTH(5)),
                                             (this->height () - labelHeight)/2+ verticalOffset,
                                             NORMAL_FONT_SIZE, m_label,
                                             this,
                                             NORMAL_FONT_COLOR, NORMAL_FONT_FAMILY, ALIGN_START_X_START_Y,
                                             0, 0, translatedlabelWidth, 0, 0, Qt::AlignRight);
        }

        m_imgRect.setRect (m_leftMargin+labelWidth,
                           (this->height () - m_image.height ())/2 + verticalOffset,
                           m_image.width (),
                           m_image.height ());
    }


    labelFont = TextLabel::getFont (NORMAL_FONT_FAMILY, NORMAL_FONT_SIZE);
    labelWidth = QFontMetrics(labelFont).width (":");
    labelHeight = QFontMetrics(labelFont).height ();

    m_palette = new QPalette();
    if(m_isEnabled)
    {
        m_palette->setColor(QPalette::Text, QColor(NORMAL_FONT_COLOR).lighter (200));
    }
    else
    {
        m_palette->setColor(QPalette::Text, QColor(QString(DISABLE_FONT_COLOR)));
    }
    m_palette->setColor(QPalette::Base, QColor(Qt::transparent));

    for(quint8 index = 0; index < MAX_MAC_OCTAL; index++)
    {
        m_octalLineEdit[index] = new LineEdit(m_imgRect.x () + SCALE_WIDTH(5) + ((SCALE_WIDTH(28)) *index),
                                              m_imgRect.y () + (m_imgRect.height () - SCALE_HEIGHT(20))/2,
                                              SCALE_WIDTH(28), SCALE_HEIGHT(20), this, index);
        m_octalLineEdit[index]->setFrame (false);
        m_octalLineEdit[index]->setAlignment (Qt::AlignLeft);

        m_octalLineEdit[index]->setPalette(*m_palette);
        m_octalLineEdit[index]->setFont (labelFont);

        m_octalLineEdit[index]->setText ("");
        currTextValue[index] = "";

        connect (m_octalLineEdit[index],
                 SIGNAL(sigFocusChange(quint8,bool,bool)),
                 this,
                 SLOT(slotLineEditFocusChange(quint8,bool,bool)));
    }

    for(quint8 index = 0; index < (MAX_MAC_OCTAL - 1); index++)
    {
        m_dotTextLabels[index] = new TextLabel(m_octalLineEdit[index]->x () + m_octalLineEdit[index]->width ()- SCALE_WIDTH(4),
                                               (m_imgRect.y () + (m_imgRect.height ()/2)),
                                               NORMAL_FONT_SIZE,
                                               MAC_SEPARATOR,
                                               this,
                                               NORMAL_FONT_COLOR,
                                               NORMAL_FONT_FAMILY,
                                               ALIGN_START_X_CENTRE_Y,
                                               200);
    }
}
//*****************************************************************************
// createDefaultElement
//      Param:
//          IN : Not Applicable
//          OUT: Not Applicable
//      Returns:
//          Not Applicable
//      Description:
//*****************************************************************************
void MacTextBox::setIsEnabled(bool isEnable)
{
    if(m_isEnabled != isEnable)
    {
        m_isEnabled = isEnable;
        this->setEnabled (m_isEnabled);

        if(isEnable == true)
        {
            m_palette->setColor(QPalette::Text, QColor(NORMAL_FONT_COLOR).lighter (200));
            for(quint8 index = 0; index < (MAX_MAC_OCTAL-1); index++)
            {
                m_dotTextLabels[index]->changeColor (NORMAL_FONT_COLOR, 200);
            }
            changeImage(IMAGE_TYPE_NORMAL);
        }
        else
        {
            m_palette->setColor(QPalette::Text, QColor(QString(DISABLE_FONT_COLOR)));
            for(quint8 index = 0; index < (MAX_MAC_OCTAL-1); index++)
            {
                m_dotTextLabels[index]->changeColor (DISABLE_FONT_COLOR);
            }
            changeImage(IMAGE_TYPE_DISABLE);
        }
        for(quint8 index = 0; index < MAX_MAC_OCTAL; index++)
        {
            m_octalLineEdit[index]->setEnabled (m_isEnabled);
            m_octalLineEdit[index]->setPalette(*m_palette);
        }
        update();
    }
}
void MacTextBox::setMacaddress(QString str)
{
    QStringList tempStrList;
    if(str != "")
    {
        tempStrList = str.split (MAC_SEPARATOR);
        for(quint8 index = 0; index < MAX_MAC_OCTAL; index++)
        {
            currTextValue[index] = tempStrList.at (index);
            m_octalLineEdit[index]->setText (currTextValue[index]);
            isAllAddrEmpty = false;
        }
    }
    else
    {
        isAllAddrEmpty = true;
        for(quint8 index = 0; index < MAX_MAC_OCTAL; index++)
        {
            currTextValue[index] = "";
            m_octalLineEdit[index]->setText ("");
        }
    }
    update ();
}

void MacTextBox::getMacaddress(QString &str)
{
    if(isAllAddrEmpty == true)
    {
        str = "";
    }
    else
    {
        QStringList tempStrList;
        for(quint8 index = 0; index < MAX_MAC_OCTAL; index++)
        {
            tempStrList.append (currTextValue[index]);
        }
        str = tempStrList.join (":");
    }
}

QString MacTextBox::getMacaddress()
{
    QString str;

    if(isAllAddrEmpty == true)
    {
        str = "";
    }
    else
    {
        QStringList tempStrList;
        for(quint8 index = 0; index < MAX_MAC_OCTAL; index++)
        {
            tempStrList.append (currTextValue[index]);
        }
        str = tempStrList.join (":");
    }
    return str;
}

void MacTextBox::navigationKeyPressed(QKeyEvent *event)
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

void MacTextBox::backspaceKeyPressed(QKeyEvent *event)
{
    event->accept();
    slotTextBoxKeyPressed(KEY_BACKSPACE, "");
}

void MacTextBox::enterKeyPressed(QKeyEvent *event)
{
    event->accept();
    if(entryByRemote == false)
    {
        editMode = true;
        m_isControlActivated = false;
        mouseClickOnBox(true);
    }
    else
    {
         m_isControlActivated = true;
         slotTextBoxKeyPressed(KEY_DONE, "");
    }

}

void MacTextBox::deleteKeyPressed(QKeyEvent *event)
{
    event->accept();
    slotTextBoxKeyPressed(KEY_CLEAR, "");
}

void MacTextBox::escKeyPressed(QKeyEvent *event)
{
    event->accept();
    m_isControlActivated = true;
    slotTextBoxKeyPressed(KEY_CLOSE, "");
}

void MacTextBox::asciiCharKeyPressed(QKeyEvent *event)
{
    QString tempStr = "";
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
        tempStr = numKeyLoweCaseRep[event->key() - Qt::Key_0];
        slotTextBoxKeyPressed (KEY_ALPHANUM,tempStr);
        break;

    case Qt::Key_A:
    case Qt::Key_B:
    case Qt::Key_C:
    case Qt::Key_D:
    case Qt::Key_E:
    case Qt::Key_F:
         event->accept();
         if((event->modifiers() & Qt::ShiftModifier) == Qt::ShiftModifier)
         {
            tempStr = numKeyAToF[event->key() - Qt::Key_A];
            slotTextBoxKeyPressed (KEY_ALPHANUM,tempStr);
         }
         else
         {
            tempStr = numKeyaTof[event->key() - Qt::Key_A];
            slotTextBoxKeyPressed (KEY_ALPHANUM,tempStr);
         }
        break;

    default:
        event->accept();
        break;
    }
}

//*****************************************************************************
// createDefaultElement
//      Param:
//          IN : Not Applicable
//          OUT: Not Applicable
//      Returns:
//          Not Applicable
//      Description:
//*****************************************************************************
void MacTextBox::mouseMoveEvent (QMouseEvent *event)
{
    if(m_imgRect.contains(event->pos())&& (m_isControlActivated))
    {
        if(editMode == false)
        {
            if(this->hasFocus ())
            {
                selectControl ();
            }
            else
            {
                forceActiveFocus();
                emit sigUpdateCurrentElement(m_indexInPage);
            }
        }
    }
}

void MacTextBox::hideEvent (QHideEvent *event)
{
    QWidget::hideEvent (event);
    if(virtualKeypad != NULL)
    {
        m_inVisibleWidget->setVisible (false);
        virtualKeypad->setVisible (false);
    }
}

void MacTextBox::showEvent (QShowEvent * event)
{
    QWidget::showEvent (event);
    if(virtualKeypad != NULL)
    {
        m_inVisibleWidget->setVisible (true);
        virtualKeypad->setVisible (true);
    }
}
//*****************************************************************************
// createDefaultElement
//      Param:
//          IN : Not Applicable
//          OUT: Not Applicable
//      Returns:
//          Not Applicable
//      Description:
//*****************************************************************************
void MacTextBox::mousePressEvent (QMouseEvent *event)
{
    if(m_imgRect.contains(event->pos())
            && (event->button() == m_leftMouseButton))
    {
        mouseClickOnBox ();
    }
}

void MacTextBox::mouseClickOnBox (bool isByRemote)
{
    if(m_currentImageType != IMAGE_TYPE_MOUSE_HOVER)
    {
        changeImage(IMAGE_TYPE_MOUSE_HOVER);
    }

    for(quint8 index =0; index < MAX_MAC_OCTAL; index++)
    {
        oldTextValue[index] = currTextValue[index];
    }

    if(isByRemote == true)
    {
        entryByRemote = true;
        m_octalLineEdit[0]->setActiveFocus ();
    }
    else
    {
        if(virtualKeypad == NULL)
        {
            /* PARASOFT: Memory Deallocated in unload Virtual Keypad */
            m_inVisibleWidget = new InvisibleWidgetCntrl(this->window());
            connect (m_inVisibleWidget,
                     SIGNAL(sigMouseClick()),
                     this,
                     SLOT(slotInvisibleCtrlMouseClick()));

            /* PARASOFT: Memory Deallocated in unload Virtual Keypad */
            virtualKeypad = new VirtualKeypad(SCALE_WIDTH(618), SCALE_HEIGHT(800), this->window ());
            connect (virtualKeypad,
                     SIGNAL(sigKeyDetected(KEY_TYPE_e,QString)),
                     this,
                     SLOT(slotTextBoxKeyPressed(KEY_TYPE_e,QString)));
            setCatchKey (false);
        }
        m_octalLineEdit[m_currSelectedOctal]->setActiveFocus ();
    }
    currCurPos = m_octalLineEdit[m_currSelectedOctal]->cursorPosition ();
}

void MacTextBox::slotLineEditFocusChange(quint8 index, bool isFocusIn, bool forceFocus)
{
    editMode = isFocusIn;
    if(editMode == false)
    {
        deSelectControl ();
    }
    else
    {
        m_currSelectedOctal = index;
        selectControl();
        updateAllOctal ();
    }
    Q_UNUSED(forceFocus);
}

void MacTextBox::updateAllOctal()
{
    for(quint8 index = 0; index < MAX_MAC_OCTAL; index++)
    {
        m_palette->setColor(QPalette::Text, QColor(QString(NORMAL_FONT_COLOR)));
        m_octalLineEdit[index]->setPalette(*m_palette);
    }
    m_palette->setColor(QPalette::Text, QColor(QString(HIGHLITED_FONT_COLOR)));
    m_octalLineEdit[m_currSelectedOctal]->setPalette(*m_palette);
    m_octalLineEdit[m_currSelectedOctal]->setActiveFocus ();
    update ();
}

void MacTextBox::unloadVirtualKeypad()
{
    if(virtualKeypad != NULL)
    {
        disconnect (virtualKeypad,
                    SIGNAL(sigKeyDetected(KEY_TYPE_e,QString)),
                    this,
                    SLOT(slotTextBoxKeyPressed(KEY_TYPE_e,QString)));
        virtualKeypad->deleteLater();
        virtualKeypad = NULL;

        disconnect (m_inVisibleWidget,
                    SIGNAL(sigMouseClick()),
                    this,
                    SLOT(slotInvisibleCtrlMouseClick()));
        delete m_inVisibleWidget;
        setCatchKey (true);
    }
}

//*****************************************************************************
// createDefaultElement
//      Param:
//          IN : Not Applicable
//          OUT: Not Applicable
//      Returns:
//          Not Applicable
//      Description:
//*****************************************************************************
void MacTextBox::doneKeyValidation(void)
{
    isAllAddrEmpty = false;
    if((currTextValue[0] == "") && (currTextValue[1] == "") &&
            (currTextValue[2] == "") && (currTextValue[3] == "") &&
            (currTextValue[4] == "") && (currTextValue[5] == ""))
    {
        isAllAddrEmpty = true;
    }
    if((currTextValue[0] == "") || (currTextValue[1] == "") ||
            (currTextValue[2] == "") || (currTextValue[3] == "") ||
            (currTextValue[4] == "") || (currTextValue[5] == ""))
    {
        for(quint8 index = 0; index < MAX_MAC_OCTAL; index++)
        {
            currTextValue[index] = "";
            oldTextValue[index] = "";
        }
        isAllAddrEmpty = true;
        // emit sig of
        //"Please Enter MAC Address",
        emit sigLoadInfopage(m_indexInPage);
    }
    for(quint8 index = 0; index < MAX_MAC_OCTAL; index++)
    {
        m_octalLineEdit[index]->setText (currTextValue[index]);
    }
    //    getMacaddress (str);
}

void MacTextBox::slotkeyRepTimerTimeout ()
{
    prevRemoteKey = Qt::Key_unknown;
    repeatKeyCnt = 0;
}
//*****************************************************************************
// createDefaultElement
//      Param:
//          IN : Not Applicable
//          OUT: Not Applicable
//      Returns:
//          Not Applicable
//      Description:
//*****************************************************************************
void MacTextBox::slotTextBoxKeyPressed(KEY_TYPE_e keyType, QString str)
{
    QString newStr = currTextValue[m_currSelectedOctal];
    QString tempStr = "";
    m_octalLineEdit[m_currSelectedOctal]->setActiveFocus();

    switch(keyType)
    {
    case KEY_ALPHANUM:
        // check for key to validator
        if(str.contains (macRegExp))
        {
            switch(currCurPos)
            {
            case 0:
                switch(newStr.length ())
                {
                case 2:
                    tempStr = newStr.mid (1,1);
                    newStr = "";
                    newStr.append (str);
                    newStr.append (tempStr);
                    currTextValue[m_currSelectedOctal] = newStr;
                    m_octalLineEdit[m_currSelectedOctal]->setText (currTextValue[m_currSelectedOctal]);

                    currCurPos = 1;
                    break;

                case 1:
                    tempStr = newStr;
                    newStr = "";
                    newStr.append (str);
                    newStr.append (tempStr);
                    currTextValue[m_currSelectedOctal] = newStr;
                    m_octalLineEdit[m_currSelectedOctal]->setText (currTextValue[m_currSelectedOctal]);

                    currCurPos = 0;
                    m_currSelectedOctal = ((m_currSelectedOctal + 1)%MAX_MAC_OCTAL);
                    break;

                case 0:
                    newStr.append (str);
                    currTextValue[m_currSelectedOctal] = newStr;
                    m_octalLineEdit[m_currSelectedOctal]->setText (currTextValue[m_currSelectedOctal]);
                    currCurPos++;
                    break;

                default:
                    break;
                }
                break;

            case 1:
                switch(newStr.length ())
                {
                case 2:
                    newStr.replace (1, 1, str);
                    currTextValue[m_currSelectedOctal] = newStr;
                    m_octalLineEdit[m_currSelectedOctal]->setText (currTextValue[m_currSelectedOctal]);
                    currCurPos = 0;
                    m_currSelectedOctal = ((m_currSelectedOctal + 1)%MAX_MAC_OCTAL);
                    break;

                case 1:
                    newStr.append (str);
                    currTextValue[m_currSelectedOctal] = newStr;
                    m_octalLineEdit[m_currSelectedOctal]->setText (currTextValue[m_currSelectedOctal]);
                    currCurPos++;
                    break;

                default:
                    break;
                }
                break;

            case 2:
                if(newStr.length () == 2)
                {
                    currCurPos = 1;
                    m_currSelectedOctal = ((m_currSelectedOctal + 1)%MAX_MAC_OCTAL);

                    newStr = currTextValue[m_currSelectedOctal];
                    if(newStr.length () >= 1)
                    {
                        newStr.replace (0, 1, str);
                    }
                    else if(newStr.length () == 0)
                    {
                        newStr.append (str);
                    }
                    currTextValue[m_currSelectedOctal] = newStr;
                    m_octalLineEdit[m_currSelectedOctal]->setText (currTextValue[m_currSelectedOctal]);
                }
                break;

            default:
                break;
            }
            m_octalLineEdit[m_currSelectedOctal]->setCursorPosition (currCurPos);
            updateAllOctal ();
        }
        break;
    case KEY_ALPHANUM_SAME_INDEX:

        if(currCurPos == 0)
        {
            m_currSelectedOctal = ((m_currSelectedOctal - 1 + MAX_MAC_OCTAL)%MAX_MAC_OCTAL);
            newStr = currTextValue[m_currSelectedOctal];
            currCurPos = 2;

            newStr = newStr.remove (currCurPos - 1, 1);
            newStr = newStr.insert (currCurPos - 1, str);

            currTextValue[m_currSelectedOctal] = newStr;
            m_octalLineEdit[m_currSelectedOctal]->setText (currTextValue[m_currSelectedOctal]);

            m_octalLineEdit[m_currSelectedOctal]->setCursorPosition (currCurPos);
        }
        else
        {
            newStr = newStr.remove (currCurPos - 1, 1);
            newStr = newStr.insert (currCurPos - 1, str);

            currTextValue[m_currSelectedOctal] = newStr;
            m_octalLineEdit[m_currSelectedOctal]->setText (currTextValue[m_currSelectedOctal]);

            m_octalLineEdit[m_currSelectedOctal]->setCursorPosition (currCurPos);
        }
        updateAllOctal ();
        break;
    case KEY_CLEAR:
        currCurPos = 0;
        currTextValue[m_currSelectedOctal] = "";
        m_octalLineEdit[m_currSelectedOctal]->setText (currTextValue[m_currSelectedOctal]);
        m_octalLineEdit[m_currSelectedOctal]->setCursorPosition (currCurPos);
        break;

    case KEY_BACKSPACE:
        if(currCurPos > 0)
        {
            currCurPos--;
            currTextValue[m_currSelectedOctal] = currTextValue[m_currSelectedOctal].remove (currCurPos, 1);
            m_octalLineEdit[m_currSelectedOctal]->setText (currTextValue[m_currSelectedOctal]);
        }
        else
        {
            m_currSelectedOctal = ((m_currSelectedOctal - 1 + MAX_MAC_OCTAL)%MAX_MAC_OCTAL);
            currCurPos = currTextValue[m_currSelectedOctal].length ();
            updateAllOctal ();
        }
        m_octalLineEdit[m_currSelectedOctal]->setCursorPosition (currCurPos);
        break;

    case KEY_LEFT_ARROW:
        if(currCurPos > 0)
        {
            currCurPos--;
        }
        else
        {
            m_currSelectedOctal = ((m_currSelectedOctal - 1 + MAX_MAC_OCTAL)%MAX_MAC_OCTAL);
            currCurPos = currTextValue[m_currSelectedOctal].length ();
            updateAllOctal ();
        }
        m_octalLineEdit[m_currSelectedOctal]->setCursorPosition (currCurPos);
        break;

    case KEY_RIGHT_ARROW:
        currCurPos++;
        if(currCurPos > currTextValue[m_currSelectedOctal].length ())
        {
            currCurPos = 0;
            m_currSelectedOctal = ((m_currSelectedOctal + 1)%MAX_MAC_OCTAL);
            updateAllOctal ();
        }
        m_octalLineEdit[m_currSelectedOctal]->setCursorPosition (currCurPos);
        break;

    case KEY_DONE:
        m_octalLineEdit[m_currSelectedOctal]->setText (currTextValue[m_currSelectedOctal]);
        m_octalLineEdit[m_currSelectedOctal]->setCursorPosition (currCurPos);

        editMode= false;

        if(entryByRemote == false)
        {
            unloadVirtualKeypad ();
        }

        entryByRemote = false;
        forceActiveFocus ();
        doneKeyValidation();
        break;

    case KEY_CLOSE:
        for(quint8 index = 0; index < MAX_MAC_OCTAL; index++)
        {
            currTextValue[index] = oldTextValue[index];
            m_octalLineEdit[index]->setText (currTextValue[index]);
        }
        editMode = false;

        if(entryByRemote == false)
        {
            unloadVirtualKeypad ();
        }

        entryByRemote = false;
        forceActiveFocus ();
        break;

    default:
        break;
    }
    update ();
}

void MacTextBox::slotInvisibleCtrlMouseClick ()
{
    if(virtualKeypad != NULL)
    {
        virtualKeypad->slotKeyDeteceted (KEY_DONE,0);
    }
}
