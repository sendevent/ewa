#include <QtGlobal>

#ifdef Q_OS_MAC

#ifndef EWA_XMESSAGEMACOS_H
#define EWA_XMESSAGEMACOS_H

#include "ewa_sitewidget.h"


class EWASiteWidgetMacos : public EWASiteWidget
{
    Q_OBJECT
    public:
        EWASiteWidgetMacos( QWidget *pParent = 0 );
        ~EWASiteWidgetMacos();
};


#endif //-- EWA_XMESSAGEMACOS_H

#endif //-- Q_OS_MAC
