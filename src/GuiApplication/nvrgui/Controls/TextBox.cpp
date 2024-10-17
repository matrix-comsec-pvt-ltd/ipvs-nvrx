#include <QFontMetrics>
#include <QMouseEvent>
#include <QTimer>
#include <QPaintEvent>
#include <QPainter>
#include <QIntValidator>

#include "EnumFile.h"
#include "TextBox.h"
#include "ApplController.h"
#include "Controls/MenuButton.h"

#define TEXTBOX_FOLDER_PATH      IMAGE_PATH "Textbox/"
#define WIDTH_PADDING            SCALE_WIDTH(10)

const QString textboxBtnFolder[MAX_TEXTBOX_SIZE] =
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

const quint8 maxNumKeyRepeat[10] = {2, 13, 4, 4, 4, 4, 4, 5, 4, 5};

const QString numKeyLoweCaseRep[10][13] =
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

const QString numKeyUpperCaseRep[10][13] =
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
                 bool isSpecialCharCountNeeded,
                 quint32 leftMarginFromCenter)
    : BgTile(startX, startY, width, height, bgType, parent), NavigationControl(controlIndex, isNavigationEnable),
      indexOfControl(controlIndex), prevRemoteKey(Qt::Key_unknown), editMode(false), repeatKeyCnt(0),
      textBoxSize(size), entryByRemote(false), remoteCapsLock(false), param(textBoxParam),
      m_isCharCountExternallyControl(isCharCountExternallyControl), m_isSpecialFunDone(false),
      m_isSpecialCharCountNeeded(isSpecialCharCountNeeded), m_leftMarginFromCenter(leftMarginFromCenter)
{
    oldTextValue = param->textStr;
    currCurPos = 0;
    currentCharLenght = 0;
    INIT_OBJ(virtualKeypad);
    INIT_OBJ(m_inVisibleWidget);
    INIT_OBJ(m_physicalKeyboardInVisibleWidget);

    if(param->isNumEntry == true)
    {
        param->validation = QRegExp(QString("[0-9]"));
    }

    if ((param->isEmailAddrType == true) && (param->startCharVal == QRegExp("")))
    {
        param->startCharVal = QRegExp(QString("[a-zA-Z0-9]"));
    }

    reloadListNeedded = true;
    imgType =(m_isEnabled) ? IMAGE_TYPE_NORMAL : IMAGE_TYPE_DISABLE;
    imagePath = TEXTBOX_FOLDER_PATH + textboxBtnFolder[textBoxSize] + imgTypePath[imgType];
    image = QPixmap(imagePath);
    SCALE_IMAGE(image);

    keyRepTimer = new QTimer(this);
    connect(keyRepTimer,
            SIGNAL(timeout()),
            this,
            SLOT(slotkeyRepTimerTimeout()));
    keyRepTimer->setInterval(1000);
    keyRepTimer->setSingleShot(true);

    createDefaultElement();

    lineEdit->setEnabled(m_isEnabled);

    this->setMouseTracking(true);
    this->setEnabled(m_isEnabled);
    this->show();
}

TextBox::~TextBox()
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

    delete lineEdit;

    if(IS_VALID_OBJ(virtualKeypad))
    {
        disconnect(virtualKeypad,
                   SIGNAL(sigKeyDetected(KEY_TYPE_e,QString)),
                   this,
                   SLOT(slotTextBoxKeyPressed(KEY_TYPE_e,QString)));
        DELETE_OBJ(virtualKeypad);

        disconnect(m_inVisibleWidget,
                   SIGNAL(sigMouseClick()),
                   this,
                   SLOT(slotInvisibleCtrlMouseClick()));
        DELETE_OBJ(m_inVisibleWidget);
        NavigationControl::setCatchKey(true);
    }
    else
    {
        deletePhysicalKeyboardWidget();
    }
}

void TextBox::changeImage(IMAGE_TYPE_e imageType)
{
    imgType = imageType;
    imagePath = TEXTBOX_FOLDER_PATH + textboxBtnFolder[textBoxSize] + imgTypePath[imgType];
    image = QPixmap(imagePath);
    SCALE_IMAGE(image);
    update();
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

void TextBox::forceActiveFocus()
{
    if(m_isEnabled)
    {
        this->setFocus();
    }
}

void TextBox::paintEvent(QPaintEvent *event)
{
    QWidget::paintEvent(event);
    BgTile::paintEvent(event);

    QPainter painter(this);
    painter.drawPixmap(imgRect, image);
}

void TextBox::focusInEvent(QFocusEvent *)
{
    if((virtualKeypad != NULL) || (entryByRemote == true))
    {
        lineEdit->setActiveFocus();
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

void TextBox::createDefaultElement()
{
    quint16 labelWidth = 0,suffixWidth = 0, labelHeight = 0, strHeight = 0, translatedlabelWidth = 0;
    quint16 width = 0;
    qint8 verticalOffset = 0;
    QFont labelFont = TextLabel::getFont(NORMAL_FONT_FAMILY, NORMAL_FONT_SIZE);
    QFont suffixFont;

    if(param->labelStr != "")
    {
        labelFont = TextLabel::getFont(NORMAL_FONT_FAMILY, NORMAL_FONT_SIZE);
        translatedlabelWidth = QFontMetrics(labelFont).width(QApplication::translate(QT_TRANSLATE_STR, param->labelStr.toUtf8().constData()));
        labelWidth = QFontMetrics(labelFont).width(param->labelStr);
        labelHeight = QFontMetrics(labelFont).height();
        width += WIDTH_PADDING;
    }

    if(param->suffixStr != "")
    {
        suffixFont = TextLabel::getFont(NORMAL_FONT_FAMILY, SCALE_FONT(SUFFIX_FONT_SIZE));
        suffixWidth = QFontMetrics(suffixFont).width(param->suffixStr);
        strHeight = QFontMetrics(suffixFont).height();
        width += WIDTH_PADDING;
    }

    width += image.width() + labelWidth + suffixWidth;

    switch(m_bgTileType)
    {
        case NO_LAYER:
        {
            if(true == param->isForceCenter)
            {
                break;
            }
            m_width = width;
            this->setGeometry(m_startX, m_startY, m_width, m_height);
            param->isCentre = false;
        }
        break;

        case TOP_TABLE_LAYER:
        {
            verticalOffset =(TOP_MARGIN / 2);
        }
        break;

        case BOTTOM_TABLE_LAYER:
        {
            verticalOffset = -(TOP_MARGIN / 2);
        }
        break;

        default:
        {
            /* Nothing to do */
        }
        break;
    }

    if(param->isCentre == true)
    {
        if(param->labelStr != "")
        {
            labelWidth =(translatedlabelWidth >((getWidth()/2) - SCALE_WIDTH(20)))?((getWidth()/2) - SCALE_WIDTH(20)) : translatedlabelWidth;
            labelText = new TextLabel(((getWidth()/2) - WIDTH_PADDING - labelWidth) - m_leftMarginFromCenter,
                                      (this->height() - labelHeight)/2 + verticalOffset,
                                      NORMAL_FONT_SIZE,param->labelStr,
                                      this, NORMAL_FONT_COLOR, NORMAL_FONT_FAMILY, ALIGN_START_X_START_Y,
                                      0, 0, labelWidth, 0, 0, Qt::AlignRight);
        }

        if(param->suffixStr != "")
        {
            suffixText = new TextLabel((this->width()/2) + image.width() +WIDTH_PADDING - m_leftMarginFromCenter,
                                       (this->height() - strHeight)/2 +verticalOffset,
                                       SCALE_FONT(SUFFIX_FONT_SIZE), param->suffixStr,
                                       this,
                                       SUFFIX_FONT_COLOR, NORMAL_FONT_FAMILY, ALIGN_START_X_START_Y,
                                       0, 0, suffixWidth, 0, 0, Qt::AlignRight);
        }

        imgRect.setRect(this->width()/2 - m_leftMarginFromCenter,
                        ((this->height() - image.height())/2) + verticalOffset,
                        image.width(), image.height());
    }
    else
    {
        if(param->labelStr != "")
        {
            translatedlabelWidth =(translatedlabelWidth >((param->leftMargin + labelWidth))) ?((param->leftMargin + labelWidth) - SCALE_WIDTH(16)) :(translatedlabelWidth);
            labelText = new TextLabel(abs((abs(translatedlabelWidth -(param->leftMargin + labelWidth))) - SCALE_WIDTH(5)),
                                      (this->height() - labelHeight)/2 + verticalOffset,
                                      NORMAL_FONT_SIZE,param->labelStr,
                                      this,
                                      NORMAL_FONT_COLOR, NORMAL_FONT_FAMILY, ALIGN_START_X_START_Y,
                                      0, 0, translatedlabelWidth, 0, 0, Qt::AlignRight);
            labelWidth += WIDTH_PADDING;
        }

        imgRect.setRect(param->leftMargin + labelWidth,
                        (this->height() - image.height())/2 + verticalOffset,
                        image.width(), image.height());

        if(param->suffixStr != "")
        {
            suffixText = new TextLabel(param->leftMargin + labelWidth + image.width() + WIDTH_PADDING,
                                       (this->height() - strHeight)/2 + verticalOffset,
                                       SCALE_FONT(SUFFIX_FONT_SIZE), param->suffixStr,
                                       this,
                                       SUFFIX_FONT_COLOR, NORMAL_FONT_FAMILY, ALIGN_START_X_START_Y,
                                       0, 0, suffixWidth, 0, 0, Qt::AlignRight);
        }
    }

    lineEdit = new LineEdit(imgRect.x() + WIDTH_PADDING,
                            imgRect.y(),
                            (imgRect.width() - SCALE_WIDTH(20)),
                            imgRect.height(),
                            this);
    lineEdit->setFrame(false);

    palette = new QPalette();
    if(m_isEnabled)
    {
        palette->setColor(QPalette::Text, QColor(NORMAL_FONT_COLOR).lighter(200));
    }
    else
    {
        palette->setColor(QPalette::Text, QColor(QString(DISABLE_FONT_COLOR)));
    }

    palette->setColor(QPalette::Base, QColor(Qt::transparent));
    lineEdit->setPalette(*palette);
    lineEdit->setFont(labelFont);

    lineEdit->setText(param->textStr);
    lineEdit->setMaxLength(param->maxChar);
    if(param->isNumEntry)
    {
        QValidator *validator = new QIntValidator(param->minNumValue, param->maxNumValue, this);
        lineEdit->setValidator(validator);
    }
}

void TextBox::setIsEnabled(bool isEnable)
{
    if (isEnable == m_isEnabled)
    {
        return;
    }

    m_isEnabled = isEnable;
    lineEdit->setEnabled(m_isEnabled);
    this->setEnabled(m_isEnabled);

    if(isEnable == true)
    {
        palette->setColor(QPalette::Text, QColor(NORMAL_FONT_COLOR).lighter(200));
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

void TextBox::take0to9KeyAction(int key)
{
    QString tempStr = "";

    if(prevRemoteKey == key)
    {
        repeatKeyCnt = ((repeatKeyCnt + 1) % maxNumKeyRepeat[key - Qt::Key_0]);
        if(keyRepTimer->isActive())
        {
            keyRepTimer->stop();
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
        slotTextBoxKeyPressed(KEY_ALPHANUM_SAME_INDEX,tempStr);
        keyRepTimer->start();
    }
    else
    {
        repeatKeyCnt = 0;
        if(keyRepTimer->isActive())
        {
            keyRepTimer->stop();
        }

        if(remoteCapsLock == true)
        {
            tempStr = numKeyUpperCaseRep[key - Qt::Key_0][repeatKeyCnt];
        }
        else
        {
            tempStr = numKeyLoweCaseRep[key - Qt::Key_0][repeatKeyCnt];
        }

        // send string according to key, repeat cnt
        slotTextBoxKeyPressed(KEY_ALPHANUM, tempStr);

        prevRemoteKey = key;
        keyRepTimer->start();
    }
}

void TextBox::mouseMoveEvent(QMouseEvent *event)
{
    if(imgRect.contains(event->pos()) && (m_isControlActivated))
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

void TextBox::hideEvent(QHideEvent *event)
{
    QWidget::hideEvent(event);
    if (IS_VALID_OBJ(virtualKeypad))
    {
        m_inVisibleWidget->setVisible(false);
        virtualKeypad->setVisible(false);
    }
}

void TextBox::showEvent(QShowEvent * event)
{
    QWidget::showEvent(event);
    if (IS_VALID_OBJ(virtualKeypad))
    {
        m_inVisibleWidget->setVisible(true);
        virtualKeypad->setVisible(true);
    }
}

void TextBox::mousePressEvent(QMouseEvent *event)
{
    if((imgRect.contains(event->pos())) && (event->button() == m_leftMouseButton))
    {
        entryByRemote = false;
        m_isControlActivated = true;

        if(imgType != IMAGE_TYPE_MOUSE_HOVER)
        {
            changeImage(IMAGE_TYPE_MOUSE_HOVER);
        }
        mouseClickOnBox();
    }
}

void TextBox::mouseClickOnBox(bool isByRemote)
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
            connect(m_inVisibleWidget,
                     SIGNAL(sigMouseClick()),
                     this,
                     SLOT(slotInvisibleCtrlMouseClick()));

            virtualKeypad = new VirtualKeypad(SCALE_WIDTH(618), SCALE_HEIGHT(800), this->window());
            connect(virtualKeypad,
                     SIGNAL(sigKeyDetected(KEY_TYPE_e,QString)),
                     this,
                     SLOT(slotTextBoxKeyPressed(KEY_TYPE_e,QString)));
        }
        setCatchKey(false);
    }

    lineEdit->setActiveFocus();
    currCurPos = lineEdit->cursorPosition();
}

void TextBox::getInputText(QString &str)
{
    str = param->textStr;
}

QString TextBox::getInputText() const
{
    return param->textStr;
}

void TextBox::setInputText(QString str)
{
    param->textStr = str;
    currCurPos = 0;
    lineEdit->setText(param->textStr);
    lineEdit->setCursorPosition (currCurPos);
    currentCharLenght = str.length();
    update();
}

void TextBox::raiseVirtualKeypad()
{
    if(IS_VALID_OBJ(virtualKeypad))
    {
        m_inVisibleWidget->raise();
        virtualKeypad->raise();
    }
}

quint16 TextBox::countStrLenWithSpChr(QString str)
{
    quint16 totalChar = str.length();

    if(m_isSpecialCharCountNeeded)
    {
        totalChar +=(str.count("%E") *(MAX_EVENT_STRING_LENGTH - 2));
        totalChar +=(str.count("%D") *(MAX_DATE_STRING_LENGTH - 2));
        totalChar +=(str.count("%T") *(MAX_TIME_STRING_LENGTH - 2));
    }

    return totalChar;
}

quint16 TextBox::countActualCharLenToTruncate(QString str, quint16 count)
{
    bool    cntCalculated = false;
    qint32  pos = 0;
    quint16 actualCnt = 0, tempCnt = 0, charVal = 0, strLen = str.length();
    QRegExp identifier = QRegExp(QString("%[EDT]"));

    if (false == str.contains(identifier))
    {
        return count;
    }

    do
    {
        pos--;
        pos = str.lastIndexOf(identifier, pos);
        if ((strLen - pos + tempCnt) >= count)
        {
            continue;
        }

        if(str.at(pos+1) == 'E')
        {
            charVal =(MAX_EVENT_STRING_LENGTH - 2);
        }
        else if(str.at(pos+1) == 'D')
        {
            charVal =(MAX_DATE_STRING_LENGTH - 2);
        }
        else if(str.at(pos+1) == 'T')
        {
            charVal =(MAX_TIME_STRING_LENGTH - 2);
        }

        if((strLen - pos + tempCnt + charVal) > count)
        {
            actualCnt = strLen - pos; // if want to display % do -1;
            cntCalculated = true;
            break;
        }
        tempCnt += charVal;

    } while((strLen - pos + tempCnt) < count);

    if (cntCalculated == false)
    {
        actualCnt = count - tempCnt;
    }

    return actualCnt;
}

void TextBox::unloadVirtualKeyboard()
{
    if(m_catchKey == false)
    {
        unloadVirtualKeypad();
        entryByRemote = true;
        editMode = true;
        createPhysicalKeyboardWidget();
    }
}

void TextBox::asciiCharKeyPressed(QKeyEvent *event)
{
    if((m_catchKey) && (entryByRemote == true))
    {
        if((event->key() == Qt::Key_Space) && (param->isEmailAddrType == true))
        {
            return;
        }

        event->accept();
        if(lineEdit->hasSelectedText())
        {
            lineEdit->insert(QString(""));
            param->textStr = lineEdit->text();
            currCurPos = lineEdit->cursorPosition();
            currentCharLenght = param->textStr.length();
            emit sigTextValueAppended(param->textStr, m_indexInPage);
        }
        slotTextBoxKeyPressed(KEY_ALPHANUM,event->text());
    }
    else
    {
        QWidget::keyPressEvent(event);
    }
}

void TextBox::navigationKeyPressed(QKeyEvent *event)
{
    if((m_catchKey) && (entryByRemote == true))
    {
        switch(event->key())
        {
            case Qt::Key_Left:
            {
                event->accept();
                if((event->modifiers() & Qt::ShiftModifier) == Qt::ShiftModifier)
                {
                    lineEdit->cursorBackward(true);
                }
                else
                {
                    if(lineEdit->hasSelectedText())
                    {
                        QString str= lineEdit->text();
                        QString selectedStr = lineEdit->selectedText();
                        int selStartPos = str.indexOf(selectedStr);
                        lineEdit->setCursorPosition(selStartPos);
                    }
                    else
                    {
                        lineEdit->cursorBackward(false);
                    }
                }
                currCurPos = lineEdit->cursorPosition();
            }
            break;

            case Qt::Key_Right:
            {
                event->accept();
                if((event->modifiers() & Qt::ShiftModifier) == Qt::ShiftModifier)
                {
                    lineEdit->cursorForward(true);
                }
                else
                {
                    if(lineEdit->hasSelectedText())
                    {
                        QString str= lineEdit->text();
                        QString selectedStr = lineEdit->selectedText();
                        int selStartPos = str.indexOf(selectedStr);
                        selStartPos += selectedStr.length();
                        lineEdit->setCursorPosition(selStartPos);
                    }
                    else
                    {
                        lineEdit->cursorForward(false);
                    }
                }
                currCurPos = lineEdit->cursorPosition();
            }
            break;

            case Qt::Key_Home:
            {
                event->accept();
                if((event->modifiers() & Qt::ShiftModifier) == Qt::ShiftModifier)
                {
                    lineEdit->home(true);
                }
                else
                {
                    lineEdit->home(false);
                }
                currCurPos = lineEdit->cursorPosition();
            }
            break;

            case Qt::Key_End:
            {
                event->accept();
                if((event->modifiers() & Qt::ShiftModifier) == Qt::ShiftModifier)
                {
                    lineEdit->end(true);
                }
                else
                {
                    lineEdit->end(false);
                }
                currCurPos = lineEdit->cursorPosition();
            }
            break;

            default:
            {
                event->accept();
            }
            break;
        }
    }
    else
    {
        QWidget::keyPressEvent(event);
    }
}

void TextBox::escKeyPressed(QKeyEvent *event)
{
    if((m_catchKey) && (entryByRemote == true))
    {
        // this is like cancel button
        // Send notification to page in which this control to change focus on close button
        event->accept();
        m_isControlActivated = true;
        slotTextBoxKeyPressed(KEY_CLOSE, "");
    }
    else
    {
        QWidget::keyPressEvent(event);
    }
}

void TextBox::backspaceKeyPressed(QKeyEvent *event)
{
    if((m_catchKey) && (entryByRemote == true))
    {
        event->accept();
        lineEdit->backspace();
        param->textStr = lineEdit->text();
        currCurPos = lineEdit->cursorPosition();
        currentCharLenght = param->textStr.length();
        emit sigTextValueAppended(param->textStr, m_indexInPage);
    }
}

void TextBox::enterKeyPressed(QKeyEvent *event)
{
    if(m_catchKey)
    {
        if(entryByRemote == true)
        {
            event->accept();
            m_isControlActivated = true;
            slotTextBoxKeyPressed(KEY_DONE, "");
        }
        else
        {
            event->accept();
            editMode = true;

            m_isControlActivated = false;
            if(imgType != IMAGE_TYPE_MOUSE_HOVER)
            {
                changeImage(IMAGE_TYPE_MOUSE_HOVER);
            }
            lineEdit->setCursorPosition(0);
            lineEdit->end(true);
            mouseClickOnBox(true);
            createPhysicalKeyboardWidget();
        }
    }
}

void TextBox::deleteKeyPressed(QKeyEvent *event)
{
    if((m_catchKey) && (entryByRemote == true))
    {
        event->accept();
        lineEdit->del();
        param->textStr = lineEdit->text();
        currCurPos = lineEdit->cursorPosition();
        currentCharLenght = param->textStr.length();
        emit sigTextValueAppended(param->textStr, m_indexInPage);
    }
}

void TextBox::ctrl_A_KeyPressed(QKeyEvent *event)
{
    if((m_catchKey) && (entryByRemote == true))
    {
        event->accept();
        lineEdit->selectAll();
    }
}

void TextBox::ctrl_C_KeyPressed(QKeyEvent *event)
{
    if((m_catchKey) && (entryByRemote == true))
    {
        event->accept();
        m_isSpecialFunDone = false;
        lineEdit->copy();
    }
}

void TextBox::ctrl_V_KeyPressed(QKeyEvent *event)
{
    if((m_catchKey) && (entryByRemote == true) && (!m_isSpecialFunDone))
    {
        event->accept();
        quint16 strLen = countStrLenWithSpChr(param->textStr);
        if(strLen < param->maxChar)
        {
            lineEdit->paste();
            param->textStr = lineEdit->text();
            strLen = countStrLenWithSpChr(param->textStr);
            if(strLen > param->maxChar)
            {
                quint16 diff = countActualCharLenToTruncate(param->textStr, (strLen - param->maxChar));
                lineEdit->setSelection(lineEdit->cursorPosition(), -diff);
                lineEdit->del();
                param->textStr = lineEdit->text();
                m_isSpecialFunDone = true;
            }
            currCurPos = lineEdit->cursorPosition();
            currentCharLenght = param->textStr.length();
            emit sigTextValueAppended(param->textStr, m_indexInPage);
        }
    }
}

void TextBox::ctrl_X_KeyPressed(QKeyEvent *event)
{
    if((m_catchKey) && (entryByRemote == true))
    {
        event->accept();
        m_isSpecialFunDone = false;
        lineEdit->cut();
        param->textStr = lineEdit->text();
        currCurPos = lineEdit->cursorPosition();
        currentCharLenght = param->textStr.length();
        emit sigTextValueAppended(param->textStr, m_indexInPage);
    }
}

void TextBox::ctrl_Y_KeyPressed(QKeyEvent *event)
{
    if((m_catchKey) && (entryByRemote == true))
    {
        event->accept();
        quint16 strLen;

        lineEdit->redo();
        param->textStr = lineEdit->text();
        strLen = countStrLenWithSpChr(param->textStr);
        if(strLen > param->maxChar)
        {
            lineEdit->redo();
            param->textStr = lineEdit->text();
        }
        currCurPos = lineEdit->cursorPosition();
        currentCharLenght = param->textStr.length();
        emit sigTextValueAppended(param->textStr, m_indexInPage);
    }
}

void TextBox::ctrl_Z_KeyPressed(QKeyEvent *event)
{
    if((m_catchKey) && (entryByRemote == true))
    {
        event->accept();
        quint16 strLen;

        lineEdit->undo();
        param->textStr = lineEdit->text();
        strLen = countStrLenWithSpChr(param->textStr);
        if(strLen > param->maxChar)
        {
            lineEdit->undo();
            param->textStr = lineEdit->text();
        }

        if(m_isSpecialFunDone)
        {
            lineEdit->undo();
            param->textStr = lineEdit->text();
            strLen = countStrLenWithSpChr(param->textStr);
            if(strLen > param->maxChar)
            {
                lineEdit->undo();
                param->textStr = lineEdit->text();
            }
        }

        m_isSpecialFunDone = false;
        currCurPos = lineEdit->cursorPosition();
        currentCharLenght = param->textStr.length();
        emit sigTextValueAppended(param->textStr, m_indexInPage);
    }
}

void TextBox::unloadVirtualKeypad()
{
    if(IS_VALID_OBJ(virtualKeypad))
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

void TextBox::createPhysicalKeyboardWidget(void)
{
    if (false == IS_VALID_OBJ(m_physicalKeyboardInVisibleWidget))
    {
        /* PARASOFT: Memory Deallocated in unloadVirtualKeypad function */
        m_physicalKeyboardInVisibleWidget = new InvisibleWidgetCntrl(this->window());
        connect(m_physicalKeyboardInVisibleWidget,
                SIGNAL(sigMouseClick()),
                this,
                SLOT(slotInvisibleCtrlMouseClick()));
    }
}

void TextBox::deletePhysicalKeyboardWidget(void)
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

void TextBox::setCurrentCharLenght(quint8 maxCharCount)
{
    currentCharLenght = maxCharCount;
}

bool TextBox::doneKeyValidation()
{
    bool retVal=true;
    bool finalState = true;
    QString tempStr;

    do
    {
        param->textStr = param->textStr.trimmed();

        if(!(param->startCharVal.isEmpty()) && (param->textStr != ""))
        {
            tempStr = param->textStr.at(0);
            if(!(tempStr.contains(param->startCharVal)))
            {
                finalState = false;
                param->textStr = oldTextValue;
                emit sigLoadInfopage(m_indexInPage, INFO_MSG_STRAT_CHAR);
                retVal = false;
                break;
            }
        }

        if(!(param->middelCharVal.isEmpty()))
        {
            for(quint16 index = 1; index < param->textStr.length(); index++)
            {
                tempStr = param->textStr.at(index);
                if(!(tempStr.contains(param->middelCharVal)))
                {
                    finalState = false;
                    param->textStr = oldTextValue;
                    // deselect call. page info loader with start char error msg
                    emit sigLoadInfopage(m_indexInPage, INFO_MSG_ERROR);
                    retVal = false;
                    break;
                }
            }
        }

        //(!param->textStr.isEmpty()) added to stop going in when text is blank.
        if((!(param->endCharVal.isEmpty())) && (!param->textStr.isEmpty()))
        {
            tempStr = param->textStr.at(param->textStr.length() -1);
            if(!(tempStr.contains(param->endCharVal)))
            {
                finalState = false;
                param->textStr = oldTextValue;
                // page info loader with start char error msg
                emit sigLoadInfopage(m_indexInPage, INFO_MSG_END_CHAR);
                retVal = false;
            }
        }
        else if((finalState == true) && (param->textStr == ""))
        {
            if(param->isTotalBlankStrAllow == false)
            {
                finalState = false;
                param->textStr = oldTextValue;
                emit sigLoadInfopage(m_indexInPage, INFO_MSG_ERROR);
                retVal = false;
            }
        }
        else if((finalState == true) && (param->textStr.length() < param->minChar))
        {
            finalState = false;
            param->textStr = oldTextValue;
            // deselect call
            // page info loader with start char error msg
            emit sigLoadInfopage(m_indexInPage, INFO_MSG_ERROR);
            retVal = false;
        }
        else if((finalState == true) && (param->isNumEntry == true))
        {
            if(param->textStr.toUInt() < param->minNumValue)
            {
                if(param->textStr.toInt() != param->extraNumValue)
                {
                    finalState = false;
                    param->textStr = oldTextValue;
                    emit sigLoadInfopage(m_indexInPage, INFO_MSG_ERROR);
                    retVal = false;
                }
            }
        }

        if(param->isEmailAddrType == true)
        {
            QStringList emailList;
            QStringList emailList2;
            emailList.clear();
            emailList2.clear();

            if(param->textStr.contains(";"))
            {
                emailList = param->textStr.split(";",QString::SkipEmptyParts);
            }

            if(param->textStr.contains(","))
            {
                emailList = param->textStr.split(",",QString::SkipEmptyParts);
            }

            for(quint8 index = 0; index < emailList.count(); index++)
            {
                if(emailList.at(index).contains(";"))
                {
                    emailList2.append(emailList.at(index).split(";",QString::SkipEmptyParts));
                }

                if(emailList.at(index).contains(","))
                {
                    emailList2.append(emailList.at(index).split(",",QString::SkipEmptyParts));
                }
            }

            if(emailList.isEmpty())
            {
                emailList.append(param->textStr);
            }
            else if(!emailList2.isEmpty())
            {
                emailList.clear();
                emailList.append(emailList2);
            }

            for(quint8 index = 0; index < emailList.count(); index++)
            {
                QString tempStr = emailList.at(index);
                QString startChar = tempStr.at(0);

                if((tempStr != "") && (!(startChar.contains(param->startCharVal))))
                {
                    param->textStr = "";
                    emit sigLoadInfopage(m_indexInPage, INFO_MSG_STRAT_CHAR);
                    break;
                }
                else if((tempStr!= "") && (!(tempStr.contains("@") && (tempStr.contains(".")))))
                {
                    param->textStr = "";
                    emit sigLoadInfopage(m_indexInPage, INFO_MSG_ERROR);
                    retVal = false;
                    break;
                }
                else if((tempStr != "") && ((tempStr.count("@") > 1)))
                {
                    param->textStr = "";
                    emit sigLoadInfopage(m_indexInPage, INFO_MSG_ERROR);
                    retVal = false;
                    break;
                }
            }

            emailList.clear();
            emailList2.clear();
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
    lineEdit->setText(param->textStr);
    lineEdit->setCursorPosition (currCurPos);
    return retVal;
}

void TextBox::slotkeyRepTimerTimeout()
{
    prevRemoteKey = Qt::Key_unknown;
    repeatKeyCnt = 0;
}

void TextBox::slotTextBoxKeyPressed(KEY_TYPE_e keyType, QString str)
{
    QString newStr = param->textStr;
    bool isAlphaNumKeyAccept = false;
    quint32 intValue;

    if((param->isEmailAddrType == true) && (keyType == KEY_SPACE))
    {
        return;
    }

    lineEdit->setActiveFocus();

    switch(keyType)
    {
        case KEY_SPACE:
        {
            str = " ";
        }
        // FALL THROUGH
        case KEY_ALPHANUM:
        {
            // check for key to validator
            if((param->validation.isEmpty()) || (str.contains(param->validation)))
            {
                newStr = newStr.insert(currCurPos, str);
                if(m_isCharCountExternallyControl)
                {
                    if(currentCharLenght < param->maxChar)
                    {
                        if(param->isNumEntry)
                        {
                            intValue = newStr.toUInt();
                            if(intValue <= param->maxNumValue)
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
                    if(newStr.length() <= param->maxChar)
                    {
                        if(param->isNumEntry)
                        {
                            intValue = newStr.toUInt();
                            if(intValue <= param->maxNumValue)
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

                if(countStrLenWithSpChr(newStr) > param->maxChar)
                {
                    isAlphaNumKeyAccept = false;
                }
            }

            if(isAlphaNumKeyAccept == true)
            {
                lineEdit->insert(str);
                param->textStr = newStr;
                currCurPos++;
                currentCharLenght++;
                emit sigTextValueAppended(param->textStr, m_indexInPage);
            }
        }
        break;

        case KEY_ALPHANUM_SAME_INDEX:
        {
            // check for key to validator
            if((param->validation.isEmpty()) || (str.contains(param->validation)))
            {
                newStr = newStr.remove(currCurPos - 1, 1);
                newStr = newStr.insert(currCurPos - 1, str);

                if(newStr.length() <= param->maxChar)
                {
                    isAlphaNumKeyAccept = true;
                }
            }

            if(isAlphaNumKeyAccept == true)
            {
                param->textStr = newStr;
                lineEdit->setText(param->textStr);
                lineEdit->setCursorPosition (currCurPos);
                emit sigTextValueAppended(param->textStr, m_indexInPage);
            }
        }
        break;

        case KEY_CLEAR:
        {
            currCurPos = 0;
            currentCharLenght = 0;
            param->textStr = "";
            lineEdit->setText(param->textStr);
            emit sigTextValueAppended(param->textStr, m_indexInPage);
        }
        break;

        case KEY_BACKSPACE:
        {
            currCurPos = lineEdit->cursorPosition();
            currentCharLenght = param->textStr.length();
            if(currCurPos > 0)
            {
                currCurPos--;
                currentCharLenght--;
                param->textStr = param->textStr.remove(currCurPos, 1);
                lineEdit->setText(param->textStr);
                lineEdit->setCursorPosition (currCurPos);
                emit sigTextValueAppended(param->textStr, m_indexInPage);
            }
            else
            {
                currCurPos = 0;
            }
        }
        break;

        case KEY_LEFT_ARROW:
        {
            currCurPos = (currCurPos > 0) ? (currCurPos - 1) : 0;
            lineEdit->setCursorPosition(currCurPos);
        }
        break;

        case KEY_RIGHT_ARROW:
        {
            if(currCurPos <(param->textStr.length() - 1))
            {
                currCurPos++;
            }
            else
            {
                currCurPos = param->textStr.length();
            }
            lineEdit->setCursorPosition (currCurPos);
        }
        break;

        case KEY_DONE:
        {
            lineEdit->setText(param->textStr);
            lineEdit->setCursorPosition (currCurPos);

            editMode = false;

            if(entryByRemote == false)
            {
                unloadVirtualKeypad();
            }
            entryByRemote = false;
            forceActiveFocus();
            doneKeyValidation();
            deletePhysicalKeyboardWidget();
        }
        break;

        case KEY_CLOSE:
        {
            param->textStr = oldTextValue;
            currCurPos = 0;
            lineEdit->setText(param->textStr);
            lineEdit->setCursorPosition (currCurPos);
            editMode = false;
            if(entryByRemote == false)
            {
                unloadVirtualKeypad();
            }
            entryByRemote = false;            
            forceActiveFocus();
        }
        break;

        default:
        {
            /* Nothing to do */
        }
        break;
    }
    update();
}

void TextBox::slotInvisibleCtrlMouseClick()
{
    if(IS_VALID_OBJ(virtualKeypad))
    {
        virtualKeypad->slotKeyDeteceted(KEY_DONE, 0);
    }

    if(IS_VALID_OBJ(m_physicalKeyboardInVisibleWidget))
    {
        slotTextBoxKeyPressed(KEY_DONE, "");
    }
}
