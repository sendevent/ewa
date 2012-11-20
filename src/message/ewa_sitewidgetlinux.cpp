#include <QtGlobal>

#if defined(Q_OS_LINUX)

#include "ewa_sitewidgetlinux.h"
#include "ewa_application.h"

#include <QResizeEvent>
#include <QX11Info>

#include <X11/Xlib.h>
#include <X11/Xatom.h>
#include <X11/Xutil.h>

EWASiteWidgetLinux::EWASiteWidgetLinux( QWidget *pParent )
:EWASiteWidget( pParent )
{
    setWindowFlags( getFlagsNormal() );
}

EWASiteWidgetLinux::~EWASiteWidgetLinux()
{
}

Qt::WindowFlags EWASiteWidgetLinux::getFlagsNormal() const
{
    Qt::WindowFlags flags = Qt::Window | Qt::FramelessWindowHint;

    switch( getCurrentMode() )
    {
        case 0: //-- EM_HVM_AUTOHIDE
        case 1: //-- EM_HVM_ALWAYS_ON_TOP
        {
            flags |=  Qt::WindowStaysOnTopHint;
            break;
        }
        case 2: //-- EM_HVM_ALWAYS_ON_DESKTOP
        {
            flags |= Qt::WindowStaysOnBottomHint;
            break;
        }
    }

    return flags;
}

Qt::WindowFlags EWASiteWidgetLinux::getFlagsMaximized() const
{
    return Qt::Window | Qt::FramelessWindowHint;
}

void EWASiteWidgetLinux::manageTaskPanelItem()
{
    Display *dpy = QX11Info::display();
    Atom skipTaskBar = XInternAtom( dpy, "_NET_WM_STATE_SKIP_TASKBAR", False );

    if( m_bViewModeNormal )
    {
        XChangeProperty( dpy, winId(), XInternAtom( dpy, "_NET_WM_STATE", False ),
            XA_ATOM, 32, PropModeAppend, ( unsigned char* ) &skipTaskBar, 1 );
    }
    else
    {
        XDeleteProperty( dpy, winId(), XInternAtom( dpy, "_NET_WM_STATE", False ) );
    }
}

void EWASiteWidgetLinux::setWindowFlags( Qt::WindowFlags type )
{
    EWASiteWidget::setWindowFlags( type );
    EWAApplication::processEvents();
    manageTaskPanelItem();
}

void EWASiteWidgetLinux::show()
{
    EWASiteWidget::show();
    manageTaskPanelItem();
}

#endif //-- Q_OS_LINUX
