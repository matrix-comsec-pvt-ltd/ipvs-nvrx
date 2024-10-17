#ifndef ABOUTUS_H
#define ABOUTUS_H

#include <QWidget>
#include "Controls/TextLabel.h"
#include "Controls/Rectangle.h"
#include "Controls/Closebuttton.h"
#include "Controls/Heading.h"
#include "ApplController.h"

class AboutUs : public QWidget
{
    Q_OBJECT
private:
    Heading         *pageHeading;
    TextLabel       *textLabel;
    ApplController  *applController;
    Rectangle       *backGround;
    CloseButtton    *closeButtton;

public:
    explicit AboutUs(QWidget *parent = 0);
    ~AboutUs();

    void showEvent(QShowEvent *event);

signals:
    void sigClosePage(TOOLBAR_BUTTON_TYPE_e index);

public slots:
    void slotButtonClick(int);
};

#endif // ABOUTUS_H

