/*******************************************************************************
**
** file: ewa_useractionsplayer.cpp
**
** class: EWAUserActionsPlayer
**
** description:
** class for handling keyboard/mouse events on webpages through JavaScript
**
** 14.10.2009
**
** sendevent@gmail.com
**
*******************************************************************************/

#include "ewa_useractionsplayer.h"
#include "ewa_sitehandle.h"
#include "ewa_timer.h"

EWAUserActionsPlayer::EWAUserActionsPlayer( QObject *pParent )
:EWAUserActionsProcessor( pParent )
{
    m_iPageCounter = m_iEventCounter = m_iExecutedActionsCounter = 0;
    m_pCurrUsrAct = 0;
    m_iOverallEventCounter = 0;
}

EWAUserActionsPlayer::~EWAUserActionsPlayer()
{
}

 void EWAUserActionsPlayer::slotTimerTicked()
{
    if( m_bPaused )
    {
        return;
    }
    
    if( !m_pCurrUsrAct )
    {
        stopEventTimer();
        return;
    }
    
    m_iUserActionDelay++;
    
    executeCurrentEvent();
}



 void EWAUserActionsPlayer::nextPageReady()
{
    m_iPageCounter++;
    m_iEventCounter = 0;
    stopEventTimer();
    
    EWAUserAction *pEvent = getAction( m_iExecutedActionsCounter );
    if( pEvent )
    {
        m_pCurrUsrAct = pEvent;
        startPlay();
        return;
    }
    
    m_pCurrUsrAct = 0;
    m_iEventCounter = 0;
    stopEventTimer();
}

void EWAUserActionsPlayer::startPlay()
{
    EWAApplication::regsterUserActionsReplayer(this);
    stopEventTimer();
    
    if( m_pCurrUsrAct )
    {
        int iTime = m_pCurrUsrAct->getTime();
        int iK = EWAApplication::getUserActionsReplayerIndex(this);
        iTime *= ( iK + iK );
        if( iTime > 0 )
        {
            m_pUsrActivityTimer->start( iTime );
        }
        else
        {
            Q_ASSERT( 0 );//executeCurrentEvent();
        }
        emit signalUserActivitiesReplayStarted();
    }
    else
    {
        stopEventTimer();
    }
}

void EWAUserActionsPlayer::executeCurrentEvent()
{
    stopEventTimer();
    
    if( m_pCurrUsrAct && m_sitePtr->getWebView() && !m_bPaused)
    {
        m_pCurrUsrAct->execute( m_sitePtr->getWebView() );
        m_iExecutedActionsCounter++;
        m_iOverallEventCounter++;
        if( m_sitePtr )
        {
            int iPagesCount = getPagesCount();
            int dAllActions = 0;
            for( int i = 0; i< iPagesCount; i++ )
            {
                dAllActions += m_pagesActionsCollectionPtr->at( i )->getActionsCount();
            }
            double dOnePercent = dAllActions/100.0;
            double dPercents = m_iExecutedActionsCounter/dOnePercent;
            emit signalUserActivitiesReplayProgress( (int)dPercents );
            if( isReplayed() )
            {
                emit signalUserActivitiesReplayFinished();
            }
        }
        m_pCurrUsrAct = getAction( m_iExecutedActionsCounter );
        if( m_pCurrUsrAct )
        {
            startPlay();
            return;
        }
        if( m_sitePtr )
        {
            m_sitePtr->setTypingProgress( 0 );
        }
    }
}

void EWAUserActionsPlayer::stop()
{
    EWAUserActionsProcessor::stop();
    EWAApplication::ungetUserActionsReplayerIndex( this );
    reset();
    
    //-- it's needed to correct "targetLoaded" signal emiting:
    int iPagesCount = getPagesCount();
    for( int i = 0; i< iPagesCount; i++ )
    {
        m_iPageCounter = i;
        m_iExecutedActionsCounter += m_pagesActionsCollectionPtr->at( m_iPageCounter )->getActionsCount();
    }
    
    if( m_sitePtr )
    {
        m_sitePtr->setTypingProgress( 0 );
    }
}

void EWAUserActionsPlayer::reset()
{
    EWAUserActionsProcessor::reset();
    m_iPageCounter = m_iEventCounter = m_iExecutedActionsCounter = 0;
    m_pCurrUsrAct = 0;
}

bool EWAUserActionsPlayer::isReplayed() const
{
    if( m_sitePtr && !m_sitePtr->replayTypeIsHttp() )
    {
        int iPagesCount = getPagesCount();
        int iTotalActionsCount = 0;
        for( int i = 0; i< iPagesCount; i++ )
        {
            iTotalActionsCount += m_pagesActionsCollectionPtr->at( i )->getActionsCount();
        }
        if( !iTotalActionsCount )
            return true;
        
        bool bRes = ( m_iExecutedActionsCounter >= iTotalActionsCount ) && m_iPageCounter+1 >= iPagesCount;
        return bRes;
    }
    
    return true;
}

int EWAUserActionsPlayer::currentPageNumber() const
{
    return m_iPageCounter-1;
}

EWAUserAction* EWAUserActionsPlayer::getAction( int iEventNum ) const
{   
    if( m_pagesActionsCollectionPtr )
    {
        int iActions = 0;
        for( int iPageNum = 0; iPageNum < m_pagesActionsCollectionPtr->count(); iPageNum++ )
        {
            EWAUserActionsCollection* pPage = m_pagesActionsCollectionPtr->at( iPageNum );
            iActions += pPage->getActionsCount();
            if( iActions > m_iExecutedActionsCounter )
            {
                int id = iEventNum - ( iActions - pPage->getActionsCount() );
                return pPage->getActionsPtr()->at( id );
            }
        }
    }
    
    return 0;
}
