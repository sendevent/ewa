#include "ewa_useractionsprocessor.h"
#include "ewa_sitehandle.h"
#include "ewa_useraction.h"
#include "ewa_timer.h"

#include <QDebug>

EWAUserActionsProcessor::EWAUserActionsProcessor( QObject *pParent )
:QObject( pParent )
{
    m_pagesActionsCollectionPtr = 0;
    m_pUsrActivityTimer = new EWATimer( this );

    connect( m_pUsrActivityTimer, SIGNAL( signalTimeOut() ),this,SLOT( slotTimerTicked() ) );
}

EWAUserActionsProcessor::~EWAUserActionsProcessor()
{
}

void EWAUserActionsProcessor::setPagesPtr( QVector<EWAUserActionsCollection*>* pPagesAction )
{
    if( !pPagesAction )
    {
        return;
    }
    
    m_pagesActionsCollectionPtr = pPagesAction;
}

void EWAUserActionsProcessor::startEventTimer()
{
    stopEventTimer();
    m_pUsrActivityTimer->start( 10 );
}

void EWAUserActionsProcessor::stopEventTimer()
{
    m_pUsrActivityTimer->stop();
    m_iUserActionDelay = 0;
}

void EWAUserActionsProcessor::stop()
{
    stopEventTimer();
}

 void EWAUserActionsProcessor::reset()
{
    stopEventTimer();
}

EWAUserAction* EWAUserActionsProcessor::getAction( int iPageNum, int iEventNum ) const
{   
    if( m_pagesActionsCollectionPtr )
    {
        if( iPageNum < m_pagesActionsCollectionPtr->count() )
        {
            EWAUserActionsCollection* pPage = m_pagesActionsCollectionPtr->at( iPageNum );
            if( pPage && pPage->getActionsCount() > iEventNum )
            {
                return pPage->getActionsPtr()->at( iEventNum );
            }
        }
    }
    
    return 0;
}

void EWAUserActionsProcessor::setSite( EWASiteHandle *pSite )
{
    if( pSite )
    {
        m_sitePtr = pSite;
        setWebView( m_sitePtr->getWebView() );   
    }
}

void EWAUserActionsProcessor::clear()
{
    if( m_pagesActionsCollectionPtr )
    {
        for( int i = m_pagesActionsCollectionPtr->count()-1; i>= 0 ; i-- )
        {
            EWAUserActionsCollection *pCollection = m_pagesActionsCollectionPtr->at( i );
            m_pagesActionsCollectionPtr->remove( i );
            delete pCollection;
        }
        
        m_pagesActionsCollectionPtr->clear();
    }
}

 void EWAUserActionsProcessor::cloneSettings( const EWAUserActionsProcessor *pOther )
{
    if( pOther )
    {
        if( m_pagesActionsCollectionPtr )
        {
            clear();

            for( int i = 0; i < pOther->m_pagesActionsCollectionPtr->count() ; i++ )
            {
                EWAUserActionsCollection *pCollection = pOther->m_pagesActionsCollectionPtr->at( i );
                EWAUserActionsCollection *pNewCollection = addPage();
                for( int j = 0; j < pCollection->getActionsCount(); j++ )
                {
                    EWAUserAction* pSrcAction = pCollection->getActionsPtr()->at( j );
                    EWAUserAction* pDstAction = pNewCollection->addUserAction( pSrcAction->getEvent() );
                    pDstAction->setTime( pSrcAction->getTime() );
                    pDstAction->setWebViewSize( pSrcAction->getWebViewSize() );
                }
            }
        }
    }
}

EWAUserActionsCollection *EWAUserActionsProcessor::addPage()
{
    EWAUserActionsCollection* pPage = new EWAUserActionsCollection( this );
    m_pagesActionsCollectionPtr->append( pPage );
    return pPage;
}

void EWAUserActionsProcessor::load( QSettings *pSettings )
{
    if( pSettings )
    {
        clear();
        
        int pagesCount = pSettings->beginReadArray( "UserActionsPages" );
        for( int pagesCounter = 0; pagesCounter < pagesCount; pagesCounter++ )
        {
            pSettings->setArrayIndex( pagesCounter );
            EWAUserActionsCollection* pPage = addPage();
            pPage->load( pSettings );
        }
        pSettings->endArray();
    }
}

void EWAUserActionsProcessor::save( QSettings *pSettings )
{
    if( pSettings )
    {
        int pagesCount = m_pagesActionsCollectionPtr->count();
        pSettings->beginWriteArray( "UserActionsPages" );
        for( int pagesCounter = 0; pagesCounter < pagesCount; pagesCounter++ )
        {
            pSettings->setArrayIndex( pagesCounter );
            EWAUserActionsCollection* pPage = m_pagesActionsCollectionPtr->at( pagesCounter );
            pPage->save( pSettings );
        }
        pSettings->endArray();
    }
}
