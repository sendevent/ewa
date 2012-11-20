#include "ewa_sendstatisticdlg.h"
#include "ewa_application.h"
#include "ewa_siteslistmanager.h"

EWASendStatisticsDlg::EWASendStatisticsDlg( QWidget *pParent )
:QDialog( pParent )
{
    ui.setupUi( this );
    
    connect( ui.sitesView, SIGNAL( signalWantUpdateWidth(int) ), 
        this, SLOT( slotUpdateWidth(int) ) );
        
    ui.sitesView->setModel( EWAApplication::getSitesManager()->getModel() );
    ui.pCountLabel->setNum( EWAApplication::getSitesManager()->getModel()->rowCount() );
    
    QString strType = tr( "Serially" );
    if( EWAApplication::getSitesManager()->isDownloadTypeParallel() )
    {
        strType = tr( "Serially" );
    }
    
    ui.pTypeLabel->setText( strType );
    
    
}

EWASendStatisticsDlg::~EWASendStatisticsDlg()
{
}


void EWASendStatisticsDlg::slotUpdateWidth( int deltaW )
{
    resize( deltaW+64, height() );
}