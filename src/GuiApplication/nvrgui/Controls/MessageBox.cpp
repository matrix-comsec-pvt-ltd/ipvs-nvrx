#include "MessageBox.h"
#include <QPainter>
#include <QTimer>
#include <QFontMetrics>
#include <QMouseEvent>
#include <QTextCursor>

#define MSGBOX_FOLDER_PATH      IMAGE_PATH"MessageBox/"


static const quint8 maxNumKeyRepeat[10] = { 2, 13, 4, 4, 4, 4, 4, 5, 4, 5};

static const QString numKeyLoweCaseRep [10][13] =
{
    {"0", " "},
    {",", ".", "/", "@", "%", "-", "_", "+", "?", "=",
     "&", "*", "1"},
    {"a", "b", "c", "2"},
    {"d", "e", "f", "3"},
    {"g", "h", "i", "4"},
    {"j", "k", "l", "5"},
    {"m", "n", "o", "6"},
    {"p", "q", "r", "s", "7"},
    {"t", "u", "v", "8"},
    {"w", "x", "y", "z", "9"}
};

static const QString numKeyUpperCaseRep [10][13] =
{
    {""},
    {""},
    {"A", "B", "C", "2"},
    {"D", "E", "F", "3"},
    {"G", "H", "I", "4"},
    {"J", "K", "L", "5"},
    {"M", "N", "O", "6"},
    {"P", "Q", "R", "S", "7"},
    {"T", "U", "V", "8"},
    {"W", "X", "Y", "Z", "9"}
};

MessageBox::MessageBox(quint32 startX,
                       quint32 startY,
                       quint32 width,
                       quint16 controlIndex,
                       QWidget *parent,
                       QString label,
                       BGTILE_TYPE_e bgType,
                       bool iscentre,
                       quint16 leftMgn,
                       quint16 maxChr,
                       QRegExp valdtion,
                       bool isNavigationEnable,
                       bool isCharCountExternallyControl,
                       bool isSpecialCharCountNeeded, quint32 leftMarginFromCenter)
    :BgTile(startX,
            startY,
            width,
            BGTILE_HEIGHT*2,
            bgType,
            parent),
      NavigationControl(controlIndex, isNavigationEnable), entryByRemote(false), remoteCapsLock(false),
      isCentre(iscentre), m_isCharCountExternallyControl(isCharCountExternallyControl), m_isSpecialFunDone(false),
      m_isSpecialCharCountNeeded(isSpecialCharCountNeeded), validation(valdtion), repeatKeyCnt(0), leftMargin(leftMgn),
      maxChar(maxChr),  labelStr(label), virtualKeypad(NULL), m_leftMarginFromCenter(leftMarginFromCenter)

{
    prevRemoteKey = Qt::Key_unknown;

    currentCharLenght = 0;
    editMode = false;
    indexOfControl = 0;
    isFocusIn = false;
    INIT_OBJ(m_inVisibleWidget);

    if(m_isEnabled)
    {
        imgType = IMAGE_TYPE_NORMAL;
    }
    else
    {
        imgType = IMAGE_TYPE_DISABLE;
    }
    imagePath = MSGBOX_FOLDER_PATH + imgTypePath[imgType];

    image = QPixmap(imagePath);
    SCALE_IMAGE(image);

    keyRepTimer = new QTimer(this);
    connect (keyRepTimer,
             SIGNAL(timeout()),
             this,
             SLOT(slotkeyRepTimerTimeout()));

    keyRepTimer->setInterval (1000);
    keyRepTimer->setSingleShot (true);

    createDefaultElement();

    this->setMouseTracking (true);
    this->setEnabled (m_isEnabled);    
    textEdit->setEnabled (m_isEnabled);        
    this->show ();
}


MessageBox::~MessageBox()
{
  slotInvisibleCtrlMouseClick ();
    if(labelStr != "")
    {
        delete labelText;
    }
    delete keyRepTimer;
    delete palette;

    disconnect (textEdit,
                SIGNAL(sigFocusChange(bool,bool)),
                this,
                SLOT(slotTextEditFocusChange(bool,bool)));
    delete textEdit;    
}

void MessageBox::createDefaultElement()
{
    quint16 labelWidth = 0, translatedlabelWidth = 0;
    quint16 width = 0;
    qint8 verticalOffset = 0;
    QFont labelFont;
    currCurPos = 0;

    if(labelStr != "")
    {
        labelFont = TextLabel::getFont (NORMAL_FONT_FAMILY, NORMAL_FONT_SIZE);
        labelWidth = QFontMetrics(labelFont).width (labelStr);
        translatedlabelWidth = QFontMetrics(labelFont).width (QApplication::translate(QT_TRANSLATE_STR, labelStr.toUtf8().constData()));
        width += SCALE_WIDTH(10);
    }

    imgType = (m_isEnabled == true) ?
                (IMAGE_TYPE_NORMAL) : IMAGE_TYPE_DISABLE;

    imagePath = MSGBOX_FOLDER_PATH + imgTypePath[imgType];

    image = QPixmap(imagePath);
    SCALE_IMAGE(image);

    width += image.width () + labelWidth ;

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

    if(isCentre == true)
    {
        if(labelStr != "")
        {
            labelWidth = (translatedlabelWidth > ((getWidth()/2) - SCALE_WIDTH(20)))? ((getWidth()/2) - SCALE_WIDTH(20)) : translatedlabelWidth;
            labelText = new TextLabel(((this->width ()/2) - SCALE_WIDTH(10) - labelWidth) - m_leftMarginFromCenter,
                                      SCALE_HEIGHT(10) + verticalOffset,
                                      NORMAL_FONT_SIZE,labelStr,
                                      this, NORMAL_FONT_COLOR, NORMAL_FONT_FAMILY, ALIGN_START_X_START_Y,
                                      0, 0, labelWidth, 0, 0, Qt::AlignRight);
        }


        imgRect.setRect (this->width ()/2 - m_leftMarginFromCenter,
                         ((this->height () - image.height ())/2) + verticalOffset,
                         image.width (),
                         image.height ());
    }
    else
    {
        if(labelStr != "")
        {
            translatedlabelWidth = (translatedlabelWidth > ((leftMargin + labelWidth) - SCALE_WIDTH(17))) ? ((leftMargin + labelWidth) - SCALE_WIDTH(17)) : (translatedlabelWidth);
            labelText = new TextLabel(abs((abs(translatedlabelWidth - (leftMargin + labelWidth))) - SCALE_WIDTH(5)),
                                      SCALE_HEIGHT(10) + verticalOffset,
                                      NORMAL_FONT_SIZE,labelStr,
                                      this,
                                      NORMAL_FONT_COLOR, NORMAL_FONT_FAMILY, ALIGN_START_X_START_Y,
                                      0, 0, translatedlabelWidth, 0, 0);
        }


        imgRect.setRect (leftMargin+labelWidth+ SCALE_WIDTH(10),
                         (this->height () - image.height ())/2 + verticalOffset,
                         image.width (),
                         image.height ());
    }

    textEdit = new TextEdit(imgRect.x () + SCALE_WIDTH(15),
                            imgRect.y () + SCALE_HEIGHT(4),
                            (imgRect.width () - SCALE_WIDTH(30)),
                            (imgRect.height () - SCALE_HEIGHT(8)) ,
                            this);

    connect (textEdit,
             SIGNAL(sigFocusChange(bool,bool)),
             this,
             SLOT(slotTextEditFocusChange(bool,bool)));

    palette = new QPalette();

    if(m_isEnabled)
    {
        palette->setColor(QPalette::Text, QColor(NORMAL_FONT_COLOR).lighter (200));

    }
    else
    {
        palette->setColor(QPalette::Text, QColor(DISABLE_FONT_COLOR));
    }

    textStr = " ";

    palette->setColor(QPalette::Base, QColor(Qt::transparent));
    textEdit->setPalette(*palette);
    textEdit->setFont (labelFont);
    textEdit->setText (textStr);
    textEdit->setCurrentCursorPosition (currCurPos);
}

void MessageBox::forceActiveFocus()
{
    if(m_isEnabled)
    {
        this->setFocus();
    }
}

void MessageBox::focusInEvent(QFocusEvent *)
{
    if((virtualKeypad != NULL) || (entryByRemote == true))
    {
        textEdit->setActiveFocus ();
    }
    else
    {
        selectControl();
    }
}

void MessageBox::unloadVirtualKeyboard()
{
    if(m_catchKey == false)
    {
        unloadVirtualKeypad();
        entryByRemote = true;
        editMode = true;
//        m_isControlActivated = false;
    }
}

void MessageBox::focusOutEvent(QFocusEvent *)
{
    deSelectControl();
}

void MessageBox::selectControl()
{
    if(imgType != IMAGE_TYPE_MOUSE_HOVER)
    {
        changeImage(IMAGE_TYPE_MOUSE_HOVER);
    }
}

void MessageBox::deSelectControl()
{
    if(imgType != IMAGE_TYPE_NORMAL)
    {
        changeImage(IMAGE_TYPE_NORMAL);
    }
}

void MessageBox::changeImage (IMAGE_TYPE_e imageType)
{
    imgType = imageType;
    imagePath = MSGBOX_FOLDER_PATH
            + imgTypePath[imgType];

    image = QPixmap(imagePath);
    SCALE_IMAGE(image);
    update ();
}

void MessageBox::paintEvent (QPaintEvent *event)
{
    QWidget::paintEvent(event);
    BgTile::paintEvent(event);

    QPainter painter(this);
    painter.drawPixmap (imgRect, image);
}

void MessageBox::navigationKeyPressed(QKeyEvent *event)
{
    if((m_catchKey == true) && (entryByRemote == true))
    {
        textEdit->navigationKeyPressed(event);
        currCurPos = textEdit->getCurrentCurrsorPosition();
    }
    else
    {
        QWidget::keyPressEvent(event);
    }
}

void MessageBox::backspaceKeyPressed(QKeyEvent *event)
{
    if((m_catchKey) && (entryByRemote == true))
    {
        event->accept();
        m_isSpecialFunDone = false;
        textEdit->backspaceKeyPressed(event);
        textStr = textEdit->toPlainText();
        currCurPos = textEdit->getCurrentCurrsorPosition();
        currentCharLenght = textStr.length();
        emit sigTextValueAppended(textStr, m_indexInPage);
    }
}

void MessageBox::enterKeyPressed(QKeyEvent *event)
{
    if(m_catchKey)
    {
        event->accept();
        if((event->modifiers() & Qt::ControlModifier) == Qt::ControlModifier)
        {
            if(entryByRemote == true)
            {
                if(textEdit->hasSelectedText())
                {
                    textEdit->insertText(QString(""));
                    textStr = textEdit->toPlainText();
                    currCurPos = textEdit->getCurrentCurrsorPosition();
                    currentCharLenght = textStr.length();
                    emit sigTextValueAppended(textStr, m_indexInPage);
                }
                slotTextBoxKeyPressed(KEY_ALPHANUM,QString("\n"));
            }
        }
        else
        {
            m_isSpecialFunDone = false;
            if(entryByRemote == true)
            {
                m_isControlActivated = true;
                slotTextBoxKeyPressed(KEY_DONE, "");
            }
            else
            {
                editMode = true;
                m_isControlActivated = false;
                if(imgType != IMAGE_TYPE_MOUSE_HOVER)
                {
                    changeImage(IMAGE_TYPE_MOUSE_HOVER);
                }
                mouseClickOnBox(true);
                textEdit->navigationKeyPressed(event);
            }
        }
    }
}

void MessageBox::deleteKeyPressed(QKeyEvent *event)
{
    if((m_catchKey) && (entryByRemote == true))
    {
        event->accept();
        m_isSpecialFunDone = false;
        textEdit->deleteKeyPressed(event);
        textStr = textEdit->toPlainText();
        currCurPos = textEdit->getCurrentCurrsorPosition();
        currentCharLenght = textStr.length();
        emit sigTextValueAppended(textStr, m_indexInPage);
    }
}

void MessageBox::escKeyPressed(QKeyEvent *event)
{
    if((m_catchKey) && (entryByRemote == true))
    {
        event->accept();
        m_isSpecialFunDone = false;
        m_isControlActivated = true;
        slotTextBoxKeyPressed(KEY_CLOSE, "");
        currCurPos = textStr.length ();
    }
    else
    {
        QWidget::keyPressEvent(event);
    }
}

void MessageBox::asciiCharKeyPressed(QKeyEvent *event)
{
    if((m_catchKey) && (entryByRemote == true))
    {
        event->accept();
        if(textEdit->hasSelectedText())
        {
            textEdit->insertText(QString(""));
            textStr = textEdit->toPlainText();
            currCurPos = textEdit->getCurrentCurrsorPosition();
            currentCharLenght = textStr.length();
            emit sigTextValueAppended(textStr, m_indexInPage);
        }
        slotTextBoxKeyPressed(KEY_ALPHANUM,event->text());
    }
    else
    {
        QWidget::keyPressEvent(event);
    }
}

void MessageBox::ctrl_A_KeyPressed(QKeyEvent *event)
{
    if((m_catchKey) && (entryByRemote == true))
    {
        event->accept();
        textEdit->selectAllText();
        currCurPos = textEdit->getCurrentCurrsorPosition();
    }
}

void MessageBox::ctrl_C_KeyPressed(QKeyEvent *event)
{
    if((m_catchKey) && (entryByRemote == true))
    {
        event->accept();
        m_isSpecialFunDone = false;
        textEdit->copy();
    }
}

void MessageBox::ctrl_V_KeyPressed(QKeyEvent *event)
{
    if((m_catchKey) && (entryByRemote == true) && (!m_isSpecialFunDone))
    {
        event->accept();        
        quint16 strLen = countStrLenWithSpChr(textStr);       
        if(strLen < maxChar)
        {
            textEdit->paste();
            textStr = textEdit->toPlainText();
            strLen = countStrLenWithSpChr(textStr);            
            if(strLen > maxChar)
            {
                quint16 diff = countActualCharLenToTruncate(textStr, (strLen - maxChar));
                textEdit->truncateText(diff);
                textStr = textEdit->toPlainText();
                m_isSpecialFunDone = true;
            }
            currCurPos = textEdit->getCurrentCurrsorPosition();
            currentCharLenght = textStr.length();
            emit sigTextValueAppended(textStr, m_indexInPage);
        }
    }
}

void MessageBox::ctrl_X_KeyPressed(QKeyEvent *event)
{
    if((m_catchKey) && (entryByRemote == true))
    {
        event->accept();
        m_isSpecialFunDone = false;
        textEdit->cut();
        textStr = textEdit->toPlainText();
        currCurPos = textEdit->getCurrentCurrsorPosition();
        currentCharLenght = textStr.length();
        emit sigTextValueAppended(textStr, m_indexInPage);
    }
}

void MessageBox::ctrl_Y_KeyPressed(QKeyEvent *event)
{
    if((m_catchKey) && (entryByRemote == true))
    {
        event->accept();
        quint16 strLen;

        textEdit->redo();
        textStr = textEdit->toPlainText();
        strLen = countStrLenWithSpChr(textStr);
        if(strLen > maxChar)
        {
            textEdit->redo();
            textStr = textEdit->toPlainText();
        }
        currCurPos = textEdit->getCurrentCurrsorPosition();
        currentCharLenght = textStr.length();
        emit sigTextValueAppended(textStr, m_indexInPage);
    }
}

void MessageBox::ctrl_Z_KeyPressed(QKeyEvent *event)
{
    if((m_catchKey) && (entryByRemote == true))
    {       
        event->accept();
        quint16 strLen;

        textEdit->undo();
        textStr = textEdit->toPlainText();
        strLen = countStrLenWithSpChr(textStr);
        if(strLen > maxChar)
        {
            textEdit->undo();
            textStr = textEdit->toPlainText();
        }
        if(m_isSpecialFunDone)
        {
            textEdit->undo();
            textStr = textEdit->toPlainText();
            strLen = countStrLenWithSpChr(textStr);
            if(strLen > maxChar)
            {
                textEdit->undo();
                textStr = textEdit->toPlainText();
            }
        }
        m_isSpecialFunDone = false;
        currCurPos = textEdit->getCurrentCurrsorPosition();
        currentCharLenght = textStr.length();
        emit sigTextValueAppended(textStr, m_indexInPage);
    }
}

void MessageBox::take0to9KeyAction(int key)
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

quint16 MessageBox::countStrLenWithSpChr(QString str)
{
    quint16 totalChar = str.length ();

    if(m_isSpecialCharCountNeeded)
    {
        totalChar += (str.count ("%E") * (MAX_EVENT_STRING_LENGTH - 2));
        totalChar += (str.count ("%D") * (MAX_DATE_STRING_LENGTH - 2));
        totalChar += (str.count ("%T") * (MAX_TIME_STRING_LENGTH - 2));
    }

    return totalChar;
}

quint16 MessageBox::countActualCharLenToTruncate(QString str, quint16 count)
{
    quint16 actualCnt = 0, strLen = str.length();
    QRegExp identifier = QRegExp(QString("%[EDT]"));

    if(str.contains(identifier))
    {
        int         pos = 0;
        quint16     tempCnt = 0;
        quint16     charVal = 0;
        bool        cntCalculated = false;

        do
        {
            pos--;
            pos = str.lastIndexOf(identifier, pos);
            if((strLen - pos + tempCnt) < count)
            {
                if(str.at(pos+1) == 'E')
                {
                    charVal = (MAX_EVENT_STRING_LENGTH - 2);
                }
                else if(str.at(pos+1) == 'D')
                {
                    charVal = (MAX_DATE_STRING_LENGTH - 2);
                }
                else if(str.at(pos+1) == 'T')
                {
                    charVal = (MAX_TIME_STRING_LENGTH - 2);
                }

                if((strLen - pos + tempCnt + charVal) > count)
                {
                    actualCnt = strLen - pos; // if want to display % do -1;
                    cntCalculated = true;
                    break;
                }
                tempCnt += charVal;
            }
        } while((strLen - pos + tempCnt) < count);

        if(!cntCalculated)
        {
            actualCnt = count - tempCnt;
        }
    }
    else
    {
        actualCnt = count;
    }

    return actualCnt;
}

void MessageBox::mouseMoveEvent (QMouseEvent *event)
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

void MessageBox::hideEvent (QHideEvent *event)
{
    QWidget::hideEvent (event);
    if(virtualKeypad != NULL)
    {
        m_inVisibleWidget->setVisible (false);
        virtualKeypad->setVisible (false);
    }
}

void MessageBox::showEvent (QShowEvent * event)
{
    QWidget::showEvent (event);
    if(virtualKeypad != NULL)
    {
        m_inVisibleWidget->setVisible (true);
        virtualKeypad->setVisible (true);
    }
}

void MessageBox::mousePressEvent (QMouseEvent *event)
{
    if((imgRect.contains(event->pos())) &&
            (event->button () == m_leftMouseButton))
    {
        entryByRemote = false;
        m_isControlActivated = true;
        mouseClickOnBox ();
    }
}

void MessageBox::slotTextEditFocusChange(bool tIsFocusIn, bool forceFocus)
{
    editMode = tIsFocusIn;
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

void MessageBox::mouseClickOnBox (bool isByRemote)
{
    if(imgType != IMAGE_TYPE_MOUSE_HOVER)
    {
        changeImage(IMAGE_TYPE_MOUSE_HOVER);
    }

    oldTextValue = textStr;

    textEdit->setActiveFocus ();

    if(isByRemote == true)
    {
        entryByRemote = true;
    }
    else
    {
        entryByRemote = false;
        if(virtualKeypad == NULL)
        {
            /* PARASOFT: Memory Deallocated in unload virtual keypad */
            m_inVisibleWidget = new InvisibleWidgetCntrl(this->window());
            connect (m_inVisibleWidget,
                     SIGNAL(sigMouseClick()),
                     this,
                     SLOT(slotInvisibleCtrlMouseClick()));

            /* PARASOFT: Memory Deallocated in unload virtual keypad */
            virtualKeypad = new VirtualKeypad(SCALE_WIDTH(618), SCALE_HEIGHT(800), this->window ());
            connect (virtualKeypad,
                     SIGNAL(sigKeyDetected(KEY_TYPE_e,QString)),
                     this,
                     SLOT(slotTextBoxKeyPressed(KEY_TYPE_e,QString)));
        }
        setCatchKey (false);
        currCurPos = textEdit->getCurrentCurrsorPosition();
    }
    textEdit->setCurrentCursorPosition (currCurPos);
}

QString MessageBox:: getInputText()
{
    return textStr;
}

void MessageBox:: setInputText(QString str)
{
    textStr = str;
    textEdit->setText (textStr);
    currCurPos = textStr.length ();
    currentCharLenght = textStr.length ();
    textEdit->setCurrentCursorPosition (currCurPos);
    update ();
}

void MessageBox::setCurrentCharLenght (quint8 maxCharCount)
{
    currentCharLenght = maxCharCount;
}

void MessageBox::setIsEnabled (bool isEnable)
{
    if(m_isEnabled != isEnable)
    {
        m_isEnabled = isEnable;
        textEdit->setEnabled (m_isEnabled);
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
        textEdit->setPalette(*palette);
        update();
    }
}

void MessageBox::unloadVirtualKeypad()
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

void MessageBox::slotkeyRepTimerTimeout ()
{
    prevRemoteKey = Qt::Key_unknown;
    repeatKeyCnt = 0;
}

void MessageBox::slotTextBoxKeyPressed(KEY_TYPE_e keyType, QString str)
{
    QString newStr = textStr;
    bool isAlphaNumKeyAccept = false;

    switch(keyType)
    {
    case KEY_SPACE:
        str = " ";
        // fall through
    case KEY_ALPHANUM:

        if(str.contains (validation))
        {
            newStr = newStr.insert (currCurPos, str);
            if(m_isCharCountExternallyControl)
            {
                if(currentCharLenght < maxChar)
                {
                    isAlphaNumKeyAccept = true;
                }
            }
            else
            {
                if(newStr.length () <= maxChar)
                {
                    isAlphaNumKeyAccept = true;
                }
            }

            if(countStrLenWithSpChr(newStr) > maxChar)
            {
                isAlphaNumKeyAccept = false;
            }
        }

        if(isAlphaNumKeyAccept == true)
        {
            textEdit->insertText(str);
            textStr = textEdit->toPlainText();
            currCurPos = textEdit->getCurrentCurrsorPosition();
            currentCharLenght = textStr.length();
            emit sigTextValueAppended(textStr, m_indexInPage);
        }
        break;

    case KEY_ALPHANUM_SAME_INDEX:
        if(str.contains (validation))
        {
            newStr = newStr.remove (currCurPos - 1, 1);
            newStr = newStr.insert (currCurPos - 1, str);

            if(newStr.length () <= maxChar)
            {
                isAlphaNumKeyAccept = true;
            }
        }

        if(isAlphaNumKeyAccept == true)
        {
            textStr = newStr;
            textEdit->setText  (textStr);
            textEdit->setCurrentCursorPosition (currCurPos);
            emit sigTextValueAppended(textStr, m_indexInPage);
        }
        break;

    case KEY_CLEAR:
        currCurPos = 0;
        currentCharLenght = 0;
        textStr = "";
        textEdit->setText  (textStr);
        emit sigTextValueAppended(textStr, m_indexInPage);
        break;

    case KEY_BACKSPACE:
        if(currCurPos > 0)
        {
            currCurPos--;
            currentCharLenght--;
            textStr = textStr.remove (currCurPos, 1);
            textEdit->setText  (textStr);
            textEdit->setCurrentCursorPosition (currCurPos);
            emit sigTextValueAppended(textStr, m_indexInPage);
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
        textEdit->setCurrentCursorPosition (currCurPos);

        break;

    case KEY_RIGHT_ARROW:
        if(currCurPos < (textStr.length () - 1))
        {
            currCurPos++;
        }
        else
        {
            currCurPos = textStr.length ();
        }
        textEdit->setCurrentCursorPosition (currCurPos);

        break;

    case KEY_DONE:
        currCurPos = 0;
        textEdit->setText  (textStr);
        textEdit->setCurrentCursorPosition (currCurPos);

        editMode = false;

        if(entryByRemote == false)
        {
            unloadVirtualKeypad();
        }
        entryByRemote = false;
        forceActiveFocus ();

        emit sigTextValueAppended(textStr, m_indexInPage);

        break;

    case KEY_CLOSE:

        textStr = oldTextValue;
        currCurPos = 0;
        textEdit->setText  (textStr);
        textEdit->setCurrentCursorPosition (currCurPos);
        emit sigTextValueAppended(textStr, m_indexInPage);

        editMode = false;

        if(entryByRemote == false)
        {
            unloadVirtualKeypad();
        }
        entryByRemote = false;
        forceActiveFocus ();

        break;

    case KEY_ENTER:
        str = "\n";
        newStr = newStr.insert (currCurPos, str);
        if(m_isCharCountExternallyControl)
        {
            if(currentCharLenght < maxChar)
            {
                isAlphaNumKeyAccept = true;
            }
        }
        else
        {
            if(newStr.length () <= maxChar)
            {
                isAlphaNumKeyAccept = true;
            }
        }

        if(isAlphaNumKeyAccept == true)
        {
            textStr = newStr;
            currCurPos++;
            currentCharLenght++;
            textEdit->setText  (textStr);
            textEdit->setCurrentCursorPosition (currCurPos);
            emit sigTextValueAppended(textStr, m_indexInPage);
        }
        break;

    default:
        break;
    }
    update ();
}

void MessageBox::slotInvisibleCtrlMouseClick ()
{
    if(virtualKeypad != NULL)
    {
        virtualKeypad->slotKeyDeteceted (KEY_DONE,0);
    }
}
