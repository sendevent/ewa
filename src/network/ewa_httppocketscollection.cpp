#include "ewa_httppocketscollection.h"
#include "ewa_sitehandle.h"
#include "ewa_siteroutepoint.h"

EWAHttpPocketsCollection::EWAHttpPocketsCollection( QObject *pParent )
:QObject( pParent )
{
    m_pRoute = new QList<EWASiteRoutePoint*>();
}

EWAHttpPocketsCollection::~EWAHttpPocketsCollection()
{
    clear();
    
    delete m_pRoute;
}

int EWAHttpPocketsCollection::packetsCollectionCount() const
{
    return m_pRoute->size();
}

EWASiteRoutePoint* EWAHttpPocketsCollection::addPacket( EWASiteRoutePoint* pPacket )
{
    EWASiteRoutePoint *pNewPoint = new EWASiteRoutePoint( pPacket );
    m_pRoute->append( pNewPoint );
    
    return pNewPoint;
}

EWASiteRoutePoint* EWAHttpPocketsCollection::getPacketAt( int i ) const
{
    if( m_pRoute && i >= 0 && i < m_pRoute->size() )
    {
        return m_pRoute->at( i );
    }
    
    return 0;
}

void EWAHttpPocketsCollection::clear()
{
    if( !m_pRoute->isEmpty() )
    for( int i = m_pRoute->count()-1; i >=0; --i )
    {
        EWASiteRoutePoint *pPoint = m_pRoute->at( i );
        m_pRoute->removeAt( i );
        delete pPoint;
    }
    
    m_pRoute->clear();
}

void EWAHttpPocketsCollection::load( QSettings *pSettings )
{
    clear();
    
    if( pSettings )
    {
        int requestCount = pSettings->beginReadArray( "requests" );
        for( int requestCounter = 0; requestCounter < requestCount; ++requestCounter )
        {
            pSettings->setArrayIndex( requestCounter );

            QNetworkRequest req;
            EWASiteRoutePoint *pStore = addPacket();
            pStore->load( pSettings );
        }
        pSettings->endArray();
    }
}

void EWAHttpPocketsCollection::save( QSettings *pSettings )
{
    if( pSettings )
    {
        int requestCount = m_pRoute->count();
        pSettings->beginWriteArray( "requests" );
        for( int requestCounter = 0; requestCounter < requestCount; ++requestCounter )
        {
            pSettings->setArrayIndex( requestCounter );

            QNetworkRequest req;
            EWASiteRoutePoint *pStore = m_pRoute->at( requestCounter );
            pStore->save( pSettings );
        }
        pSettings->endArray();
    }
}

void EWAHttpPocketsCollection::setSite( EWASiteHandle *pSite )
{
    if( pSite )
    {
        m_sitePtr = pSite;
    }
}

void EWAHttpPocketsCollection::cloneSettings( const EWAHttpPocketsCollection *pOther )
{
    clear();
    delete m_pRoute;

    m_pRoute = new QList<EWASiteRoutePoint*>();
    for( int i = 0; i < pOther->packetsCollectionCount(); i++ )
    {
        addPacket( pOther->getPacketAt( i ) );
    }
}
