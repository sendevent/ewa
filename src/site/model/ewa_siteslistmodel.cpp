/*******************************************************************************
**
** file: ewa_siteslistmodel.cpp
**
** class: EWASitesListModel
**
** description:
** Model for sites set.
**
** 23.03.2009
**
** ewauthor@indatray.com
**
*******************************************************************************/

#include "ewa_siteslistmodel.h"
#include "ewa_sitehandle.h"
#include "ewa_networkaccessmanager.h"

#include <QtGui>

EWASitesListModel::EWASitesListModel( QObject *parent )
 : QAbstractTableModel( parent )
 ,m_pSitesList(new QList<EWASiteHandle*>())
 ,m_pNetworkManagersList( new QList<EWANetworkAccessManager*>() )
 ,m_bModified(false)
{
}

EWASitesListModel::~EWASitesListModel()
{
    delete m_pSitesList;
    delete m_pNetworkManagersList;
}

QVariant EWASitesListModel::data( const QModelIndex &index, int role ) const
{

    QVariant var;
    if ( !index.isValid() )
    {
        return var;
    }
    int rowId = index.row();
    int columnId = index.column();
    int sitesCount = m_pSitesList->count();
    if( rowId < 0
    || rowId > sitesCount
    || !sitesCount )
    {
        return var;
    }
    switch( role )
    {
        case Qt::ToolTipRole:
        {
            QString tooltip;
            EWASiteHandle *pSite = m_pSitesList->at( rowId );
            switch( columnId )
            {
                case 0:
                {

                    QString common;
                    if( pSite->isUsed() )
                    {
                        common  = tr( "exclude<br><b>%1</b><br>from" );
                    }
                    else
                    {
                        common = tr( "include<br><b>%1</b><br>into" );
                    }
                    common = common.arg( pSite->getVisibleMsgTitle() );
                    tooltip = tr( "Click to %1 processing list" )
                    .arg( common );
                    break;
                }
                case 1:
                {
                    if( pSite->isDownloadingActive() )
                    {
                        tooltip = tr( "Abort downloading<br><b>%1</b>" )
                        .arg( pSite->getVisibleMsgTitle() );
                    }
                    else
                    {
                        tooltip = tr( "Start downloading<br><b>%1</b>" )
                        .arg( pSite->getVisibleMsgTitle() );
                    }
                    break;
                }
                case 2:
                {
                    tooltip = tr( "<img margin=-10 padding=-10 src=\"%1\">" ).arg( pSite->getThumbnailFileName() );
                    break;
                }
                case 3:
                {
                    QString info;
                    if( !pSite->replayTypeIsHttp() && pSite->getTypingProgress() )
                    {
                        info = tr( "User's Activities reproducing: %1" )
                        .arg( pSite->getTypingProgress() );
                    }
                    else if( pSite->isDownloadingActive() )
                    {
                        info = tr( "Downloading... %1%" )
                        .arg( pSite->getDownloadProgress() );
                    }
                    else if( pSite->isStarted() && pSite->isUsed() )
                    {
                        info = tr( "Next check will be in: %1" )
                        .arg( pSite->getCountdownCurrentHMS() );
                    }
                    else
                    {
                        info = tr( "Time/Download/User's Activities Reproducing progress" );
                    }

                    tooltip = tr( "<b>%1</b><br>%2<br>Double-click to edit site options" )
                    .arg( pSite->getUrlStrLimit32() )
                    .arg( info );

                    break;
                }
            }
            if( columnId != 2 ) 
            {
                tooltip.append( tr( "<br>Downloaded %1 times" ).arg( pSite->getDownloadCounter() ) );
            }
            var.setValue( tooltip );
            break;
        }
        default:
        {
            var.setValue( m_pSitesList->at( rowId )->getPointer() );
            break;
        }
    }

    return var;
}

Qt::ItemFlags EWASitesListModel::flags( const QModelIndex &index ) const
{
    Qt::ItemFlags flags = Qt::NoItemFlags;
    if ( index.isValid() )
    {
        flags = ( Qt::ItemIsEnabled | Qt::ItemIsSelectable );
        int column = index.column();
        if( column == 0 || column == 1 )
            flags |= Qt::ItemIsEditable;
    }
    return flags;
}

int EWASitesListModel::rowCount( const QModelIndex& index ) const
{
    Q_UNUSED( index );
    return m_pSitesList->size();
}

int EWASitesListModel::columnCount( const QModelIndex& parent ) const
{
    Q_UNUSED( parent );
    return 4;
}

EWASiteHandle* EWASitesListModel::addSite( const EWASiteHandle *pCopyIt )
{
    int row = m_pSitesList->count();

    EWANetworkAccessManager *pNAManager = new EWANetworkAccessManager();
    EWASiteHandle *pSite = 0;
    if( pCopyIt )
    {
        pSite = new EWASiteHandle( pCopyIt );
    }
    else
    {
        pSite = new EWASiteHandle( qobject_cast<QObject*>( this ) );
    }
    
    pSite->setEWANetworkManagerPtr( pNAManager );
    pNAManager->setSite( pSite );

    QModelIndex index = QModelIndex();
    beginInsertRows( index, row, row );
        m_pSitesList->append( pSite );
        m_pNetworkManagersList->append( pNAManager );
    endInsertRows();

    reset();

    connect( pSite, SIGNAL( signalSiteLoadProgress(int) ), this, SLOT( slotSiteChanged() ) );
    connect( pSite, SIGNAL( signalSiteTypingProgress(int) ), this, SLOT( slotSiteChanged() ) );
    connect( pSite, SIGNAL( signalOneSecond() ), this, SLOT( slotSiteChanged() ) );
    connect( pSite, SIGNAL( signalUsageChanged() ), this, SLOT( slotSiteChanged() ) );
    connect( pSite, SIGNAL( signalFaviconChanged() ), this, SLOT( slotSiteChanged() ) );
    connect( pSite, SIGNAL( signalSiteModified() ), this, SLOT( slotSetModified() ) );
    
    connect( pSite->getWebPage(), SIGNAL( loadStarted() ), this, SLOT( slotProcessLoadingStateChange() ) );
    connect( pSite->getWebPage(), SIGNAL( loadFinished(bool) ), this, SLOT( slotProcessLoadingStateChange() ) );
    
    setDirty();
    
    return pSite;
}

bool EWASitesListModel::removeSite( EWASiteHandle *pSite )
{
    if( pSite )
    {
        long id = m_pSitesList->indexOf( pSite );
        if( id != -1 )
        {
            return removeRow( id );
        }
    }

    return false;
}


bool EWASitesListModel::removeRows( int row, int count, const QModelIndex &parent )
{
    if ( parent.isValid() )
    {
        return false;
    }

    if ( row < 0
    || row >= m_pSitesList->size()
    || row + count <= 0
    || row + count > m_pSitesList->size() )
    {
        return false;
    }

    int beginRow = qMax( 0, row );
    int endRow = qMin( row + count - 1, m_pSitesList->size() - 1 );

    while ( endRow >= beginRow )
    {
        removeRow( endRow );
        --endRow;
    }

    return true;
}

bool EWASitesListModel::removeRow( int row )
{
    if ( row >= 0
    && row < m_pSitesList->size()
    && m_pSitesList->size() )
    {
        EWASiteHandle *pSite = m_pSitesList->takeAt( row );
        EWANetworkAccessManager *pManager = m_pNetworkManagersList->takeAt( row );
        if( pSite && pManager)
        {
            beginRemoveRows( QModelIndex(), row, row );
                
                delete pSite;
                delete pManager;
                
            endRemoveRows();
            
            setDirty();
            
            reset();

            return true;
        }
    }
    
    return false;
}

EWASiteHandle *EWASitesListModel::getSiteByNumber( int number ) const
{
    if( number >= 0 && number < m_pSitesList->count() )
    {
        return m_pSitesList->at( number );
    }

    return 0;
}

int EWASitesListModel::getSiteNumber( EWASiteHandle *pSite ) const
{
    if( pSite )
    {
        return m_pSitesList->indexOf( pSite );
    }

    return -1;
}

void EWASitesListModel::slotSiteChanged()
{
    EWASiteHandle *pSite = qobject_cast<EWASiteHandle *>( sender() );
    if( pSite )
    {
        int row = m_pSitesList->indexOf( pSite );
        if( row != -1 )
        {
            QModelIndex modId0 = index( row, 0 );
            QModelIndex modId3 = index( row, 3 );
            emit dataChanged( modId0, modId3 );
        }
    }
}

QModelIndex EWASitesListModel::increaseSitePosition( int pos )
{
    if( pos < 0 || pos >= rowCount()-1 )
    {
        return QModelIndex();
    }
    int nextId = pos+1;

    m_pSitesList->swap( pos, nextId );
    m_pNetworkManagersList->swap( pos, nextId );
    
    setDirty();
    
    QModelIndex modId0 = index( pos, 0 );
    QModelIndex modId1 = index( nextId, 3 );
    
    emit dataChanged( modId0, modId1 );
    return modId1;
}

QModelIndex EWASitesListModel::decreaseSitePosition( int pos )
{
    if( pos <= 0 || pos >= rowCount() )
    {
        return QModelIndex();
    }

    int nextId = pos-1;

    m_pSitesList->swap( pos, nextId );
    m_pNetworkManagersList->swap( pos, nextId );
    
    setDirty();

    QModelIndex modId0 = index( nextId, 0 );
    QModelIndex modId1 = index( pos, 3 );

    emit dataChanged( modId0, modId1 );
    return modId0;
}

void EWASitesListModel::slotSetModified()
{
    setDirty();
}

void EWASitesListModel::setDirty()
{
    setModified( true );
}

void EWASitesListModel::setClean()
{
    setModified( false );
}

void EWASitesListModel::slotProcessLoadingStateChange()
{
    EWASiteHandle *pSite = qobject_cast<EWASiteHandle*>( sender() );
    if( pSite )
    {
        int iRow = getSiteNumber( pSite );
        emit dataChanged( index( iRow, 1 ), index( iRow, 3 ) );
    }
}