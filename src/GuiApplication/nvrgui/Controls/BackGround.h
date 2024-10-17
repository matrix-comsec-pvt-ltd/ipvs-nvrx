#ifndef BACKGROUND_H
#define BACKGROUND_H

#include <QWidget>
#include "Controls/Rectangle.h"
#include "Controls/Heading.h"
#include "Controls/Closebuttton.h"
#include <QGraphicsOpacityEffect>
#include "KeyBoard.h"

#define UNITBORDERTHIKNESS          1

#define LEFT_PANEL_TOP_MARGIN       SCALE_HEIGHT(8)
#define LEFT_PANEL_LEFT_MARGIN      SCALE_WIDTH(8)
#define LEFT_PANEL_BOTTOM_MARGIN    SCALE_HEIGHT(15)

#define OUTER_TOP_MARGIN            15
#define INNER_TOP_MARGIN            ((2 * OUTER_TOP_MARGIN) + OUTER_TOP_MARGIN)

#define OUTER_LEFT_MARGIN           26
#define INNER_LEFT_MARGIN           (10 + OUTER_LEFT_MARGIN)

#define OUTER_RIGHT_MARGIN          15
#define INNER_RIGHT_MARGIN          (10 + OUTER_RIGHT_MARGIN)

#define OUTER_BOTTOM_MARGIN         15
#define INNER_BOTTOM_MARGIN         (10 + OUTER_BOTTOM_MARGIN)

//background types enum
typedef enum
{
    BACKGROUND_TYPE_1 = 0,
    BACKGROUND_TYPE_2,
    BACKGROUND_TYPE_3,
    BACKGROUND_TYPE_4
} BG_TYPE_e;

typedef enum
{
    MAIN_CLOSE_BUTTON,
    SUB_PAGE_CLOSE_BUTTON
}CONTROL_IDEX_e;


class BackGround : public KeyBoard
{
    Q_OBJECT
private:
    Rectangle* m_mainRectangle;
    Rectangle* m_complemetryRectangle;    

    Rectangle* m_leftPanelTopBorder;
    Rectangle* m_leftPanelBottomBorder;
    Rectangle* m_leftPanelLeftBorder;
    Rectangle* m_leftPanelRightBorder;

    Rectangle* m_mainRightPanelRect;
    Rectangle* m_innerRightPanelRect;

    Rectangle* m_outerBottomBorder;
    Rectangle* m_innerBottomBorder;

    Rectangle* m_outerLeftBorder;
    Rectangle* m_innerLeftBorder;

    Heading* m_pageHeading;
    Heading* m_subPageheading;
    QString m_pageHeadingText;
    QString m_subPageheadingText;
    QString m_backgroundColor;
    QString m_complmentryBackgroundColor, m_borderColor_1, m_borderColor_2;

    int m_startX, m_startY;
    int m_mainRectHeight, m_mainRectWidth ;
    int m_complementryRectWidth, m_complementryRectHeight;
    int m_backGroundType;
    bool m_isCloseButton;
    qreal m_opacityValue;

protected:
    TOOLBAR_BUTTON_TYPE_e m_toolbarPageIndex;
    CloseButtton* m_mainCloseButton;
    CloseButtton* m_subCloseButton;

    void virtual rightPanelClose();

public:
     BackGround(int startX,
                int startY,
                int mainRectWidth,
                int mainRectHeight,
                BG_TYPE_e bgtype,
                TOOLBAR_BUTTON_TYPE_e toolbarPageIndex = MAX_TOOLBAR_BUTTON,
                QWidget *parent = 0,
                int complementryRectWidth = 0,
                int complementryRectHeight = 0,
                QString pageHeading = "",
                QString subPageHeading = "",
                bool isCloseButton = true,
                QString backgroundColor = NORMAL_BKG_COLOR,
                qreal opacityValue = 1.0);

     BackGround(int startX,
                int startY,
                int mainRectWidth,
                int mainRectHeight,
                BG_TYPE_e bgtype,
                TOOLBAR_BUTTON_TYPE_e toolbarPageIndex = MAX_TOOLBAR_BUTTON,
                QWidget *parent = 0,
                bool isCloseButton = true,
                QString pageHeading = "",
                QString backgroundColor = NORMAL_BKG_COLOR,
                qreal opacityValue = 1.0,
                QString boardColor = "");

     ~BackGround();

     void createElements();
     void resetGeometry(int startY, int mainRectHeight);
     void resetGeometry(int startX);
     void resetGeometry(int startX, int startY, int width, int height);
     void setGeometryForRectangles();
     void showRightpanel();
     void hideRightpanel();
     void drawRightPanel();
     void drawLeftPanel();
     void drawTopPanel();
     void drawBottomPanel();
     void drawHeading(int centerX, int centerY);
     void drawSubPageHeading(int centerX, int centerY);
     void changeSubPageHeading(QString text);
     CloseButtton* getCloseButton();

signals:
     void sigClosePage(TOOLBAR_BUTTON_TYPE_e index);

protected slots:
    virtual void slotCloseButtonClicked(int indexInPage);
};

#endif // BACKGROUND_H
