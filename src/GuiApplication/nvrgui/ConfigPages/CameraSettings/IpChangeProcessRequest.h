#ifndef IPCHANGEPROCESSREQUEST_H
#define IPCHANGEPROCESSREQUEST_H

#include <QWidget>
#include "ApplController.h"
#include "EnumFile.h"

class IpChangeProcessRequest : public QWidget
{
    Q_OBJECT
public:
    explicit IpChangeProcessRequest(QWidget *parent = 0);
    
    void paintEvent (QPaintEvent *);
signals:
    
public slots:
    
};

#endif // IPCHANGEPROCESSREQUEST_H
