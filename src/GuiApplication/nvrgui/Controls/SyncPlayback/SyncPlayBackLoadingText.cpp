#include "SyncPlayBackLoadingText.h"

#define LOADING_TEXT        "Loading..."

SyncPlayBackLoadingText::SyncPlayBackLoadingText(quint16 loadingTextAreaWidth,
                                                 quint16 loadingTextAreaHeight,
                                                 QWidget* parent) : QWidget(parent),
    m_loadingTextAreaWidth(loadingTextAreaWidth), m_loadingTextAreaHeight(loadingTextAreaHeight)
{
    this->setGeometry(0, 0, parent->width(), parent->height());

    m_rightBackground = new Rectangle(loadingTextAreaWidth,
                                      0,
                                      (parent->width() - loadingTextAreaWidth),
                                      loadingTextAreaHeight,
                                      PROCESS_BKG_COLOR,
                                      this,
                                      0, 0,
                                      PROCESS_BKG_COLOR,
                                      0.55);
    m_bottomBackground = new Rectangle(0,
                                       loadingTextAreaHeight,
                                       parent->width(),
                                       (parent->height() - loadingTextAreaHeight),
                                       PROCESS_BKG_COLOR,
                                       this,
                                       0, 0,
                                       PROCESS_BKG_COLOR,
                                       0.55);
    quint16 textWidth = 0;
    quint16 textHeight = 0;
    TextLabel::getWidthHeight(TextLabel::getFont(NORMAL_FONT_FAMILY, SCALE_FONT(30)),
                              LOADING_TEXT,
                              textWidth,
                              textHeight);
    textWidth += SCALE_WIDTH(10);
    textHeight += SCALE_HEIGHT(15);
    m_textBackgroungRectangle = new Rectangle(((m_loadingTextAreaWidth - textWidth)/ 2),
                                              ((m_loadingTextAreaHeight - textHeight) / 2),
                                              textWidth,
                                              textHeight,
                                              CLICKED_BKG_COLOR,
                                              this);
    m_loadingText = new TextLabel((m_textBackgroungRectangle->x() + (m_textBackgroungRectangle->width() / 2)),
                                  (m_textBackgroungRectangle->y() + (m_textBackgroungRectangle->height() / 2)),
                                  SCALE_FONT(30),
                                  LOADING_TEXT,
                                  this,
                                  HIGHLITED_FONT_COLOR,
                                  NORMAL_FONT_FAMILY,
                                  ALIGN_CENTRE_X_CENTER_Y);
}

SyncPlayBackLoadingText::~SyncPlayBackLoadingText()
{
    delete m_loadingText;
    delete m_textBackgroungRectangle;
    delete m_bottomBackground;
    delete m_rightBackground;
}
