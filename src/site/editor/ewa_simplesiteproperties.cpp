#include "ewa_simplesiteproperties.h"

EWASimpleSiteProperties::EWASimpleSiteProperties( QWidget *pParent )
:QWidget( pParent )
{
    ui.setupUi( this );
}

EWASimpleSiteProperties::~EWASimpleSiteProperties()
{
}

void EWASimpleSiteProperties::setSite( EWASiteHandle *pSite )
{
    ui.pXMsgColorManager->setSite( pSite );
}

void EWASimpleSiteProperties::resetSite()
{
    ui.pXMsgColorManager->resetSite();
}

void EWASimpleSiteProperties::changeEvent( QEvent * event )
{
    if( event->type() == QEvent::LanguageChange )
    {
        ui.retranslateUi( this );
    }

    return QWidget::changeEvent( event );
}
