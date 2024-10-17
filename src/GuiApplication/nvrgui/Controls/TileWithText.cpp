#include "TileWithText.h"
#include <QEvent>

TileWithText::TileWithText(int xParam,
                           int yParam,
                           int width,
                           int height,
                           int textLableOffset,
                           QString textString,
                           BGTILE_TYPE_e bgTileType,
                           QWidget* parent,
                           bool isTileAreaEnable,
                           int tileIdentity,
                           QString textFontColor, int indexInPage)
    : BgTile(xParam, yParam, width, height, bgTileType, parent,indexInPage),
      m_textString(textString)
{
    m_textLabelOffset = textLableOffset;
    m_isTileAreaEnable = isTileAreaEnable;
    m_tileIndentity = tileIdentity;
    m_indexInPage = indexInPage;
//    m_bgTileIndex = indexInPage;

    m_textLable = new TextLabel(m_textLabelOffset,
                                (this->height() / 2),
                                NORMAL_FONT_SIZE,
                                m_textString,
                                this,
                                textFontColor,
                                NORMAL_FONT_FAMILY,
                                ALIGN_START_X_CENTRE_Y,
                                0,
                                false,
                                0,
                                m_indexInPage);
//    connect(m_textLable,
//            SIGNAL(sigTextClick(int)),
//            this,
//            SLOT(slotMouseClick(int)));

    connect(this,
            SIGNAL(sigBgTileClick(int)),
            this,
            SLOT(slotMouseClick(int)));

    if(m_isTileAreaEnable)
    {
        this->installEventFilter(this);
    }
}

TileWithText::~TileWithText()
{
    if(NULL != m_textLable)
    {
        disconnect(this,
             SIGNAL(sigBgTileClick(int)),
             this,
             SLOT(slotMouseClick(int)));
        delete m_textLable;
        m_textLable = NULL;
    }
}

void TileWithText::changeColor(QString color)
{
    m_textLable->changeColor(color);
}

void TileWithText::changeText(QString text)
{
    m_textLable->changeText(text);
}

QString TileWithText::getText()
{
    return m_textLable->getText();
}

bool TileWithText::eventFilter(QObject *object, QEvent *event)
{
    if(m_isTileAreaEnable)
    {
        if(event->type() == QEvent::Leave)
        {
            emit sigMouseHoverInOutForTile(m_tileIndentity, false);
        }
        else if(event->type() == QEvent::Enter)
        {
            emit sigMouseHoverInOutForTile(m_tileIndentity, true);
        }
    }

    return QObject::eventFilter(object,event);
}

void TileWithText::slotMouseClick(int indexInPage)
{
    emit sigTileWithTextClick(indexInPage);
}
