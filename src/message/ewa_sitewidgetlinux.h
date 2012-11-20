#ifndef EWA_XMESSAGELINUX_H
#define EWA_XMESSAGELINUX_H

#include <QtGlobal>

#if defined(Q_OS_LINUX)

#include "ewa_sitewidget.h"

class EWASiteWidgetLinux : public EWASiteWidget
{
    public:
        EWASiteWidgetLinux( QWidget *pParent = 0 );
        ~EWASiteWidgetLinux();

        virtual Qt::WindowFlags getFlagsNormal() const;
        virtual Qt::WindowFlags getFlagsMaximized() const;

        void setWindowFlags( Qt::WindowFlags type );

        virtual void show();

    protected:
        void manageTaskPanelItem();
};

#endif //-- Q_OS_LINUX

#endif //-- EWA_XMESSAGELINUX_H
