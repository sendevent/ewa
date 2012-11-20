/*******************************************************************************
**
** file: ewa_sitesmodelview.cpp
**
** class: EWASitesModelView
**
** description:
** TableView for sites list model.
**
** 23.03.2009
**
** ewauthor@indatray.com
**
*******************************************************************************/

#include "ewa_sitesmodelview.h"
#include "ewa_siteslistmodel.h"

#include "ewa_siteslistmodeldelegate.h"
#include "ewa_sitefavicondelegate.h"
#include "ewa_siteusagedelegate.h"
#include "ewa_siteactiondelegate.h"

#include <QHeaderView>

EWASitesModelView::EWASitesModelView( QWidget *parent )
:QTableView( parent )
{
    setEditTriggers( QAbstractItemView::AllEditTriggers	 );
    setItemDelegateForColumn( 3, new EWASitesListModelDelegate( this ) );
    setItemDelegateForColumn( 2, new EWASiteFaviconDelegate( this ) );
    setItemDelegateForColumn( 1, new EWASiteActionDelegate( this ) );
    setItemDelegateForColumn( 0, new EWASiteUsageDelegate( this ) );
    verticalHeader()->resizeSections( QHeaderView::Custom );
    horizontalHeader()->resizeSections( QHeaderView::Custom );

    setEditTriggers( QAbstractItemView::AllEditTriggers );
    setTabKeyNavigation( false );
    setProperty( "showDropIndicator", QVariant( false ) );
    setSelectionMode( QAbstractItemView::SingleSelection );
    setSelectionBehavior( QAbstractItemView::SelectRows );
    setMouseTracking( true );
    setIconSize( QSize( 16, 16 ) );
    setShowGrid( false );
    setCornerButtonEnabled( false );
    horizontalHeader()->setVisible( false );
    horizontalHeader()->setDefaultSectionSize( 16 );
    horizontalHeader()->setHighlightSections( false );
    horizontalHeader()->setMinimumSectionSize( 16 );
    verticalHeader()->setVisible( false );
    verticalHeader()->setDefaultSectionSize( 16 );
    verticalHeader()->setHighlightSections( false );
    verticalHeader()->setMinimumSectionSize( 16 );
    
    setAlternatingRowColors( true );
}

EWASitesModelView::~EWASitesModelView()
{
}

void EWASitesModelView::setModel( QAbstractItemModel *model )
{
    m_modelPtr = qobject_cast<EWASitesListModel*>( model );
    
    QTableView::setModel( model );
    reset();

    normalizeColumnsWidths();
}

void EWASitesModelView::mouseReleaseEvent( QMouseEvent *event )
{
    if( event->button() == Qt::RightButton )
    {
        emit customContextMenuRequested( mapToGlobal( event->pos() ) );
    }

    QTableView::mouseReleaseEvent( event );
}

void EWASitesModelView::resizeEvent( QResizeEvent *event )
{
    QAbstractItemView::resizeEvent( event );

    normalizeColumnsWidths();
}

void EWASitesModelView::normalizeColumnsWidths()
{
    if( isVisible() )
    {//-- "It's a kind of maaagic, maaagic..."
        setColumnWidth( 0, 16 );
        setColumnWidth( 1, 16 );
        setColumnWidth( 2, 16 );
        
        int deltaWidth = 45; //-- width of first 3 colums + spasing
        if( verticalScrollBar()->isVisible() )
        {
            deltaWidth += verticalScrollBar()->width();
        }

        for( int i = 0; i<model()->columnCount()-1; i++ )
        {
            deltaWidth += columnSpan( 0, i )*2;
        }

        int newWidth = width() - deltaWidth;
        setColumnWidth( 3, newWidth );
        
        update();
    }
}

void EWASitesModelView::reset()
{
    QTableView::reset();
    normalizeColumnsWidths();
}


void EWASitesModelView::showEvent( QShowEvent *event )
{
    QTableView::showEvent( event );
    normalizeColumnsWidths();
}

void EWASitesModelView::dataChanged( const QModelIndex & topLeft, const QModelIndex & bottomRight )
{
    QAbstractItemView::dataChanged( topLeft, bottomRight );
    
    updateRow( topLeft.row() );
}

void EWASitesModelView::mouseMoveEvent( QMouseEvent *event )
{
    QTableView::mouseMoveEvent( event );
    
    int iHoveredRow = -1;
    
    QModelIndex index = indexAt( event->pos() );
    if( index.isValid() )
    {
        iHoveredRow = index.row();
    }
    
    m_modelPtr->setRowUnderMouse( iHoveredRow );
    updateRow( iHoveredRow );
}

void EWASitesModelView::leaveEvent( QEvent *event )
{
    QTableView::leaveEvent( event );
    
    int iPrevRow = m_modelPtr->getRowUnderMouse();
    if( -1 != iPrevRow )
    {
        m_modelPtr->setRowUnderMouse( -1 );
        updateRow( iPrevRow );
    }
}

void EWASitesModelView::updateRow( int iRow )
{
    if( m_modelPtr 
        && iRow >= 0 
        && iRow < m_modelPtr->rowCount() )
    {
        for( int i = 0; i < m_modelPtr->columnCount(); ++i )
            update( m_modelPtr->index( iRow, i ) );
    }
}