#include "BackGround.h"

BackGround::BackGround(int startX,
                       int startY,
                       int mainRectWidth,
                       int mainRectHeight,
                       BG_TYPE_e bgtype,
                       TOOLBAR_BUTTON_TYPE_e toolbarPageIndex,
                       QWidget *parent,
                       int complementryRectWidth,
                       int complementryRectHeight,
                       QString pageHeading,
                       QString subPageHeading,
                       bool isCloseButton,
                       QString backgroundColor,
                       qreal opacityValue)
    : KeyBoard(parent), m_backgroundColor(backgroundColor), m_opacityValue(opacityValue)
{
    m_isCloseButton = isCloseButton;
    m_startX = startX;
    m_startY = startY;
    if(m_opacityValue != 1.0)
    {
        m_complmentryBackgroundColor = m_borderColor_1 = m_borderColor_2 = m_backgroundColor;
    }
    else
    {
        m_complmentryBackgroundColor = NORMAL_BKG_COLOR;
        m_borderColor_1 = BORDER_1_COLOR;
        m_borderColor_2 = BORDER_2_COLOR;
    }

    m_mainRectWidth = mainRectWidth;
    m_mainRectHeight = mainRectHeight;

    m_complementryRectWidth = complementryRectWidth;
    m_complementryRectHeight = complementryRectHeight;

    m_backGroundType = bgtype;
    m_pageHeadingText = pageHeading;
    m_subPageheadingText = subPageHeading;
    m_toolbarPageIndex = toolbarPageIndex;

    createElements();
    this->show();
}

BackGround::BackGround(int startX,
                       int startY,
                       int mainRectWidth,
                       int mainRectHeight,
                       BG_TYPE_e bgtype,
                       TOOLBAR_BUTTON_TYPE_e toolbarPageIndex,
                       QWidget *parent,
                       bool isCloseButton,
                       QString pageHeading,
                       QString backgroundColor,
                       qreal opacityValue,
                       QString borderColor)
    : KeyBoard(parent), m_backgroundColor(backgroundColor), m_opacityValue(opacityValue)
{
    m_isCloseButton = isCloseButton;
    m_startX = startX;
    m_startY = startY;

    if(m_opacityValue != 1.0)
    {
        m_complmentryBackgroundColor = m_borderColor_1 = m_borderColor_2 = m_backgroundColor;
    }
    else
    {
        m_complmentryBackgroundColor = NORMAL_BKG_COLOR;
        m_borderColor_1 = BORDER_1_COLOR;
        m_borderColor_2 = BORDER_2_COLOR;
    }

    if(borderColor != "")
    {
        m_borderColor_1 = m_borderColor_2 = borderColor;
    }

    m_mainRectWidth = mainRectWidth;
    m_mainRectHeight = mainRectHeight;

    m_complementryRectWidth = 0;
    m_complementryRectHeight = 0;

    m_backGroundType = bgtype;
    m_pageHeadingText = pageHeading;
    m_subPageheadingText = "";
    m_toolbarPageIndex = toolbarPageIndex;

    createElements();
    this->show();
}

BackGround::~BackGround()
{
    if(m_complemetryRectangle != NULL)
    {
        delete m_complemetryRectangle;
    }

    if(m_mainRectangle != NULL)
    {
        delete m_mainRectangle;
    }

    if(m_backGroundType == BACKGROUND_TYPE_2)
    {
        delete m_leftPanelTopBorder;
        delete m_leftPanelLeftBorder;
        delete m_leftPanelBottomBorder;
        delete m_leftPanelRightBorder;

        delete m_mainRightPanelRect;
        delete m_innerRightPanelRect;

        delete m_outerLeftBorder;
        delete m_outerBottomBorder;

        delete m_innerLeftBorder;
        delete m_innerBottomBorder;
    }

    if(m_pageHeading != NULL)
    {
        delete m_pageHeading;
    }

    if(m_subPageheading != NULL)
    {
        delete m_subPageheading;
    }

    if(m_mainCloseButton != NULL)
    {
        disconnect(m_mainCloseButton,
                   SIGNAL(sigButtonClick(int)),
                   this,
                   SLOT(slotCloseButtonClicked(int)));
        delete m_mainCloseButton;
    }

    if(m_subCloseButton != NULL)
    {
        disconnect(m_subCloseButton,
                   SIGNAL(sigButtonClick(int)),
                   this,
                   SLOT(slotCloseButtonClicked(int)));
        delete m_subCloseButton;
    }
}

void BackGround::createElements()
{
    m_complemetryRectangle = NULL;
    m_mainRectangle = NULL;
    m_pageHeading = NULL;
    m_subPageheading = NULL;
    m_mainCloseButton = NULL;
    m_subCloseButton = NULL;

    switch(m_backGroundType)
    {
    case BACKGROUND_TYPE_1:
        this->setGeometry (QRect(m_startX,
                                 m_startY ,
                                 (m_mainRectWidth + SCALE_WIDTH(10)),
                                 (m_mainRectHeight + m_complementryRectHeight)));
        m_mainRectangle = new Rectangle(0,
                                        m_complementryRectHeight,
                                        m_mainRectWidth,
                                        m_mainRectHeight,
                                        m_backgroundColor,
                                        this,
                                        SCALE_WIDTH(RECT_RADIUS),
                                        0,
                                        m_backgroundColor,
                                        m_opacityValue);
        m_complemetryRectangle = new Rectangle(0,
                                               0,
                                               m_complementryRectWidth,
                                               (m_complementryRectHeight + (2 * SCALE_WIDTH(RECT_RADIUS))),
                                               m_complmentryBackgroundColor,
                                               this,
                                               SCALE_WIDTH(RECT_RADIUS),
                                               0,
                                               m_backgroundColor,
                                               m_opacityValue);
        if(m_isCloseButton)
        {
            m_mainCloseButton = new CloseButtton((m_complementryRectWidth - SCALE_WIDTH(20)),
                                                 (m_complementryRectHeight / 2),
                                                 this,
                                                 CLOSE_BTN_TYPE_1,
                                                 MAIN_CLOSE_BUTTON);

        }
        drawHeading((m_complementryRectWidth / 2),
                    (m_complementryRectHeight / 2));
        break;

    case BACKGROUND_TYPE_2:
        this->setGeometry (QRect(m_startX ,
                                 m_startY ,
                                 (m_mainRectWidth + m_complementryRectWidth) ,
                                 m_mainRectHeight));
        m_mainRectangle = new Rectangle(0,
                                        0,
                                        m_mainRectWidth,
                                        m_mainRectHeight,
                                        m_backgroundColor,
                                        this,
                                        SCALE_WIDTH(RECT_RADIUS),
                                        0,
                                        m_backgroundColor,
                                        m_opacityValue);
        m_complemetryRectangle = new Rectangle((m_mainRectWidth - (2 * SCALE_WIDTH(RECT_RADIUS))),
                                               (m_mainRectHeight - m_complementryRectHeight),
                                               (m_complementryRectWidth + (2 * SCALE_WIDTH(RECT_RADIUS))),
                                               m_complementryRectHeight,
                                               m_complmentryBackgroundColor,
                                               this,
                                               SCALE_WIDTH(RECT_RADIUS),
                                               0,
                                               m_backgroundColor,
                                               m_opacityValue);

        m_leftPanelTopBorder = new Rectangle(LEFT_PANEL_LEFT_MARGIN,
                                             (m_mainRectHeight - m_complementryRectHeight + LEFT_PANEL_TOP_MARGIN),
                                             (m_mainRectWidth - (2 * LEFT_PANEL_LEFT_MARGIN)),
                                             UNITBORDERTHIKNESS,
                                             m_borderColor_1,
                                             this,
                                             0, 0,
                                             m_borderColor_1,
                                             m_opacityValue);
        m_leftPanelLeftBorder = new Rectangle(LEFT_PANEL_LEFT_MARGIN,
                                              (m_mainRectHeight - m_complementryRectHeight + LEFT_PANEL_TOP_MARGIN),
                                              UNITBORDERTHIKNESS,
                                              (m_complementryRectHeight - (LEFT_PANEL_TOP_MARGIN + LEFT_PANEL_BOTTOM_MARGIN)),
                                              m_borderColor_2,
                                              this,
                                              0, 0,
                                              m_borderColor_2,
                                              m_opacityValue);
        m_leftPanelBottomBorder = new Rectangle(LEFT_PANEL_LEFT_MARGIN,
                                                (m_mainRectHeight - LEFT_PANEL_BOTTOM_MARGIN),
                                                (m_mainRectWidth - (2 * LEFT_PANEL_LEFT_MARGIN)),
                                                UNITBORDERTHIKNESS,
                                                m_borderColor_2,
                                                this,
                                                0, 0,
                                                m_borderColor_2,
                                                m_opacityValue);
        m_leftPanelRightBorder = new Rectangle((m_mainRectWidth - LEFT_PANEL_LEFT_MARGIN),
                                               (m_mainRectHeight - m_complementryRectHeight + LEFT_PANEL_TOP_MARGIN),
                                               UNITBORDERTHIKNESS,
                                               (m_complementryRectHeight - (LEFT_PANEL_TOP_MARGIN + LEFT_PANEL_BOTTOM_MARGIN)),
                                               m_borderColor_1,
                                               this,
                                               0, 0,
                                               m_borderColor_2,
                                               m_opacityValue);

        m_mainRightPanelRect = new Rectangle((m_mainRectWidth + SCALE_WIDTH(OUTER_LEFT_MARGIN)),
                                             (m_mainRectHeight - m_complementryRectHeight + SCALE_HEIGHT(OUTER_TOP_MARGIN)),
                                             (m_complementryRectWidth - (SCALE_WIDTH(OUTER_LEFT_MARGIN) + SCALE_WIDTH(OUTER_RIGHT_MARGIN))),
                                             (m_complementryRectHeight - (SCALE_HEIGHT(OUTER_TOP_MARGIN) + SCALE_HEIGHT(OUTER_BOTTOM_MARGIN))),
                                             CLICKED_BKG_COLOR,
                                             this,
                                             0,
                                             UNITBORDERTHIKNESS,
                                             m_borderColor_2,
                                             m_opacityValue);
        m_innerRightPanelRect = new Rectangle((m_mainRectWidth + SCALE_WIDTH(INNER_LEFT_MARGIN)),
                                              (m_mainRectHeight - m_complementryRectHeight + SCALE_HEIGHT(INNER_TOP_MARGIN)),
                                              (m_complementryRectWidth - (SCALE_WIDTH(INNER_LEFT_MARGIN) + SCALE_WIDTH(INNER_RIGHT_MARGIN))),
                                              (m_complementryRectHeight - (SCALE_HEIGHT(INNER_TOP_MARGIN) + SCALE_HEIGHT(INNER_BOTTOM_MARGIN))),
                                              CLICKED_BKG_COLOR,
                                              this,
                                              0,
                                              UNITBORDERTHIKNESS,
                                              m_borderColor_2,
                                              m_opacityValue);

        m_outerLeftBorder = new Rectangle((m_mainRectWidth + SCALE_WIDTH(OUTER_LEFT_MARGIN)),
                                          (m_mainRectHeight - m_complementryRectHeight + SCALE_HEIGHT(OUTER_TOP_MARGIN)),
                                          UNITBORDERTHIKNESS,
                                          (m_complementryRectHeight - (SCALE_HEIGHT(OUTER_TOP_MARGIN) + SCALE_HEIGHT(OUTER_BOTTOM_MARGIN))),
                                          m_borderColor_1,
                                          this,
                                          0, 0,
                                          m_borderColor_1,
                                          m_opacityValue);
        m_outerBottomBorder = new Rectangle((m_mainRectWidth + SCALE_WIDTH(OUTER_LEFT_MARGIN)),
                                            (m_mainRectHeight - SCALE_HEIGHT(OUTER_BOTTOM_MARGIN)),
                                            (m_complementryRectWidth - (SCALE_WIDTH(OUTER_LEFT_MARGIN) + SCALE_WIDTH(OUTER_RIGHT_MARGIN))),
                                            UNITBORDERTHIKNESS,
                                            m_borderColor_1,
                                            this,
                                            0, 0,
                                            m_borderColor_1,
                                            m_opacityValue);

        m_innerLeftBorder = new Rectangle((m_mainRectWidth + SCALE_WIDTH(INNER_LEFT_MARGIN)),
                                          (m_mainRectHeight - m_complementryRectHeight + SCALE_HEIGHT(INNER_TOP_MARGIN)),
                                          UNITBORDERTHIKNESS,
                                          (m_complementryRectHeight - (SCALE_HEIGHT(INNER_TOP_MARGIN) + SCALE_HEIGHT(INNER_BOTTOM_MARGIN))),
                                          m_borderColor_1,
                                          this,
                                          0, 0,
                                          m_borderColor_1,
                                          m_opacityValue);
        m_innerBottomBorder = new Rectangle((m_mainRectWidth + SCALE_WIDTH(INNER_LEFT_MARGIN)),
                                            (m_mainRectHeight - SCALE_HEIGHT(INNER_BOTTOM_MARGIN)),
                                            (m_complementryRectWidth - (SCALE_WIDTH(INNER_LEFT_MARGIN) + SCALE_WIDTH(INNER_RIGHT_MARGIN))),
                                            UNITBORDERTHIKNESS,
                                            m_borderColor_1,
                                            this,
                                            0, 0,
                                            m_borderColor_1,
                                            m_opacityValue);

        if(m_isCloseButton)
        {
            m_mainCloseButton = new CloseButtton((m_mainRectWidth - SCALE_WIDTH(20)),
                                                 ((m_mainRectHeight - m_complementryRectHeight) / 2),
                                                 this,
                                                 CLOSE_BTN_TYPE_1,
                                                 MAIN_CLOSE_BUTTON);
            m_subCloseButton = new CloseButtton((m_mainRectWidth + m_complementryRectWidth - SCALE_WIDTH(30)),
                                                (m_mainRectHeight - m_complementryRectHeight + SCALE_HEIGHT(OUTER_TOP_MARGIN) + ((SCALE_HEIGHT(INNER_TOP_MARGIN) - SCALE_HEIGHT(OUTER_TOP_MARGIN)) / 2)),
                                                this,
                                                CLOSE_BTN_TYPE_1,
                                                SUB_PAGE_CLOSE_BUTTON);
        }


        drawSubPageHeading((m_complementryRectWidth / 2 + m_mainRectWidth),
                           (m_mainRectHeight - m_complementryRectHeight + SCALE_HEIGHT(OUTER_TOP_MARGIN) + ((SCALE_HEIGHT(INNER_TOP_MARGIN) - SCALE_HEIGHT(OUTER_TOP_MARGIN)) / 2)));
        drawHeading((m_mainRectWidth / 2),
                    ((m_mainRectHeight - m_complementryRectHeight) / 2));
        hideRightpanel();
        break;

    case BACKGROUND_TYPE_3:
        this->setGeometry (QRect(m_startX ,
                                 m_startY ,
                                 m_mainRectWidth,
                                 m_mainRectHeight));
        m_mainRectangle = new Rectangle(0,
                                        0,
                                        m_mainRectWidth,
                                        m_mainRectHeight,
                                        m_backgroundColor,
                                        this,
                                        0,
                                        UNITBORDERTHIKNESS,
                                        m_borderColor_2,
                                        m_opacityValue);
        if(m_isCloseButton)
        {
            m_mainCloseButton = new CloseButtton((m_mainRectWidth - SCALE_WIDTH(20)),
                                                 SCALE_HEIGHT(20),
                                                 this,
                                                 CLOSE_BTN_TYPE_1,
                                                 MAIN_CLOSE_BUTTON);
            m_subPageheadingText = m_pageHeadingText;
            drawSubPageHeading((m_mainRectWidth / 2),
                               SCALE_HEIGHT(20));
        }
        break;

    case BACKGROUND_TYPE_4:
        this->setGeometry (QRect(m_startX ,
                                 m_startY ,
                                 m_mainRectWidth,
                                 m_mainRectHeight));
        m_mainRectangle = new Rectangle(0,
                                        0,
                                        m_mainRectWidth,
                                        m_mainRectHeight,
                                        m_backgroundColor,
                                        this,
                                        SCALE_WIDTH(RECT_RADIUS),
                                        (2 * UNITBORDERTHIKNESS),
                                        m_borderColor_2,
                                        m_opacityValue);
        if(m_isCloseButton)
        {
            m_mainCloseButton = new CloseButtton((m_mainRectWidth - SCALE_WIDTH(30)),
                                                 SCALE_HEIGHT(30),
                                                 this,
                                                 CLOSE_BTN_TYPE_1,
                                                 MAIN_CLOSE_BUTTON);
        }
        m_subPageheadingText = m_pageHeadingText;
        drawSubPageHeading((m_mainRectWidth / 2),
                           SCALE_HEIGHT(30));
        break;
    }

    if(m_mainCloseButton != NULL)
    {
        connect(m_mainCloseButton,
                SIGNAL(sigButtonClick(int)),
                this,
                SLOT(slotCloseButtonClicked(int)));
    }

    if(m_subCloseButton != NULL)
    {
        connect(m_subCloseButton,
                SIGNAL(sigButtonClick(int)),
                this,
                SLOT(slotCloseButtonClicked(int)));
    }
}

void BackGround::setGeometryForRectangles()
{
    switch(m_backGroundType)
    {
    // Top + Main Panel
    case BACKGROUND_TYPE_1:
        // (width + 10) ::-> Hlthstatus last col tooltip, allowing to go
        //  outside of width Geometry
        this->setGeometry (QRect(m_startX,
                                 m_startY ,
                                 (m_mainRectWidth + SCALE_WIDTH(10)),
                                 (m_mainRectHeight + m_complementryRectHeight)));
        drawTopPanel();
        drawBottomPanel();
        break;

        // Left + Right Panel
    case BACKGROUND_TYPE_2:
        this->setGeometry (QRect(m_startX ,
                                 m_startY ,
                                 (m_mainRectWidth + m_complementryRectWidth) ,
                                 m_mainRectHeight));
        drawLeftPanel();
        drawRightPanel();
        //        hideRightpanel();
        break;

        //Normal Panel
    case BACKGROUND_TYPE_3:
        this->setGeometry (QRect(m_startX ,
                                 m_startY ,
                                 m_mainRectWidth,
                                 m_mainRectHeight));
        m_mainRectangle->resetGeometry(0, 0, m_mainRectWidth, m_mainRectHeight);
        if(m_isCloseButton)
        {
            m_mainCloseButton->resetGeometry((m_mainRectWidth - SCALE_WIDTH(20)),
                                             SCALE_HEIGHT(20));
        }
        if(m_subPageheading != NULL)
        {
            m_subPageheading->resetGeometry((m_mainRectWidth / 2),
                                            SCALE_HEIGHT(20));
        }
        break;

        //Rounded Rectangle
    case BACKGROUND_TYPE_4:
        this->setGeometry (QRect(m_startX ,
                                 m_startY ,
                                 m_mainRectWidth,
                                 m_mainRectHeight));
        m_mainRectangle->resetGeometry(0, 0, m_mainRectWidth, m_mainRectHeight);
        if(m_isCloseButton)
        {
            m_mainCloseButton->resetGeometry((m_mainRectWidth -SCALE_WIDTH(30)),
                                             SCALE_HEIGHT(30));
        }
        if(m_subPageheading != NULL)
        {
            m_subPageheading->resetGeometry((m_mainRectWidth / 2),
                                            SCALE_HEIGHT(30));
        }
        break;
    }
}

void BackGround::drawTopPanel()
{
    m_complemetryRectangle->resetGeometry(0,
                                          0,
                                          m_complementryRectWidth,
                                          (m_complementryRectHeight + (2 * SCALE_WIDTH(RECT_RADIUS))));
}

void BackGround::drawBottomPanel()
{
    m_mainRectangle->resetGeometry(0,
                                   m_complementryRectHeight,
                                   m_mainRectWidth,
                                   m_mainRectHeight);
}

void BackGround::drawHeading(int centerX, int centerY)
{
    if(m_pageHeadingText != "")
    {
        m_pageHeading = new Heading(centerX, centerY, m_pageHeadingText, this);
    }
}

void BackGround::drawSubPageHeading(int centerX, int centerY)
{
    if(m_subPageheadingText != "")
    {
        m_subPageheading = new Heading(centerX, centerY, m_subPageheadingText, this, HEADING_TYPE_2);
    }
}

void BackGround::changeSubPageHeading(QString text)
{
    m_subPageheadingText = text;
    if(m_subPageheading == NULL)
    {
        drawSubPageHeading((m_complementryRectWidth / 2 + m_mainRectWidth),
                           (m_mainRectHeight - m_complementryRectHeight + SCALE_HEIGHT(OUTER_TOP_MARGIN) + ((SCALE_HEIGHT(INNER_TOP_MARGIN) - SCALE_HEIGHT(OUTER_TOP_MARGIN)) / 2)));
    }
    else
    {
        m_subPageheading->changeHeadingText(text);
    }
}

CloseButtton* BackGround::getCloseButton()
{
    return m_mainCloseButton;
}

void BackGround::drawLeftPanel()
{
    m_leftPanelTopBorder->resetGeometry(LEFT_PANEL_LEFT_MARGIN,
                                        (m_mainRectHeight - m_complementryRectHeight + LEFT_PANEL_TOP_MARGIN),
                                        (m_mainRectWidth - (2 * LEFT_PANEL_LEFT_MARGIN)),
                                        UNITBORDERTHIKNESS);

    m_leftPanelLeftBorder->resetGeometry(LEFT_PANEL_LEFT_MARGIN,
                                         (m_mainRectHeight - m_complementryRectHeight + LEFT_PANEL_TOP_MARGIN),
                                         UNITBORDERTHIKNESS,
                                         (m_complementryRectHeight - (LEFT_PANEL_TOP_MARGIN + LEFT_PANEL_BOTTOM_MARGIN)));

    m_leftPanelBottomBorder->resetGeometry(LEFT_PANEL_LEFT_MARGIN,
                                           (m_mainRectHeight - LEFT_PANEL_BOTTOM_MARGIN),
                                           (m_mainRectWidth - (2 * LEFT_PANEL_LEFT_MARGIN)),
                                           UNITBORDERTHIKNESS);

    m_leftPanelRightBorder->resetGeometry((m_mainRectWidth - LEFT_PANEL_LEFT_MARGIN),
                                          (m_mainRectHeight - m_complementryRectHeight + LEFT_PANEL_TOP_MARGIN),
                                          UNITBORDERTHIKNESS,
                                          (m_complementryRectHeight - (LEFT_PANEL_TOP_MARGIN + LEFT_PANEL_BOTTOM_MARGIN)));

    m_mainRectangle->resetGeometry(0,
                                   0,
                                   m_mainRectWidth,
                                   m_mainRectHeight);
}

void BackGround::drawRightPanel()
{
    m_mainRightPanelRect->resetGeometry((m_mainRectWidth + SCALE_WIDTH(OUTER_LEFT_MARGIN)),
                                        (m_mainRectHeight - m_complementryRectHeight + SCALE_HEIGHT(OUTER_TOP_MARGIN)),
                                        (m_complementryRectWidth - (SCALE_WIDTH(OUTER_LEFT_MARGIN) + SCALE_WIDTH(OUTER_RIGHT_MARGIN))),
                                        (m_complementryRectHeight - (SCALE_HEIGHT(OUTER_TOP_MARGIN) + SCALE_HEIGHT(OUTER_BOTTOM_MARGIN))));

    m_innerRightPanelRect->resetGeometry((m_mainRectWidth + SCALE_WIDTH(INNER_LEFT_MARGIN)),
                                         (m_mainRectHeight - m_complementryRectHeight + SCALE_HEIGHT(INNER_TOP_MARGIN)),
                                         (m_complementryRectWidth - (SCALE_WIDTH(INNER_LEFT_MARGIN) + SCALE_WIDTH(INNER_RIGHT_MARGIN))),
                                         (m_complementryRectHeight - (SCALE_HEIGHT(INNER_TOP_MARGIN) + SCALE_HEIGHT(INNER_BOTTOM_MARGIN))));

    m_outerLeftBorder->resetGeometry((m_mainRectWidth + SCALE_WIDTH(OUTER_LEFT_MARGIN)),
                                     (m_mainRectHeight - m_complementryRectHeight + SCALE_HEIGHT(OUTER_TOP_MARGIN)),
                                     UNITBORDERTHIKNESS,
                                     (m_complementryRectHeight - (SCALE_HEIGHT(OUTER_TOP_MARGIN) + SCALE_HEIGHT(OUTER_BOTTOM_MARGIN))));

    m_outerBottomBorder->resetGeometry((m_mainRectWidth + SCALE_WIDTH(OUTER_LEFT_MARGIN)),
                                       (m_mainRectHeight - SCALE_HEIGHT(OUTER_BOTTOM_MARGIN)),
                                       (m_complementryRectWidth - (SCALE_WIDTH(OUTER_LEFT_MARGIN) + SCALE_WIDTH(OUTER_RIGHT_MARGIN))),
                                       UNITBORDERTHIKNESS);

    m_innerLeftBorder->resetGeometry((m_mainRectWidth + SCALE_WIDTH(INNER_LEFT_MARGIN)),
                                     (m_mainRectHeight - m_complementryRectHeight + SCALE_HEIGHT(INNER_TOP_MARGIN)),
                                     UNITBORDERTHIKNESS,
                                     (m_complementryRectHeight - (SCALE_HEIGHT(INNER_TOP_MARGIN) + SCALE_HEIGHT(INNER_BOTTOM_MARGIN))));

    m_innerBottomBorder->resetGeometry((m_mainRectWidth + SCALE_WIDTH(INNER_LEFT_MARGIN)),
                                       (m_mainRectHeight - SCALE_HEIGHT(INNER_BOTTOM_MARGIN)),
                                       (m_complementryRectWidth - (SCALE_WIDTH(INNER_LEFT_MARGIN) + SCALE_WIDTH(INNER_RIGHT_MARGIN))),
                                       UNITBORDERTHIKNESS);

    m_complemetryRectangle->resetGeometry((m_mainRectWidth - (2 * SCALE_WIDTH(RECT_RADIUS))),
                                          (m_mainRectHeight - m_complementryRectHeight),
                                          (m_complementryRectWidth + (2 * SCALE_WIDTH(RECT_RADIUS))),
                                          m_complementryRectHeight);
}

void BackGround::showRightpanel()
{
    m_complemetryRectangle->setVisible (true);

    m_mainRightPanelRect->setVisible (true);
    m_innerRightPanelRect->setVisible (true);

    m_outerLeftBorder->setVisible (true);
    m_outerBottomBorder->setVisible (true);

    m_innerLeftBorder->setVisible (true);
    m_innerBottomBorder->setVisible (true);

    if(m_subCloseButton != NULL)
    {
        m_subCloseButton->setVisible(true);
    }

    if(m_subPageheading != NULL)
    {
        m_subPageheading->setVisible(true);
    }
}

void BackGround::hideRightpanel()
{
    m_complemetryRectangle->setVisible (false);

    m_mainRightPanelRect->setVisible (false);
    m_innerRightPanelRect->setVisible (false);

    m_outerLeftBorder->setVisible (false);
    m_outerBottomBorder->setVisible (false);

    m_innerLeftBorder->setVisible (false);
    m_innerBottomBorder->setVisible (false);

    if(m_subCloseButton != NULL)
    {
        m_subCloseButton->setVisible(false);
    }

    if(m_subPageheading != NULL)
    {
        m_subPageheading->setVisible(false);
    }
}

void BackGround::resetGeometry(int startY, int mainRectHeight)
{
    m_startY = startY;
    m_mainRectHeight = mainRectHeight;
    setGeometryForRectangles();
    update();
}

void BackGround::resetGeometry(int startX)
{
    m_startX = startX;
    setGeometryForRectangles();
    update();
}

void BackGround::resetGeometry(int startX, int startY, int width, int height)
{
    m_startX = startX;
    m_startY = startY;
    m_mainRectWidth = width;
    m_mainRectHeight = height;
    setGeometryForRectangles();
    update();
}

void BackGround::rightPanelClose ()
{

}

void BackGround::slotCloseButtonClicked(int indexInPage)
{
    if(indexInPage == SUB_PAGE_CLOSE_BUTTON)
    {
        hideRightpanel();
        rightPanelClose ();
    }
    else
    {
        emit sigClosePage(m_toolbarPageIndex);
    }
}
