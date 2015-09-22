/*******************************************************************************
**
** file: ewa_useraction.cpp
**
** class: EWAUserActionsCollection
**
** description:
** Container for store user actions on the web page.
**
** 16.10.2009
**
** sendevent@gmail.com
**
*******************************************************************************/

#include "ewa_useractionscollection.h"
#include "ewa_useraction.h"
#include "ewa_webview.h"

#include <QSettings>

EWAUserActionsCollection::EWAUserActionsCollection( QObject *pParent )
:QObject( pParent )
{
    m_pUserActions = new QVector< EWAUserAction* >();
}

EWAUserActionsCollection::EWAUserActionsCollection( const EWAUserActionsCollection *pOther )
{
    m_pUserActions = new QVector< EWAUserAction* >();
    cloneDataFrom( pOther );
}

EWAUserActionsCollection::~EWAUserActionsCollection()
{
    clear();
    
    delete m_pUserActions;
}

void EWAUserActionsCollection::clear()
{
    for( int i = m_pUserActions->count()-1; i>= 0 ; i-- )
    {
        EWAUserAction* pAction = m_pUserActions->at( i );
        m_pUserActions->remove( i );
        delete pAction;
    }
    m_pUserActions->clear();
}

EWAUserAction* EWAUserActionsCollection::addUserAction( const QEvent *pEvent, EWAWebView *pWebView )
{
    EWAUserAction *pAction = new EWAUserAction( pEvent, 1, pWebView );
    m_pUserActions->append( pAction );
    
    return pAction;
}

EWAUserAction* EWAUserActionsCollection::addUserAction( const EWAUserAction *pSrcAction )
{
    EWAUserAction *pAction = new EWAUserAction( *pSrcAction );
    m_pUserActions->append( pAction );
    
    return pAction;
}

int EWAUserActionsCollection::getActionsCount() const 
{
    return m_pUserActions->size(); 
}

QVector< EWAUserAction* > *EWAUserActionsCollection::getActionsPtr() const
{
    return m_pUserActions;
}

void EWAUserActionsCollection::save( QSettings *pSettings )
{
    if( !pSettings )
    {
        return;
    }
    
    pSettings->beginWriteArray( QLatin1String( "UserActions" ) );
    int iActionsCount = getActionsCount();
    for( int i = 0; i < iActionsCount; i++ )
    {
        pSettings->setArrayIndex( i );
        
        EWAUserAction *pAction = getActionsPtr()->at( i );
        pAction->save( pSettings );
    }
    pSettings->endArray();
}

void EWAUserActionsCollection::load( QSettings *pSettings )
{
    if( !pSettings )
    {
        return;
    }
    
    clear();
        
    m_pUserActions->clear();
    
    int eventsCount = pSettings->beginReadArray( QLatin1String( "UserActions" ) );
    for( int i = 0; i < eventsCount; i++ )
    {
        pSettings->setArrayIndex( i );
        
        EWAUserAction *pAction = new EWAUserAction();
        pAction->load( pSettings );
        m_pUserActions->append( pAction );
    }
    pSettings->endArray();
}


QEvent *EWAUserActionsCollection::loadEvent( QSettings *pSettings )
{
    QEvent *pEvent = 0;
    QEvent::Type eventType = (QEvent::Type)pSettings->value( QLatin1String( "type" ) ).toInt();
    if( eventType == QEvent::MouseButtonPress || eventType == QEvent::MouseButtonRelease )
    {
        pEvent = loadMouseEvent( pSettings, eventType );
    }
    else if( eventType == QEvent::KeyPress || eventType == QEvent::KeyRelease )
    {
        pEvent = loadKeyEvent( pSettings, eventType );
    }

    return pEvent;
}

QMouseEvent *EWAUserActionsCollection::loadMouseEvent( QSettings *pSettings, QEvent::Type type )
{
    if( pSettings )
    {
        return new QMouseEvent( 
            type
            ,pSettings->value( QLatin1String( "pos" ) ).toPoint()
            ,( Qt::MouseButton ) pSettings->value( QLatin1String( "button" ) ).toInt()
            ,( Qt::MouseButtons ) pSettings->value( QLatin1String( "buttons" ) ).toInt()
            ,( Qt::KeyboardModifiers ) pSettings->value( QLatin1String( "modifiers" ) ).toInt() 
        );
    }
    return 0;
}

QKeyEvent *EWAUserActionsCollection::loadKeyEvent( QSettings *pSettings, QEvent::Type type )
{
    if( pSettings )
    {
        return new QKeyEvent( 
            type
            ,( Qt::Key )pSettings->value( QLatin1String( "key" ) ).toInt() 
            ,( Qt::KeyboardModifiers ) pSettings->value( QLatin1String( "modifiers" ) ).toInt()
            ,pSettings->value( QLatin1String( "text" ) ).toString()
            ,pSettings->value( QLatin1String( "autoRepeat" ) ).toBool()
            ,pSettings->value( QLatin1String( "count" ) ).toInt()
            );
    }

    return 0;
}

int EWAUserActionsCollection::getMinDelay() const
{
    if( !m_pUserActions || m_pUserActions->isEmpty() )
    {
        return -1;
    }
    
    int iMin = m_pUserActions->first()->getTime();
    
    for( int i = 0; i <m_pUserActions->count(); i++ )
    {
        iMin = qMin( iMin, m_pUserActions->at( i )->getTime() );
    }
    
    return iMin;
}

int EWAUserActionsCollection::getMaxDelay() const
{
    if( !m_pUserActions || m_pUserActions->isEmpty() )
    {
        return -1;
    }
    
    int iMax;
    iMax = m_pUserActions->first()->getTime();
    
    for( int i = 0; i <m_pUserActions->count(); i++ )
    {
        iMax = qMax( iMax, m_pUserActions->at( i )->getTime() );
    }
    
    return iMax;
}

void EWAUserActionsCollection::cloneDataFrom( const EWAUserActionsCollection *pCollection )
{
    clear();

    if( pCollection )
    {
        for( int i = 0; i < pCollection->getActionsCount(); i++ )
        {
            addUserAction( pCollection->getActionsPtr()->at( i ) );
        }
    }
}
