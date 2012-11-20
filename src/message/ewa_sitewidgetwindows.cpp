#include "ewa_sitewidgetwindows.h"

#ifdef Q_OS_WIN
#include <windows.h>

QWidgetList EWASiteWidgetWindows::s_widgetsList;

EWASiteWidgetWindows::EWASiteWidgetWindows( QWidget *pParent )
:EWASiteWidget( pParent )
{
    s_widgetsList.append( this );
    
    setWindowFlags( getFlagsNormal() );
}

EWASiteWidgetWindows::~EWASiteWidgetWindows()
{
    s_widgetsList.removeAt( s_widgetsList.indexOf( this ) );
}

Qt::WindowFlags EWASiteWidgetWindows::getFlagsNormal() const
{
    Qt::WindowFlags flags = Qt::Tool | Qt::FramelessWindowHint;
    
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
            flags = Qt::SplashScreen | Qt::FramelessWindowHint;
            break;
        }
    }
    
    return flags;
}

Qt::WindowFlags EWASiteWidgetWindows::getFlagsMaximized() const
{
    return Qt::Window | Qt::FramelessWindowHint;
}

void EWASiteWidgetWindows::slotSetViewAlwaysOnDesktop( bool bAndShow )
{
    EWASiteWidget::slotSetViewAlwaysOnDesktop( bAndShow );
    
    if( bAndShow )
    {
		//call4EveryReady( &EWASiteWidgetWindows::show );
		show();
        call4EveryReady( &EWASiteWidgetWindows::lower );
    }
}

/*static*/ void EWASiteWidgetWindows::call4EveryReady( ptr2VoidMemberVoidNonconst ptr )
{
    foreach( QWidget *pWidget, s_widgetsList )
    {
        EWASiteWidgetWindows *pWinWidget = qobject_cast<EWASiteWidgetWindows*>( pWidget );
        if( pWinWidget )
        {
            if( pWinWidget->isReady() )
            {
                (pWinWidget->*ptr)();
            }
        }
    }
}


void EWASiteWidgetWindows::onShowDesktop()
{
    if( isReady() )
    {
		//const int iCount = 5;
        for( int i = 0; i < 5; ++i )
        {
			if( IsWindow( winId() ) )
            ::SetActiveWindow( winId() );
            
			if( IsWindow( winId() ) )
            ::SetForegroundWindow( winId() );
            
			if( IsWindow( winId() ) )
            ::SetWindowPos( winId(), HWND_TOPMOST, 0, 0, 0, 0, SWP_NOSIZE | SWP_NOMOVE );

			if( QSysInfo::windowsVersion() > QSysInfo::WV_5_1 )
			{
				if( IsWindow( winId() ) )
				::SetWindowPos( winId(), HWND_BOTTOM, 0, 0, 0, 0, SWP_SHOWWINDOW | SWP_NOSIZE | SWP_NOMOVE );

				if( IsWindow( winId() ) )
				BringWindowToTop( winId() );
			}
			else
			{
				::SetWindowPos( winId(), HWND_NOTOPMOST, 0, 0, 0, 0, SWP_SHOWWINDOW | SWP_NOSIZE | SWP_NOMOVE );
			}
        }
    }
}


bool EWASiteWidgetWindows::isReady() const
{
    return ( getCurrentMode() == EM_HVM_ALWAYS_ON_DESKTOP )
        && m_bViewModeNormal 
        && ( isVisible() || wasShown() );
}

#endif //-- Q_OS_WIN

