#include <QToolTip>
#include <QHelpEvent>
#include "ewa_systemtrayicon.h"
                      #include "ewa_application.h"
EwaSystemTrayIcon::EwaSystemTrayIcon( QObject * parent )
:QSystemTrayIcon( parent )
{
    //EWAApplication::classAdded( this );
}

EwaSystemTrayIcon::~EwaSystemTrayIcon()
{
    //EWAApplication::classRemoved( this );
}