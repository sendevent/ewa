#include "ewa_statisticmodelview.h"

#include "ewa_sitesmodelview.h"
#include "ewa_siteslistmodel.h"
#include "ewa_sitehandle.h"

#include <QHeaderView>
#include <QScrollBar>

EWAStatisticModelView::EWAStatisticModelView( QWidget *parent )
: QTableWidget( parent )
{
    setColumnCount( 6 );
    
    horizontalHeader()->setVisible( true );
    
    QStringList labels;
    labels << tr( "Host" ) 
        << tr( "Period" ) 
        << tr( "Baloon" )
        << tr( "Rules" )
        << tr( "Changes" ) 
        << tr( "AccessMode" );
    
    setHorizontalHeaderLabels( labels );
}

EWAStatisticModelView::~EWAStatisticModelView()
{
}


void EWAStatisticModelView::setModel( QAbstractItemModel *model )
{
    if( model )
    {
        m_modelPtr = qobject_cast<EWASitesListModel*>( model );
        int iRows = m_modelPtr->rowCount();
        setRowCount( iRows );
        
        EWASiteHandle *pSite;
        for( int i = 0; i < iRows; ++i )
        {
            pSite = m_modelPtr->getSiteByNumber( i );
            
            QTableWidgetItem *pHeadertItem = new QTableWidgetItem( pSite->getFavicoPixmap(), "" );
            setVerticalHeaderItem( i, pHeadertItem );
            
            QTableWidgetItem *pHostItem = new QTableWidgetItem( pSite->getHost() );
            setItem( i, 0, pHostItem );
            
            QTableWidgetItem *pPeriodItem = new QTableWidgetItem( QString( "%1" ).arg( pSite->getPeriodInSeconds() ) );
            setItem( i, 1, pPeriodItem );
            
            QTableWidgetItem *pBaloonItem = new QTableWidgetItem( QString( "%1" ).arg( !pSite->extendedMessages() ) );
            setItem( i, 2, pBaloonItem );
            
            QTableWidgetItem *pRulesItem = new QTableWidgetItem( QString( "%1" ).arg( pSite->rulesCount() ) );
            setItem( i, 3, pRulesItem );
            
            QTableWidgetItem *pChangesItem = new QTableWidgetItem( QString( "%1" ).arg( pSite->getShowPolicy()->isUsed() ) );
            setItem( i, 4, pChangesItem );
            
            QTableWidgetItem *pAccessModeItem = new QTableWidgetItem( QString( "%1" ).arg( pSite->replayTypeIsHttp() ) );
            setItem( i, 5, pAccessModeItem );
        }
        
        resizeColumnsToContents();
        
        int iNeeded = horizontalScrollBar()->maximum() * horizontalScrollBar()->singleStep() + horizontalHeader()->length();
        
        emit signalWantUpdateWidth( iNeeded );
        
        qApp->processEvents();
    }
}

