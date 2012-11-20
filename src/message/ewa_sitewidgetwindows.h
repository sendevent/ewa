#include <QtGlobal>

#ifdef Q_OS_WIN

#ifndef EWA_XMESSAGEWINDOWS_H
#define EWA_XMESSAGEWINDOWS_H

#include "ewa_sitewidget.h"

class EWASiteWidgetWindows : public EWASiteWidget
{
    Q_OBJECT
    
    public:
        typedef void (EWASiteWidgetWindows::*ptr2VoidMemberVoidNonconst)();
        
        EWASiteWidgetWindows( QWidget *pParent = 0 );
        ~EWASiteWidgetWindows();
        
        
        static void call4EveryReady( ptr2VoidMemberVoidNonconst ptr );

        Qt::WindowFlags getFlagsNormal() const;
        Qt::WindowFlags getFlagsMaximized() const;
        
        void onShowDesktop();
        
    private:
        static QWidgetList s_widgetsList;
        bool isReady() const;
        
    protected slots:
        virtual void slotSetViewAlwaysOnDesktop( bool bAndShow );
};

#endif //-- EWA_XMESSAGEWINDOWS_H

#endif //-- Q_OS_WIN