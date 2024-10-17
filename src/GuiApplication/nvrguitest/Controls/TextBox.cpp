#include <QFontMetrics>
#include <QMouseEvent>
#include <QTimer>
#include "TextBox.h"
#include "Enumfile.h"
#include <QPaintEvent>
#include <QPainter>
//#include "Controls/MenuButton.h"

#define TEXTBOX_FOLDER_PATH      IMAGE_PATH "/Textbox/"
#define WIDTH_PADDING            (10)

const QString textboxBtnFolder[MAX_TEXTBOX_SIZE] =
{
    "Small/",
    "Medium/",
    "UltraMedium/",
    "Large/",
    "ExtraSmall/",
    "ExtraLarge/",
    "TableTypeTextbox/"
};

const quint8 maxNumKeyRepeat[10] =
{ 2, 13, 4, 4, 4, 4, 4, 5, 4, 5};

const QString numKeyLoweCaseRep [10][13] =
{
    {"0", " "},
    {"1", ",", ".", "/", "{", "}", "_", "-", "+", "?", "=","&", "*"},
    {"2", "a", "b", "c"},
    {"3", "d", "e", "f"},
    {"4", "g", "h", "i"},
    {"5", "j", "k", "l"},
    {"6", "m", "n", "o"},
    {"7", "p", "q", "r", "s"},
    {"8", "t", "u", "v"},
    {"9", "w", "x", "y", "z"}
};


const QString numKeyUpperCaseRep [10][13] =
{
    {")", " "},
    {"!", "<", ">", "?", "{", "}", "_", ":", "=", "|", "\"","~", "`"},
    {"@", "A", "B", "C"},
    {"#", "D", "E", "F"},
    {"$", "G", "H", "I"},
    {"%", "J", "K", "L"},
    {"^", "M", "N", "O"},
    {"&", "P", "Q", "R", "S"},
    {"*", "T", "U", "V"},
    {"(", "W", "X", "Y", "Z"}
};

//*****************************************************************************
// TextBox
//      Param:
//          IN : Not Applicable
//          OUT: Not Applicable
//      Returns:
//          Not Applicable
//      Description:
//*****************************************************************************
TextBox::TextBox(quint32 startX,
                 quint32 startY,
                 quint32 width,
                 quint32 height,
                 quint16 controlIndex,
                 TEXTBOX_SIZE_e size,
                 QWidget *parent,
                 TextboxParam *textBoxParam,
                 BGTILE_TYPE_e bgType,
                 bool isNavigationEnable,
                 bool isCharCountExternallyControl,
                 bool isDoneKeySigNeeded)
    :BgTile(startX,
            startY,
            width,
            height,
            bgType,
            parent), NavigationControl(controlIndex, isNavigationEnable)
{
    indexOfControl = controlIndex;
    param = textBoxParam;
    textBoxSize = size;
    oldTextValue = param->textStr;
    entryByRemote = false;
    remoteCapsLock = false;
    editMode = false;
    prevRemoteKey = Qt::Key_unknown;
    repeatKeyCnt = 0;
    m_isCharCountExternallyControl = isCharCountExternallyControl;
    m_isDoneKeySigNeeded = isDoneKeySigNeeded;

    if(param->isNumEntry == true)
    {
        param->validation = QRegExp(QString("[0-9]"));
    }

    virtualKeypad = NULL;
    reloadListNeedded = true;

    if(m_isEnabled)
    {
        imgType = IMAGE_TYPE_NORMAL;
    }
    else
    {
        imgType = IMAGE_TYPE_DISABLE;
    }
    imagePath = TEXTBOX_FOLDER_PATH + textboxBtnFolder[textBoxSize]
            + imgTypePath[imgType];

    image = QPixmap(imagePath);    

    keyRepTimer = new QTimer(this);
    connect (keyRepTimer,
             SIGNAL(timeout()),
             this,
             SLOT(slotkeyRepTimerTimeout()));
    keyRepTimer->setInterval (1000);
    keyRepTimer->setSingleShot (true);

    createDefaultElement();

    lineEdit->setEnabled (m_isEnabled);

    this->setMouseTracking (true);
    this->setEnabled (m_isEnabled);

    this->show ();
}
//*****************************************************************************
// ~TextBox
//      Param:
//          IN : Not Applicable
//          OUT: Not Applicable
//      Returns:
//          Not Applicable
//      Description:
//*****************************************************************************
TextBox::~TextBox ()
{
    if(param->labelStr != "")
    {
        delete labelText;
    }
    if(param->suffixStr != "")
    {
        delete suffixText;
    }

    delete keyRepTimer;
    delete palette;
    disconnect (lineEdit,
                SIGNAL(sigFocusChange(quint8,bool,bool)),
                this,
                SLOT(slotLineEditFocusChange(quint8,bool,bool)));
    delete lineEdit;
    unloadVirtualKeypad();
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
void TextBox::changeImage (IMAGE_TYPE_e imageType)
{
    imgType = imageType;
    imagePath = TEXTBOX_FOLDER_PATH + textboxBtnFolder[textBoxSize]
            + imgTypePath[imgType];

    image = QPixmap(imagePath);    
    update ();
}

void TextBox::selectControl()
{
    if(imgType != IMAGE_TYPE_MOUSE_HOVER)
    {
        changeImage(IMAGE_TYPE_MOUSE_HOVER);
    }
}

void TextBox::deSelectControl()
{
    if(imgType != IMAGE_TYPE_NORMAL)
    {
        changeImage(IMAGE_TYPE_NORMAL);
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
void TextBox::forceActiveFocus()
{
    if(m_isEnabled)
    {
        this->setFocus();
    }
}
//*****************************************************************************
// paintEvent
//      Param:
//          IN : Not Applicable
//          OUT: Not Applicable
//      Returns:
//          Not Applicable
//      Description:
//*****************************************************************************
void TextBox::paintEvent (QPaintEvent *event)
{
    QWidget::paintEvent(event);
    BgTile::paintEvent(event);

    QPainter painter(this);
    painter.drawPixmap (imgRect, image);
}

void TextBox::focusInEvent(QFocusEvent *)
{
    if((virtualKeypad != NULL) || (entryByRemote == true))
    {
        lineEdit->setActiveFocus ();
    }
    else
    {
        selectControl();
    }
}

void TextBox::focusOutEvent(QFocusEvent *)
{
    if(editMode == false)
    {
        deSelectControl();
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
void TextBox::createDefaultElement()
{
    quint16 labelWidth = 0,suffixWidth = 0, labelHeight = 0, strHeight = 0;
    quint16 width = 0;
    qint8 verticalOffset = 0;
    QFont labelFont = TextLabel::getFont (NORMAL_FONT_FAMILY, NORMAL_FONT_SIZE);
    QFont suffixFont;

    if(param->labelStr != "")
    {
        labelFont = TextLabel::getFont (NORMAL_FONT_FAMILY, NORMAL_FONT_SIZE);
        labelWidth = QFontMetrics(labelFont).width (param->labelStr);
        labelHeight = QFontMetrics(labelFont).height ();
        width += WIDTH_PADDING;
    }

    if(param->suffixStr != "")
    {
        suffixFont = TextLabel::getFont (NORMAL_FONT_FAMILY, SUFFIX_FONT_SIZE);
        suffixWidth = QFontMetrics(suffixFont).width (param->suffixStr);
        strHeight = QFontMetrics(suffixFont).height ();
        width += WIDTH_PADDING;
    }

    width += image.width () + labelWidth + suffixWidth;

    switch(m_bgTileType)
    {
    case NO_LAYER:
        m_width = width;
        this->setGeometry(m_startX, m_startY, m_width, m_height);
        param->isCentre = false;
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

    if(param->isCentre == true)
    {
        if(param->labelStr != "")
        {
            labelText = new TextLabel(((this->width ()/2) - WIDTH_PADDING - labelWidth),
                                      (this->height () - labelHeight)/2 + verticalOffset,
                                      NORMAL_FONT_SIZE,param->labelStr,
                                      this);
        }

        if(param->suffixStr != "")
        {
            suffixText = new TextLabel((this->width ()/2) + image.width () +WIDTH_PADDING,
                                       (this->height () - strHeight)/2 +verticalOffset,
                                       (SUFFIX_FONT_SIZE), param->suffixStr,
                                       this,
                                       SUFFIX_FONT_COLOR);
        }

        imgRect.setRect (this->width ()/2,
                         ((this->height () - image.height ())/2) + verticalOffset,
                         image.width (),
                         image.height ());
    }
    else
    {
        if(param->labelStr != "")
        {
            labelText = new TextLabel(param->leftMargin,
                                      (this->height () - labelHeight)/2 + verticalOffset,
                                      NORMAL_FONT_SIZE,param->labelStr,
                                      this);
            labelWidth += WIDTH_PADDING;
        }
        imgRect.setRect (param->leftMargin+labelWidth,
                         (this->height () - image.height ())/2 + verticalOffset,
                         image.width (),
                         image.height ());

        if(param->suffixStr != "")
        {
            suffixText = new TextLabel(param->leftMargin+labelWidth + image.width () +WIDTH_PADDING,
                                       (this->height () - strHeight)/2 + verticalOffset,
                                       (SUFFIX_FONT_SIZE), param->suffixStr,
                                       this,
                                       SUFFIX_FONT_COLOR);
        }


    }

    lineEdit = new LineEdit(imgRect.x () + WIDTH_PADDING,
                            imgRect.y (),
                            (imgRect.width () - (20)),
                            imgRect.height (),
                            this);
    connect (lineEdit,
             SIGNAL(sigFocusChange(quint8,bool,bool)),
             this,
             SLOT(slotLineEditFocusChange(quint8,bool,bool)));
    lineEdit->setFrame (false);

    palette = new QPalette();

    if(m_isEnabled)
    {
        palette->setColor(QPalette::Text, QColor(NORMAL_FONT_COLOR).lighter (200));

    }
    else
    {
        palette->setColor(QPalette::Text, QColor(QString(DISABLE_FONT_COLOR)));
    }

    palette->setColor(QPalette::Base, QColor(Qt::transparent));
    lineEdit->setPalette(*palette);
    lineEdit->setFont (labelFont);

    lineEdit->setText (param->textStr);
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
void TextBox::setIsEnabled(bool isEnable)
{
    if(isEnable != m_isEnabled)
    {
        m_isEnabled = isEnable;
        lineEdit->setEnabled (m_isEnabled);
        this->setEnabled (m_isEnabled);

        if(isEnable == true)
        {
            palette->setColor(QPalette::Text, QColor(NORMAL_FONT_COLOR).lighter (200));
            changeImage(IMAGE_TYPE_NORMAL);
        }
        else
        {
            palette->setColor(QPalette::Text, QColor(QString(DISABLE_FONT_COLOR)));
            changeImage(IMAGE_TYPE_DISABLE);
        }
        lineEdit->setPalette(*palette);
        update();
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
void TextBox::keyPressEvent(QKeyEvent *event)
{
    if(m_catchKey)
    {
        if(entryByRemote == false)
        {
            switch(event->key())
            {
            case Qt::Key_Return:
            case Qt::Key_Enter:
                event->accept();
                editMode = true;
                m_isControlActivated = false;
                if(imgType != IMAGE_TYPE_MOUSE_HOVER)
                {
                    changeImage(IMAGE_TYPE_MOUSE_HOVER);
                }
                mouseClickOnBox(true);
                break;

            default:
                QWidget::keyPressEvent(event);
                break;
            }
        }
        else
        {
            switch(event->key())
            {
            case Qt::Key_Backspace:
                event->accept();
                slotTextBoxKeyPressed(KEY_BACKSPACE, "");
                break;

            case Qt::Key_Left:
                event->accept();
                slotTextBoxKeyPressed(KEY_LEFT_ARROW, "");
                break;

            case Qt::Key_Right:
                event->accept();
                slotTextBoxKeyPressed(KEY_RIGHT_ARROW, "");
                break;

            case Qt::Key_Delete:
                event->accept();
                slotTextBoxKeyPressed(KEY_CLEAR, "");
                break;

            case Qt::Key_Enter:
            case Qt::Key_Return:
                event->accept();
                m_isControlActivated = true;
                slotTextBoxKeyPressed(KEY_DONE, "");
                break;

            case Qt::Key_Escape:
                // this is like cancel button
                // Send notification to page in which this control to
                // change focus on close button
                event->accept();
                m_isControlActivated = true;
                slotTextBoxKeyPressed(KEY_CLOSE, "");
                break;

            case Qt::Key_CapsLock:
                event->accept();
                remoteCapsLock = (!(remoteCapsLock));
                break;

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
                take0to9KeyAction(event->key());
                break;

            default:
                event->accept();
                break;
            }
        }
    }
}
//*****************************************************************************
// take0to9KeyAction
//      Param:
//          IN : Not Applicable
//          OUT: Not Applicable
//      Returns:
//          Not Applicable
//      Description:
//*****************************************************************************
void TextBox::take0to9KeyAction(int key)
{
    QString tempStr = "";
    if(prevRemoteKey == key)
    {
        repeatKeyCnt = ((repeatKeyCnt + 1)% maxNumKeyRepeat[key - Qt::Key_0]);
        if(keyRepTimer->isActive ())
        {
            keyRepTimer->stop ();
        }

        if(remoteCapsLock == true)
        {
            tempStr = numKeyUpperCaseRep[key - Qt::Key_0][repeatKeyCnt];
        }
        else
        {
            tempStr = numKeyLoweCaseRep[key - Qt::Key_0][repeatKeyCnt];
        }
        // send string according to key , repeat cnt
        slotTextBoxKeyPressed (KEY_ALPHANUM_SAME_INDEX,tempStr);
        keyRepTimer->start ();
    }
    else
    {
        repeatKeyCnt = 0;
        if(keyRepTimer->isActive ())
        {
            keyRepTimer->stop ();
        }

        if(remoteCapsLock == true)
        {
            tempStr = numKeyUpperCaseRep[key - Qt::Key_0][repeatKeyCnt];
        }
        else
        {
            tempStr = numKeyLoweCaseRep[key - Qt::Key_0][repeatKeyCnt];
        }
        // send string according to key , repeat cnt
        slotTextBoxKeyPressed (KEY_ALPHANUM,tempStr);

        prevRemoteKey = key;
        keyRepTimer->start ();
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
void TextBox::mouseMoveEvent (QMouseEvent *event)
{
    if(imgRect.contains(event->pos()) && (m_isControlActivated))
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

void TextBox::hideEvent (QHideEvent *event)
{
    QWidget::hideEvent (event);
    if(virtualKeypad != NULL)
    {
        m_inVisibleWidget->setVisible (false);
        virtualKeypad->setVisible (false);
    }
}

void TextBox::showEvent (QShowEvent * event)
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
void TextBox::mousePressEvent (QMouseEvent *event)
{
    if((imgRect.contains(event->pos())) &&
            (event->button () == m_leftMouseButton))
    {
        entryByRemote = false;
        m_isControlActivated = true;

        if(imgType != IMAGE_TYPE_MOUSE_HOVER)
        {
            changeImage(IMAGE_TYPE_MOUSE_HOVER);
        }
        m_mouseClicked = true;
        mouseClickOnBox ();
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
void TextBox::mouseClickOnBox (bool isByRemote)
{
    oldTextValue = param->textStr;

    if(isByRemote == true)
    {
        entryByRemote = true;
    }
    else
    {
        if(virtualKeypad == NULL)
        {
            m_inVisibleWidget = new InvisibleWidgetCntrl(this->window());
            connect (m_inVisibleWidget,
                     SIGNAL(sigMouseClick()),
                     this,
                     SLOT(slotInvisibleCtrlMouseClick()));

            virtualKeypad = new VirtualKeypad((618), (800), this->window ());
            connect (virtualKeypad,
                     SIGNAL(sigKeyDetected(KEY_TYPE_e,QString)),
                     this,
                     SLOT(slotTextBoxKeyPressed(KEY_TYPE_e,QString)));
        }
        setCatchKey (false);
    }

    lineEdit->setActiveFocus ();
    currCurPos = lineEdit->cursorPosition ();
}


void TextBox::getInputText(QString &str)
{
    str = param->textStr;
}

QString TextBox::getInputText() const
{
    return param->textStr;
}

void TextBox::setInputText (QString str)
{
    param->textStr = str;
    currCurPos = 0;
    lineEdit->setText (param->textStr);
    lineEdit->setCursorPosition  (currCurPos);
    currentCharLenght = str.length ();
    update ();
}

void TextBox::raiseVirtualKeypad()
{
    if(IS_VALID_OBJ(virtualKeypad))
    {
        m_inVisibleWidget->raise();
        virtualKeypad->raise();
    }
}


void TextBox::unloadVirtualKeypad()
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

void TextBox::setCurrentCharLenght(quint8 maxCharCount)
{
    currentCharLenght = maxCharCount;
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
void TextBox::doneKeyValidation ()
{
    bool finalState = true;
    QString tempStr;

    do
    {
        param->textStr = param->textStr.trimmed ();

        if(!(param->startCharVal.isEmpty ()) && (param->textStr != ""))
        {
            tempStr = param->textStr.at (0);
            if(!(tempStr.contains (param->startCharVal)))
            {
                finalState = false;
                param->textStr = oldTextValue;
                emit sigLoadInfopage(m_indexInPage, INFO_MSG_STRAT_CHAR);
                break;
            }
        }

        if(!(param->middelCharVal.isEmpty ()))
        {
            for(quint16 index = 1; index < param->textStr.length (); index++)
            {
                tempStr = param->textStr.at (index);
                if(!(tempStr.contains (param->middelCharVal)))
                {
                    finalState = false;
                    param->textStr = oldTextValue;
                    // deselect call
                    // page info loader with start char error msg
                    emit sigLoadInfopage(m_indexInPage, INFO_MSG_ERROR);
                    break;
                }
            }
        }
        // (!param->textStr.isEmpty()) added to stop going in when text is blank.
        if((!(param->endCharVal.isEmpty ())) &&  (!param->textStr.isEmpty()))
        {
            tempStr = param->textStr.at (param->textStr.length () -1);
            if(!(tempStr.contains (param->endCharVal)))
            {
                finalState = false;
                param->textStr = oldTextValue;
                // page info loader with start char error msg
                emit sigLoadInfopage(m_indexInPage, INFO_MSG_END_CHAR);
            }
        }
        else if ((finalState == true) && (param->textStr == ""))
        {
            if(param->isTotalBlankStrAllow == false)
            {
                finalState = false;
                param->textStr = oldTextValue;
                emit sigLoadInfopage(m_indexInPage, INFO_MSG_ERROR);
            }
        }
        else if ((finalState == true) && (param->textStr.length () < param->minChar))
        {
            finalState = false;
            param->textStr = oldTextValue;
            // deselect call
            // page info loader with start char error msg
            emit sigLoadInfopage(m_indexInPage, INFO_MSG_ERROR);
        }
        else if((finalState == true) && (param->isNumEntry == true))
        {
            if(param->textStr.toUInt ()  < param->minNumValue)
            {
                if(param->textStr.toInt () != param->extraNumValue)
                {
                    finalState = false;
                    param->textStr = oldTextValue;
                    emit sigLoadInfopage(m_indexInPage, INFO_MSG_ERROR);
                }
            }
        }

        if(param->isEmailAddrType == true)
        {
            QStringList emailList;
            QStringList emailList2;
            emailList.clear ();
            emailList2.clear ();

            if(param->textStr.contains (";"))
            {
                emailList = param->textStr.split (";",QString::SkipEmptyParts);
            }

            if(param->textStr.contains (","))
            {
                emailList = param->textStr.split (",",QString::SkipEmptyParts);
            }

            for(quint8 index = 0; index < emailList.count (); index++)
            {
                if(emailList.at (index).contains (";"))
                {
                    emailList2.append (emailList.at (index).split (";",QString::SkipEmptyParts));
                }

                if(emailList.at (index).contains (","))
                {
                    emailList2.append (emailList.at (index).split (",",QString::SkipEmptyParts));
                }
            }

            if(emailList.isEmpty ())
            {
                emailList.append (param->textStr);
            }
            else if (!emailList2.isEmpty ())
            {
                emailList.clear ();
                emailList.append (emailList2);
            }

            for(quint8 index = 0; index < emailList.count (); index++)
            {
                QString tempStr = emailList.at(index);
                QString startChar = tempStr.at(0);

                if((tempStr != "") && (!(startChar.contains (param->startCharVal))))
                {
                    param->textStr = "";
                    emit sigLoadInfopage(m_indexInPage, INFO_MSG_STRAT_CHAR);
                    break;
                }
                else if((tempStr!= "") && (!(tempStr.contains ("@") && (tempStr.contains (".")))))
                {
                    param->textStr = "";
                    emit sigLoadInfopage(m_indexInPage, INFO_MSG_ERROR);
                    break;
                }
                else if((tempStr != "") && ((tempStr.count("@") > 1)))
                {
                    param->textStr = "";
                    emit sigLoadInfopage(m_indexInPage, INFO_MSG_ERROR);
                    break;
                }
            }

            emailList.clear ();
            emailList2.clear ();
        }

    }while(0);

    if(finalState == true)
    {
        oldTextValue = param->textStr;
    }
    else
    {
        emit sigTextValueAppended(param->textStr, m_indexInPage);
    }

    currCurPos = 0;
    lineEdit->setText (param->textStr);
    lineEdit->setCursorPosition  (currCurPos);

    if(m_isDoneKeySigNeeded)
    {
        emit sigDoneKeyClicked(m_indexInPage);
    }
}

void TextBox::slotLineEditFocusChange(quint8, bool isFocusIn, bool forceFocus)
{
    editMode = isFocusIn;
    if(editMode == false)
    {
        deSelectControl ();
    }
    else
    {
        if(imgType != IMAGE_TYPE_MOUSE_HOVER)
        {
            changeImage(IMAGE_TYPE_MOUSE_HOVER);
        }
    }
    if(forceFocus)
    {
        forceActiveFocus ();
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
void TextBox::slotkeyRepTimerTimeout ()
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
void TextBox::slotTextBoxKeyPressed(KEY_TYPE_e keyType, QString str)
{
    QString newStr = param->textStr;
    bool isAlphaNumKeyAccept = false;
    quint32 intValue;

    if((param->isEmailAddrType == true) && (keyType == KEY_SPACE))
    {
        return;
    }

    switch(keyType)
    {
    case KEY_SPACE:
        str = " ";
        // fall through
    case KEY_ALPHANUM:
        // check for key to validator
        if((param->validation.isEmpty ()) ||
                (str.contains (param->validation)))
        {
            newStr = newStr.insert (currCurPos, str);
            if(m_isCharCountExternallyControl)
            {
                if(currentCharLenght < param->maxChar)
                {
                    if(param->isNumEntry)
                    {
                        intValue = newStr.toUInt ();
                        if((intValue <= param->maxNumValue))
                        {
                            isAlphaNumKeyAccept = true;
                        }
                    }
                    else
                    {
                        isAlphaNumKeyAccept = true;
                    }
                }
            }
            else
            {
                if(newStr.length () <= param->maxChar)
                {
                    if(param->isNumEntry)
                    {
                        intValue = newStr.toUInt ();
                        if((intValue <= param->maxNumValue))
                        {
                            isAlphaNumKeyAccept = true;
                        }
                    }
                    else
                    {
                        isAlphaNumKeyAccept = true;
                    }
                }
            }
        }

        if(isAlphaNumKeyAccept == true)
        {
            param->textStr = newStr;
            currCurPos++;
            currentCharLenght++;
            lineEdit->setText (param->textStr);
            lineEdit->setCursorPosition  (currCurPos);
            emit sigTextValueAppended(param->textStr, m_indexInPage);
        }
        break;

    case KEY_ALPHANUM_SAME_INDEX:
        // check for key to validator
        if((param->validation.isEmpty ()) ||
                (str.contains (param->validation)))
        {
            newStr = newStr.remove (currCurPos - 1, 1);
            newStr = newStr.insert (currCurPos - 1, str);

            if(newStr.length () <= param->maxChar)
            {
                isAlphaNumKeyAccept = true;
            }
        }

        if(isAlphaNumKeyAccept == true)
        {
            param->textStr = newStr;
            lineEdit->setText (param->textStr);
            lineEdit->setCursorPosition  (currCurPos);
            emit sigTextValueAppended(param->textStr, m_indexInPage);
        }
        break;

    case KEY_CLEAR:
        currCurPos = 0;
        currentCharLenght = 0;
        param->textStr = "";
        lineEdit->setText (param->textStr);
        emit sigTextValueAppended(param->textStr, m_indexInPage);
        break;

    case KEY_BACKSPACE:
        if(currCurPos > 0)
        {
            currCurPos--;
            currentCharLenght--;
            param->textStr = param->textStr.remove (currCurPos, 1);
            lineEdit->setText (param->textStr);
            lineEdit->setCursorPosition  (currCurPos);
            emit sigTextValueAppended(param->textStr, m_indexInPage);
        }
        else
        {
            currCurPos = 0;
        }
        break;

    case KEY_LEFT_ARROW:
        if(currCurPos > 0)
        {
            currCurPos--;
        }
        else
        {
            currCurPos = 0;
        }
        lineEdit->setCursorPosition  (currCurPos);
        break;

    case KEY_RIGHT_ARROW:
        if(currCurPos < (param->textStr.length () - 1))
        {
            currCurPos++;
        }
        else
        {
            currCurPos = param->textStr.length ();
        }
        lineEdit->setCursorPosition  (currCurPos);
        break;

    case KEY_DONE:
        lineEdit->setText (param->textStr);
        lineEdit->setCursorPosition  (currCurPos);

        editMode = false;

        if(entryByRemote == false)
        {
            unloadVirtualKeypad();
        }
        entryByRemote = false;
        forceActiveFocus ();
        doneKeyValidation();

        //        emit sigTextValueAppended(param->textStr, m_indexInPage);
        break;

    case KEY_CLOSE:
        param->textStr = oldTextValue;
        currCurPos = 0;
        lineEdit->setText (param->textStr);
        lineEdit->setCursorPosition  (currCurPos);
        editMode = false;
        if(entryByRemote == false)
        {
            unloadVirtualKeypad();
        }
        entryByRemote = false;
        forceActiveFocus ();
        //        emit sigTextValueAppended(param->textStr, m_indexInPage);
        break;

    default:
        break;
    }
    update ();
}

void TextBox::slotInvisibleCtrlMouseClick ()
{
    if(true == m_mouseClicked)
    {
        m_mouseClicked = false;
        return;
    }
    if(virtualKeypad != NULL)
    {
        virtualKeypad->slotKeyDeteceted (KEY_DONE,0);
    }
}
