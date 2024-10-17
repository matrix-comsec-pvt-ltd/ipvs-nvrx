#include "TextWithList.h"
#include "EnumFile.h"
#include <QFontMetrics>
#include <QMouseEvent>
#include <QTimer>
#include <QPaintEvent>
#include <QPainter>

#define DOWN_ARROW_IMG_PATH     IMAGE_PATH"DownArrow/"
#define TEXTBOX_FOLDER_PATH     IMAGE_PATH "Textbox/"
#define WIDTH_PADDING           SCALE_WIDTH(10)
#define BUTTON_HEIGHT           SCALE_HEIGHT(30)

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

TextWithList::TextWithList(quint32 startX, quint32 startY, quint32 width, quint32 height, quint16 controlIndex,
                           QMap<quint8, QString> listStr, QWidget *parent, TextboxParam *textBoxParam, BGTILE_TYPE_e bgType,
                           bool isNavigationEnable, quint8 maxElemetOnList, TEXTBOX_SIZE_e butnSize,
                           QString placeHolderStr, bool isDropUpList, LIST_FILTER_TYPE_e filterType, quint32 leftMarginFromCenter)
    : BgTile(startX, startY, width, height, bgType, parent), NavigationControl(controlIndex, isNavigationEnable)
{
    INIT_OBJ(m_labelText);
    INIT_OBJ(m_palette);
    INIT_OBJ(m_lineEdit);
    INIT_OBJ(m_downArrowImg);
    INIT_OBJ(m_dropDownInVisibleWidget);
    INIT_OBJ(m_dropDownList);
    INIT_OBJ(m_virtualKeypad);
    m_editMode = false;
    m_entryByRemote = false;
    m_maxElemetOnList = maxElemetOnList;
    m_valueList = listStr;
    m_param = textBoxParam;
    m_textBoxSize = butnSize;
    m_maxListNo = 0;
    m_currListNo = 0;
    m_valueStringList.clear ();
    m_mouseClickOnBox = false;    
    m_isDropUpList = isDropUpList;
    m_currCurPos = 0;
    m_actualSelectedIndex = m_valueList.size();
    m_filterType = filterType;
    m_leftMarginFromCenter = leftMarginFromCenter;

    for(quint8 index = 0; index < m_valueList.size(); index++)
    {
        m_valueStringList.append (m_valueList.value(index));
    }

    if (placeHolderStr == "")
    {
        m_PlaceHolderText = "";
    }
    else
    {
        m_PlaceHolderText = QApplication::translate(QT_TRANSLATE_STR, placeHolderStr.toUtf8().constData());
        if (m_PlaceHolderText == "")
        {
            m_PlaceHolderText = placeHolderStr;
        }
    }

    m_imgType = (m_isEnabled) ? IMAGE_TYPE_NORMAL : IMAGE_TYPE_DISABLE;
    m_imagePath = TEXTBOX_FOLDER_PATH + textboxBtnFolder[m_textBoxSize] + imgTypePath[m_imgType];
    m_image = QPixmap(m_imagePath);
    SCALE_IMAGE(m_image);
    createDefaultElement();

    m_downArrowImg = new Image((m_imgRect.x() + m_imgRect.width() - (SCALE_WIDTH(32)/2)),
                               (m_imgRect.y() + (m_imgRect.height()/2)),
                               DOWN_ARROW_IMG_PATH,
                               this,
                               CENTER_X_CENTER_Y,
                               0,
                               true);
    m_lineEdit->setEnabled(m_isEnabled);
    this->setMouseTracking(true);
    this->setEnabled(m_isEnabled);
    this->show();
}

TextWithList::~TextWithList()
{
    DELETE_OBJ(m_downArrowImg);
    DELETE_OBJ(m_labelText);
    DELETE_OBJ(m_palette);
    DELETE_OBJ(m_lineEdit);
    unloadList();
    unloadVirtualKeypad();

    if (m_dropDownInVisibleWidget != NULL)
    {
        disconnect (m_dropDownInVisibleWidget,
                    SIGNAL(sigMouseClick()),
                    this,
                    SLOT(slotUnloadDropList()));
        DELETE_OBJ(m_dropDownInVisibleWidget);
    }
}

void TextWithList::createDefaultElement()
{
    quint16 labelWidth = 0, suffixWidth = 0, labelHeight = 0, translatedlabelWidth = 0;
    quint16 width = 0;
    qint8 verticalOffset = 0;
    QFont labelFont = TextLabel::getFont(NORMAL_FONT_FAMILY, NORMAL_FONT_SIZE);

    if(m_param->labelStr != "")
    {
        labelFont = TextLabel::getFont(NORMAL_FONT_FAMILY, NORMAL_FONT_SIZE);
        translatedlabelWidth = QFontMetrics(labelFont).width (QApplication::translate(QT_TRANSLATE_STR, m_param->labelStr.toUtf8().constData()));
        labelWidth = QFontMetrics(labelFont).width(m_param->labelStr);
        labelHeight = QFontMetrics(labelFont).height();
        width += WIDTH_PADDING;
    }

    width += m_image.width() + labelWidth + suffixWidth;

    switch(m_bgTileType)
    {
    case NO_LAYER:
        m_width = width;
        this->setGeometry(m_startX, m_startY, m_width, m_height);
        m_param->isCentre = false;
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

    if(m_param->isCentre == true)
    {
        if(m_param->labelStr != "")
        {
            labelWidth = (translatedlabelWidth > ((getWidth()/2) - SCALE_WIDTH(20)))? ((getWidth()/2) - SCALE_WIDTH(20)) : translatedlabelWidth;

            m_labelText = new TextLabel(((getWidth()/2) - WIDTH_PADDING - labelWidth) - m_leftMarginFromCenter,
                                      (this->height () - labelHeight)/2 + verticalOffset,
                                      NORMAL_FONT_SIZE, m_param->labelStr,
                                      this, NORMAL_FONT_COLOR, NORMAL_FONT_FAMILY, ALIGN_START_X_START_Y,
                                      0, 0, labelWidth, 0, 0, Qt::AlignRight);
        }

        m_imgRect.setRect(this->width()/2 - m_leftMarginFromCenter,
                        ((this->height() - m_image.height())/2) + verticalOffset,
                        m_image.width(),
                        m_image.height());
    }
    else
    {
        if(m_param->labelStr != "")
        {
            translatedlabelWidth = (translatedlabelWidth > ((m_param->leftMargin + labelWidth))) ? ((m_param->leftMargin + labelWidth) - SCALE_WIDTH(16)) : (translatedlabelWidth);

            m_labelText = new TextLabel(abs((abs(translatedlabelWidth - (m_param->leftMargin + labelWidth))) - SCALE_WIDTH(5)),
                                      (this->height () - labelHeight)/2 + verticalOffset,
                                      NORMAL_FONT_SIZE,m_param->labelStr,
                                      this,
                                      NORMAL_FONT_COLOR, NORMAL_FONT_FAMILY, ALIGN_START_X_START_Y,
                                      0, 0, translatedlabelWidth, 0, 0, Qt::AlignRight);
            labelWidth += WIDTH_PADDING;
        }

        m_imgRect.setRect (m_param->leftMargin + labelWidth,
                         (this->height() - m_image.height())/2 + verticalOffset,
                         m_image.width(),
                         m_image.height());
    }

    m_lineEdit = new LineEdit(m_imgRect.x() + WIDTH_PADDING,
                            m_imgRect.y(),
                            (m_imgRect.width() - SCALE_WIDTH(20)),
                            m_imgRect.height(),
                            this);
    connect (m_lineEdit,
             SIGNAL(sigFocusChange(quint8,bool,bool)),
             this,
             SLOT(slotLineEditFocusChange(quint8,bool,bool)));
    m_lineEdit->setFrame(false);
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
    m_lineEdit->setPalette(*m_palette);
    m_lineEdit->setFont(labelFont);
    m_lineEdit->setPlaceholderText(m_PlaceHolderText);
    m_lineEdit->setMaxLength(m_param->maxChar);
}

void TextWithList::setIsEnabled(bool isEnable)
{
    if(isEnable != m_isEnabled)
    {
        m_isEnabled = isEnable;
        m_lineEdit->setEnabled(m_isEnabled);
        this->setEnabled(m_isEnabled);

        if(isEnable == true)
        {
            m_palette->setColor(QPalette::Text, QColor(NORMAL_FONT_COLOR).lighter (200));
            changeImage(IMAGE_TYPE_NORMAL);
            m_downArrowImg->selectControl();
        }
        else
        {
            m_palette->setColor(QPalette::Text, QColor(QString(DISABLE_FONT_COLOR)));
            changeImage(IMAGE_TYPE_DISABLE);
            m_downArrowImg->deSelectControl();
        }

        m_lineEdit->setPalette(*m_palette);
        update();
    }
}

void TextWithList::changeImage(IMAGE_TYPE_e imageType)
{
    m_imgType = imageType;
    m_imagePath = TEXTBOX_FOLDER_PATH + textboxBtnFolder[m_textBoxSize] + imgTypePath[m_imgType];
    m_image = QPixmap(m_imagePath);
    SCALE_IMAGE(m_image);
    update ();
}

void TextWithList::setCurrValue(QString val, bool isClearText, bool forceDispText)
{
    m_currListNo = m_valueList.key(val);
    m_actualSelectedIndex = isClearText ? m_maxListNo : m_currListNo;
    m_param->textStr = val;

    if (isClearText == true)
    {
        m_lineEdit->setText("");
    }
    else if ((m_PlaceHolderText == "") || (forceDispText == true))
    {
        m_currCurPos = 0;
        m_lineEdit->setText(m_param->textStr);
        m_lineEdit->setCursorPosition(m_currCurPos);
    }

    m_lineEdit->setPlaceholderText(m_PlaceHolderText);
    update();
}

QString TextWithList::getCurrValue(bool actualIndex) const
{
	return m_valueList.value(actualIndex ? m_actualSelectedIndex : m_currListNo);
}

void TextWithList::setIndexofCurrElement(quint8 index)
{
    m_currListNo = index;
    m_actualSelectedIndex = index;
    update();
}

quint8 TextWithList::getIndexofCurrElement() const
{
    return m_currListNo;
}

void TextWithList::changeTextAtIndex(quint8 index, QString newVal)
{
    if(index < m_maxListNo)
    {
        m_valueList.insert(index, newVal);
    }
}

void TextWithList::appendInList(QString str)
{
    m_valueList.insert(m_maxListNo,str);
    m_valueStringList.append(str);
    m_maxListNo = m_valueList.size();
}

void TextWithList::setNewList(QMap<quint8, QString> list, quint8 newSelectedIndex,  bool clrDispText, bool forceDispText)
{   
    if (m_valueList != list)
    {
        m_valueList = list;

        if (m_valueList.isEmpty())
        {
            m_valueList.insert(0,"");
        }

        m_maxListNo = m_valueList.size();

        m_valueStringList.clear();

        for(quint8 index = 0; index < m_valueList.size(); index++)
        {
            m_valueStringList.append(m_valueList.value(index));
        }
    }

    if (newSelectedIndex < m_maxListNo)
    {
        m_currListNo = newSelectedIndex;
    }
    else
    {
        m_currListNo = 0;
    }

    if (clrDispText == true)
    {
        m_actualSelectedIndex = m_maxListNo;
    }
    else
    {
        m_actualSelectedIndex = m_currListNo;
    }

    setCurrValue(getCurrValue(clrDispText), clrDispText, forceDispText);

}

void TextWithList::mouseMoveEvent(QMouseEvent *event)
{
    if(m_downArrowImg->geometry().contains(event->pos()))
    {
        m_downArrowImg->selectControl();
    }
    else
    {
        m_downArrowImg->deSelectControl();
    }

    if((m_imgRect.contains(event->pos()))&&(m_editMode == false))
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
    else
    {
        deSelectControl();
    }
}

void TextWithList::mousePressEvent(QMouseEvent *event)
{
    if((m_imgRect.contains(event->pos())) && (event->button() == m_leftMouseButton))
    {
        if(m_editMode == false)
        {
            if(!this->hasFocus())
            {
                forceActiveFocus();
                emit sigUpdateCurrentElement(m_indexInPage);
            }
        }

        m_entryByRemote = false;
        m_isControlActivated = true;
        m_lineEdit->setPlaceholderText("");

        if(!(m_downArrowImg->geometry().contains(event->pos())))
        {
            m_mouseClickOnBox = true;
            m_lineEdit->setPlaceholderText(m_PlaceHolderText);
        }

        m_param->textStr = "";
        m_lineEdit->setText(m_param->textStr);
		
        if ((m_valueList.isEmpty()) || ((m_valueList.size() == 1) && (m_valueList.value(0) == "")))
        {
            m_lineEdit->setPlaceholderText(m_PlaceHolderText);
            emit sigValueListEmpty(m_indexInPage);
        }
		else
		{
			slotTextValueAppended(m_param->textStr, m_indexInPage);
		}
        m_mouseClicked = true;
    }
}

void TextWithList::hideEvent(QHideEvent *event)
{
    QWidget::hideEvent(event);
    if(m_virtualKeypad != NULL)
    {
        m_dropDownInVisibleWidget->setVisible(false);
        m_virtualKeypad->setVisible(false);
    }
}

void TextWithList::showEvent(QShowEvent * event)
{
    QWidget::showEvent(event);
    if(m_virtualKeypad != NULL)
    {
        m_dropDownInVisibleWidget->setVisible(true);
        m_virtualKeypad->setVisible(true);
    }
}


void TextWithList::selectControl()
{
    if(m_imgType != IMAGE_TYPE_MOUSE_HOVER)
    {
        changeImage(IMAGE_TYPE_MOUSE_HOVER);
    }
}

void TextWithList::deSelectControl()
{
    if(m_imgType != IMAGE_TYPE_NORMAL)
    {
        changeImage(IMAGE_TYPE_NORMAL);
    }
}

void TextWithList::forceActiveFocus()
{
    if(m_isEnabled)
    {
        this->setFocus();
    }
}

void TextWithList::paintEvent (QPaintEvent *event)
{
    QWidget::paintEvent(event);
    BgTile::paintEvent(event);

    QPainter painter(this);
    painter.drawPixmap(m_imgRect, m_image);
}

void TextWithList::focusInEvent(QFocusEvent *)
{
    if((m_virtualKeypad != NULL) || (m_entryByRemote == true))
    {
        m_lineEdit->setActiveFocus();
    }
    else
    {
        selectControl();
    }
}

void TextWithList::focusOutEvent(QFocusEvent *)
{
    if(m_editMode == false)
    {
        deSelectControl();
    }
}

void TextWithList::slotLineEditFocusChange(quint8, bool isFocusIn, bool forceFocus)
{
    m_editMode = isFocusIn;
    if(m_editMode == false)
    {
        deSelectControl();
    }
    else
    {
        if(m_imgType != IMAGE_TYPE_MOUSE_HOVER)
        {
            changeImage(IMAGE_TYPE_MOUSE_HOVER);
        }
    }

    if(forceFocus)
    {
        forceActiveFocus();
    }
}

void TextWithList::loadList(QMap<quint8, QString>  maplist, quint8 listIndex)
{
    if(m_dropDownList == NULL)
    {
        if (m_dropDownInVisibleWidget == NULL)
        {
            m_dropDownInVisibleWidget = new InvisibleWidgetCntrl(parentWidget());
            m_dropDownInVisibleWidget->setGeometry(QRect(0,
                                                         0,
                                                         parentWidget()->width(),
                                                         parentWidget()->height()));
            connect (m_dropDownInVisibleWidget,
                     SIGNAL(sigMouseClick()),
                     this,
                     SLOT(slotUnloadDropList()));

            m_dropDownInVisibleWidget->show();
        }

        qint32 listStartY;
        if (m_isDropUpList)
        {
            /* Prepare list above the drop list button */
            listStartY = this->y() - (BUTTON_HEIGHT * ((m_maxListNo > m_maxElemetOnList) ? m_maxElemetOnList : m_maxListNo)) + SCALE_HEIGHT(1);
        }
        else
        {
            /* Prepare list below the drop list button */
            listStartY = this->y() + m_imgRect.y() + m_imgRect.height();
        }

        m_dropDownList = new DropDownList(this->x() + m_imgRect.x(),
                                          listStartY,
                                          (m_imgRect.width() - SCALE_WIDTH(10)),
                                          maplist,
                                          listIndex,
                                          parentWidget(),
                                          m_maxElemetOnList);
        connect(m_dropDownList,
                SIGNAL(destroyed()),
                this,
                SLOT(slotDropListDestroyed()));
        connect(m_dropDownList,
                SIGNAL(sigValueChanged(quint8,QString)),
                this,
                SLOT(slotValueChanged(quint8,QString)));

        if (m_mouseClickOnBox == true)
        {
            m_virtualKeypad = new VirtualKeypad(SCALE_WIDTH(618), SCALE_HEIGHT(800), this->window());
            connect(m_virtualKeypad,
                    SIGNAL(sigKeyDetected(KEY_TYPE_e, QString)),
                    this,
                    SLOT(slotTextBoxKeyPressed(KEY_TYPE_e, QString)));

            m_mouseClickOnBox = false;
            m_currCurPos = 0;
            m_lineEdit->setCursorPosition(m_currCurPos);
            m_lineEdit->setActiveFocus();
            setCatchKey(false);
        }
    }
    else
    {
        unloadList();
        loadList(maplist, listIndex);
    }
}

void TextWithList::unloadList()
{
    if(m_dropDownList != NULL)
    {
        disconnect(m_dropDownList,
                   SIGNAL(destroyed()),
                   this,
                   SLOT(slotDropListDestroyed()));
        disconnect(m_dropDownList,
                   SIGNAL(sigValueChanged(quint8,QString)),
                   this,
                   SLOT(slotValueChanged(quint8,QString)));
        delete m_dropDownList;
        m_dropDownList = NULL;
    }
}

void TextWithList::slotDropListDestroyed()
{
    disconnect(m_dropDownList,
               SIGNAL(destroyed()),
               this,
               SLOT(slotDropListDestroyed()));
    m_dropDownList = NULL;

    disconnect (m_dropDownInVisibleWidget,
                SIGNAL(sigMouseClick()),
                this,
                SLOT(slotUnloadDropList()));
    DELETE_OBJ(m_dropDownInVisibleWidget);

    unloadVirtualKeypad();
    forceActiveFocus();
}

void TextWithList::slotValueChanged(quint8, QString str)
{
    disconnect(m_dropDownList,
               SIGNAL(sigValueChanged(quint8,QString)),
               this,
               SLOT(slotValueChanged(quint8,QString)));

    m_lineEdit->setText(str);
    m_param->textStr = str;
    m_currListNo = m_actualSelectedIndex = m_valueList.key(str);
    m_currCurPos = 0;
	m_lineEdit->setCursorPosition(m_currCurPos);
    m_editMode = false;
    m_entryByRemote = false;
    emit sigValueChanged(str, m_indexInPage);
}

void TextWithList::slotUnloadDropList()
{
    unloadList();
    unloadVirtualKeypad();

    disconnect (m_dropDownInVisibleWidget,
                SIGNAL(sigMouseClick()),
                this,
                SLOT(slotUnloadDropList()));
    DELETE_OBJ(m_dropDownInVisibleWidget);

    m_currCurPos = 0;
	m_lineEdit->setCursorPosition(m_currCurPos);
    m_lineEdit->setText(getCurrValue(true));

    if (getCurrValue(true) == "")
    {
        m_lineEdit->setPlaceholderText(m_PlaceHolderText);
    }
	
    m_editMode = false;
    m_entryByRemote = false;
    forceActiveFocus();
}

void TextWithList::slotTextValueAppended(QString str, int)
{
    QStringList filterList;
    filterList.clear();
    QStringList :: Iterator it;

    if(str.isEmpty())
    {        
        filterList = m_valueStringList;
    }
    else if (m_filterType == LIST_FILTER_TYPE_ANY_CHAR)
    {
        filterList = m_valueStringList.filter(str, Qt::CaseInsensitive);
    }
    else if (m_filterType == LIST_FILTER_TYPE_INITIAL_CHAR_ONLY)
    {
        for (it = m_valueStringList.begin(); it != m_valueStringList.end(); it++)
        {
            QString string = *it;
            string = string.toLower();
            str = str.toLower();
            qint8 chrCount = 0;

            for (qint8 index = 0; (index != str.length() && index != string.length()); index++)
            {
                if (str.at(index) != string.at(index))
                {
                    break;
                }

                chrCount++;
            }

            if (chrCount == str.length())
            {
                filterList.append(*it);
            }
        }
    }

    if(!filterList.isEmpty())
    {
        QMap<quint8, QString>  mapList;
        mapList.clear();

        for(quint8 index = 0; index < filterList.length(); index++)
        {
            mapList.insert(index,(filterList.at(index)));
        }

		quint8 selectedIndex = (filterList == m_valueStringList) ? m_actualSelectedIndex : m_maxListNo;
        loadList(mapList, selectedIndex);
    }
    else
    {
        unloadList();
    }
}

void TextWithList::unloadVirtualKeypad()
{
    if(m_virtualKeypad != NULL)
    {
        disconnect (m_virtualKeypad,
                    SIGNAL(sigKeyDetected(KEY_TYPE_e,QString)),
                    this,
                    SLOT(slotTextBoxKeyPressed(KEY_TYPE_e,QString)));
        m_virtualKeypad->deleteLater();
        m_virtualKeypad = NULL;
        setCatchKey(true);
    }
}

void TextWithList::slotTextBoxKeyPressed(KEY_TYPE_e keyType, QString str)
{
    QString newStr = m_param->textStr;
    bool isAlphaNumKeyAccept = false;

    switch(keyType)
    {
    case KEY_ALPHANUM:
        // check for key to validator
        if((m_param->validation.isEmpty()) || (str.contains(m_param->validation)))
        {
            newStr = newStr.insert(m_currCurPos, str);
            if(newStr.length() <= m_param->maxChar)
            {
                isAlphaNumKeyAccept = true;
            }
        }

        if(isAlphaNumKeyAccept == true)
        {
            m_currCurPos++;
            m_param->textStr = newStr;
            m_lineEdit->setText(m_param->textStr);
            m_lineEdit->setCursorPosition(m_currCurPos);
            slotTextValueAppended(m_param->textStr, m_indexInPage);
        }
        break;

    case KEY_CLEAR:
        m_currCurPos = 0;
        m_param->textStr = "";
        m_lineEdit->setText(m_param->textStr);
        m_lineEdit->setCursorPosition(m_currCurPos);
        slotTextValueAppended(m_param->textStr, m_indexInPage);
        break;

    case KEY_BACKSPACE:
        m_currCurPos = m_lineEdit->cursorPosition();

        if(m_currCurPos > 0)
        {
            m_currCurPos--;
            m_param->textStr = m_param->textStr.remove (m_currCurPos, 1);
            m_lineEdit->setText(m_param->textStr);
            m_lineEdit->setCursorPosition(m_currCurPos);
            slotTextValueAppended(m_param->textStr, m_indexInPage);
        }
        else
        {
            m_currCurPos = 0;
            m_lineEdit->setCursorPosition(m_currCurPos);
        }
        break;

    case KEY_LEFT_ARROW:
        if(m_currCurPos > 0)
        {
            m_currCurPos--;
        }
        else
        {
            m_currCurPos = 0;
        }
        m_lineEdit->setCursorPosition(m_currCurPos);
        break;

    case KEY_RIGHT_ARROW:
        if(m_currCurPos < (m_param->textStr.length() - 1))
        {
            m_currCurPos++;
        }
        else
        {
            m_currCurPos = m_param->textStr.length();
        }
        m_lineEdit->setCursorPosition(m_currCurPos);
        break;

    case KEY_DONE:
        m_editMode = false;
        m_lineEdit->setText(getCurrValue(true));
        if(m_entryByRemote == false)
        {
            unloadVirtualKeypad();
        }
        m_entryByRemote = false;
        break;

    case KEY_CLOSE:
        m_lineEdit->setText(getCurrValue());
        m_lineEdit->setCursorPosition(m_currCurPos);
        m_editMode = false;
        m_entryByRemote = false;
        slotUnloadDropList();
        break;

    default:
        break;
    }

    keyType != KEY_CLOSE ? (keyType != KEY_DONE ? m_lineEdit->setActiveFocus() : forceActiveFocus()) : forceActiveFocus();
    update();
}

void TextWithList::unloadVirtualKeyboard()
{
    if(m_catchKey == false)
    {
        unloadVirtualKeypad();
        m_entryByRemote = true;
        m_editMode = true;
    }
}

void TextWithList::asciiCharKeyPressed(QKeyEvent *event)
{
    if((m_catchKey) && (m_entryByRemote == true))
    {
        if((event->key() == Qt::Key_Space) && (m_param->isEmailAddrType == true))
        {
            return;
        }

        event->accept();
        if(m_lineEdit->hasSelectedText())
        {
            m_lineEdit->insert(QString(""));
            m_param->textStr = m_lineEdit->text();
            m_currCurPos = m_lineEdit->cursorPosition();
        }
        slotTextBoxKeyPressed(KEY_ALPHANUM, event->text());
    }
    else
    {
        QWidget::keyPressEvent(event);
    }
}

void TextWithList::navigationKeyPressed(QKeyEvent *event)
{
    if((m_catchKey) && (m_entryByRemote == true))
    {
        switch(event->key())
        {
        case Qt::Key_Left:
            event->accept();
            if((event->modifiers() & Qt::ShiftModifier) == Qt::ShiftModifier)
            {
                m_lineEdit->cursorBackward(true);
            }
            else
            {
                if(m_lineEdit->hasSelectedText())
                {
                    QString str= m_lineEdit->text();
                    QString selectedStr = m_lineEdit->selectedText();
                    int selStartPos = str.indexOf(selectedStr);
                    m_lineEdit->setCursorPosition(selStartPos);
                }
                else
                {
                    m_lineEdit->cursorBackward(false);
                }
            }
            m_currCurPos = m_lineEdit->cursorPosition ();
            break;

        case Qt::Key_Right:
            event->accept();
            if((event->modifiers() & Qt::ShiftModifier) == Qt::ShiftModifier)
            {
                m_lineEdit->cursorForward(true);
            }
            else
            {
                if(m_lineEdit->hasSelectedText())
                {
                    QString str= m_lineEdit->text();
                    QString selectedStr = m_lineEdit->selectedText();
                    int selStartPos = str.indexOf(selectedStr);
                    selStartPos += selectedStr.length();
                    m_lineEdit->setCursorPosition(selStartPos);
                }
                else
                {
                    m_lineEdit->cursorForward(false);
                }
            }
            m_currCurPos = m_lineEdit->cursorPosition ();
            break;

        case Qt::Key_Home:
            event->accept();
            if((event->modifiers() & Qt::ShiftModifier) == Qt::ShiftModifier)
            {
                m_lineEdit->home(true);
            }
            else
            {
                m_lineEdit->home(false);
            }
            m_currCurPos = m_lineEdit->cursorPosition ();
            break;

        case Qt::Key_End:
            event->accept();
            if((event->modifiers() & Qt::ShiftModifier) == Qt::ShiftModifier)
            {
                m_lineEdit->end(true);
            }
            else
            {
                m_lineEdit->end(false);
            }
            m_currCurPos = m_lineEdit->cursorPosition ();
            break;

        case Qt::Key_Down:
            event->accept();
            if(IS_VALID_OBJ(m_dropDownList))
            {
                m_dropDownList->setFocus();
                m_dropDownList->setFocusToPage();
            }
            break;

        default:
            event->accept();
            break;
        }
    }
    else
    {
        QWidget::keyPressEvent(event);
    }
}

void TextWithList::escKeyPressed(QKeyEvent *event)
{
    if((m_catchKey) && (m_entryByRemote == true))
    {
        event->accept();
        m_isControlActivated = true;
        slotTextBoxKeyPressed(KEY_CLOSE, "");
    }
    else
    {
        QWidget::keyPressEvent(event);
    }
}

void TextWithList::backspaceKeyPressed(QKeyEvent *event)
{
    if((m_catchKey) && (m_entryByRemote == true))
    {
        event->accept();
        slotTextBoxKeyPressed(KEY_BACKSPACE, "");
    }
}

void TextWithList::enterKeyPressed(QKeyEvent *event)
{
    if(m_catchKey)
    {
        event->accept();
        m_isControlActivated = true;

        if(m_entryByRemote == true)
        {
            slotUnloadDropList();
            m_entryByRemote = false;
        }
        else
        {
            if(!this->hasFocus())
            {
                forceActiveFocus();
                emit sigUpdateCurrentElement(m_indexInPage);
            }

            m_lineEdit->setPlaceholderText("");
            m_param->textStr = "";
            m_lineEdit->setText(m_param->textStr);
            if ((m_valueList.isEmpty()) || ((m_valueList.size() == 1) && (m_valueList.value(0) == "")))
            {
                m_lineEdit->setPlaceholderText(m_PlaceHolderText);
                emit sigValueListEmpty(m_indexInPage);
                return;
            }

            m_entryByRemote = true;
            m_editMode = true;
            slotTextValueAppended(m_param->textStr, m_indexInPage);
            if(m_imgType != IMAGE_TYPE_MOUSE_HOVER)
            {
                changeImage(IMAGE_TYPE_MOUSE_HOVER);
            }

            m_lineEdit->setCursorPosition(0);
            m_lineEdit->end(true);
            m_lineEdit->setActiveFocus();
            m_currCurPos = m_lineEdit->cursorPosition();
        }
    }
}

void TextWithList::deleteKeyPressed(QKeyEvent *event)
{
    if((m_catchKey) && (m_entryByRemote == true))
    {
        event->accept();
        m_lineEdit->del();
        m_param->textStr = m_lineEdit->text();
        m_currCurPos = m_lineEdit->cursorPosition ();
        slotTextValueAppended(m_param->textStr, m_indexInPage);
    }
}
