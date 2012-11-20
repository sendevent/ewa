/*******************************************************************************
**
** file: ewa_useractionsrecorder.cpp
**
** class: EWAUserActionsRecorder
**
** description:
** class for handling keyboard/mouse events on webpages through JavaScript
**
** 14.10.2009
**
** ewauthor@indatray.com
**
*******************************************************************************/

#include "ewa_useractionsrecorder.h"

EWAUserActionsRecorder::EWAUserActionsRecorder( QObject *pParent )
:EWAUserActionsProcessor( pParent )
{
}

EWAUserActionsRecorder::~EWAUserActionsRecorder()
{
}

 void EWAUserActionsRecorder::reset()
{
    EWAUserActionsProcessor::reset();
    m_pagesActionsCollectionPtr->clear();
}

 void EWAUserActionsRecorder::nextPageReady()
{
    addPage();
    startEventTimer();
}

 void EWAUserActionsRecorder::slotTimerTicked()
{
    if( m_bPaused )
    {
        return;
    }
    m_iUserActionDelay++;

    if( m_iUserActionDelay >= 1000 )
    {
        stopEventTimer(); //-- m_iUserActionDelay now is 0;
        m_iUserActionDelay = 1000;
    }
}

void EWAUserActionsRecorder::rememberEvent( QEvent *pEvent )
{
    if( pEvent && m_pagesActionsCollectionPtr && m_pagesActionsCollectionPtr->size() )
    {
        EWAUserAction *pAction = m_pagesActionsCollectionPtr->last()->addUserAction( pEvent, m_pWebView );
        if( pAction )
        {
            pAction->setTime( m_iUserActionDelay );
            pAction->setWebViewSize( m_pWebView->page()->preferredContentsSize() );
        }
        startEventTimer();
    }
}
