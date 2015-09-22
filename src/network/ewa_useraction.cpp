/*******************************************************************************
**
** file: ewa_useraction.cpp
**
** class: EWAUserAction
**
** description:
** base for classes of handle keyboard/mouse/focus events on webpages through JavaScript
**
** 14.10.2009
**
** sendevent@gmail.com
**
*******************************************************************************/

#include "ewa_useraction.h"
#include "ewa_application.h"
#include "ewa_useractionsprocessor.h"

#include <QKeyEvent>
#include <QMouseEvent>
#include <QSettings>

#include <QDebug>

EWAUserAction::EWAUserAction( const QEvent *pEvent, int iTime, EWAWebView *pWebView )
{
    m_pWebView = pWebView;
    
    if( pEvent )
    {
        setEvent( pEvent );
    }
    if( iTime >= 0 )
    {
        setTime( iTime );
    }
}

EWAUserAction::EWAUserAction( const EWAUserAction& other )
{
    this->operator =( other );
}

EWAUserAction::~EWAUserAction()
{
    delete m_pEvent;
}
 
EWAUserAction& EWAUserAction::operator=( const EWAUserAction& other )
{
    m_pWebView = other.m_pWebView;
    //m_clickedElement = other.m_clickedElement;
    m_webViewSize = other.m_webViewSize;

    setTime( other.getTime() );
    setEvent( other.getEvent() );
    
    return *this;
}

 void EWAUserAction::setTime( int t ) 
{
    m_iTime = qMax( t, EWAUserActionsProcessor::getMinActDelay() );
}

bool EWAUserAction::isKeyEvent( QEvent::Type eType ) const
{
    return ( ( eType == QEvent::KeyPress ) || ( eType == QEvent::KeyRelease ) );
}

bool EWAUserAction::isMouseEvent( QEvent::Type eType ) const
{
    return ( ( eType == QEvent::MouseButtonPress ) || ( eType == QEvent::MouseButtonRelease ) );
}

void EWAUserAction::setEvent( const QEvent* pEvent )
{
    if( pEvent )
    {
        if( isKeyEvent( pEvent->type() ) )
        {
            const QKeyEvent *pKeyEvent = static_cast< const QKeyEvent* >( pEvent );
            if( pKeyEvent )
            {
                return rememberKeyEvent( pKeyEvent );
            }
        }
        
        if( isMouseEvent( pEvent->type() ) )
        {
            const QMouseEvent *pMouseEvent = static_cast< const QMouseEvent* >( pEvent );
            if( pMouseEvent )
            {
                return rememberMouseEvent( pMouseEvent );
            }
        }
    }
    
    Q_ASSERT( 0 );
}


void EWAUserAction::rememberKeyEvent( const QKeyEvent *pEvent )
{
    if( pEvent )
    {
        m_pEvent = new QKeyEvent( pEvent->type(), pEvent->key(), pEvent->modifiers(),
            pEvent->text(), pEvent->isAutoRepeat(), pEvent->count() );
    }
}
void EWAUserAction::rememberMouseEvent( const QMouseEvent *pEvent )
{
    if( pEvent )
    {
        QPoint click = pEvent->pos();
        
        /*if( m_pWebView && (pEvent->button() == Qt::LeftButton || pEvent->buttons() == Qt::LeftButton ) )
        {
            m_clickedElement = m_pWebView->getPathFromWebElement( pEvent->pos() );
        }*/
        
        m_pEvent = new QMouseEvent( pEvent->type(), click, pEvent->button(),
            pEvent->buttons(), pEvent->modifiers() );
    }
}

void EWAUserAction::save( QSettings *pSettings )
{
    if( !pSettings || !m_pEvent )
    {
        return;
    }
    
    bool bSaved = false;
    if( isMouseEvent( m_pEvent->type() ) )
    {
        bSaved = saveMouseEvent( pSettings, static_cast< QMouseEvent* >( m_pEvent ) );
        /*if( bSaved )
        {
            pSettings->beginWriteArray( "ClickedElement" );
                for( int nodeCounter = 0; nodeCounter < m_clickedElement.count(); ++nodeCounter )
                {
                    pSettings->setArrayIndex( nodeCounter );
                    pSettings->setValue( QLatin1String( "node" ), m_clickedElement.at( nodeCounter ) );
                }
            pSettings->endArray();
        }*/
        
    }
    else if( isKeyEvent( m_pEvent->type() ) )
    {
        bSaved = saveKeyEvent( pSettings, static_cast< QKeyEvent* >( m_pEvent ) );
    }
    
    if( bSaved )
    {
        pSettings->setValue( QLatin1String( "delay" ), getTime() );
        pSettings->setValue( QLatin1String( "sz" ), getWebViewSize() );
    }
}

bool EWAUserAction::saveMouseEvent( QSettings *pSettings, QMouseEvent *pSrcEvent )
{
    if( pSettings && pSrcEvent )
    {
        pSettings->setValue( QLatin1String( "type" ), pSrcEvent->type() );
        pSettings->setValue( QLatin1String( "pos" ), pSrcEvent->pos() );
        pSettings->setValue( QLatin1String( "button" ), pSrcEvent->button() );
        pSettings->setValue( QLatin1String( "buttons" ), (int)pSrcEvent->buttons() );
        pSettings->setValue( QLatin1String( "modifiers" ), (int)pSrcEvent->modifiers() );
        return true;
    }
    
    return false;
}
bool EWAUserAction::saveKeyEvent( QSettings *pSettings, QKeyEvent *pSrcEvent )
{
    if( pSettings && pSrcEvent )
    {
        pSettings->setValue( QLatin1String( "type" ), (int)pSrcEvent->type() );
        pSettings->setValue( QLatin1String( "key" ), (int)pSrcEvent->key() );
        pSettings->setValue( QLatin1String( "modifiers" ), (int)pSrcEvent->modifiers() );
        pSettings->setValue( QLatin1String( "text" ), pSrcEvent->text() );
        pSettings->setValue( QLatin1String( "autoRepeat" ), pSrcEvent->isAutoRepeat() );
        pSettings->setValue( QLatin1String( "count" ), pSrcEvent->count() );
        return true;
    }
    
    return false;
}

void EWAUserAction::setClickCoords( const QPoint& pnt )
{
    QMouseEvent *pOldEvent = static_cast<QMouseEvent *>( m_pEvent );
    if( pOldEvent )
    {
        QMouseEvent *pNewEvent = new QMouseEvent( pOldEvent->type()
            ,pnt
            ,pOldEvent->button()
            ,pOldEvent->buttons()
            ,pOldEvent->modifiers() );
        
        delete m_pEvent;
        
        m_pEvent = pNewEvent;
    }
}

void EWAUserAction::load( QSettings *pSettings )
{
    if( !pSettings )
    {
        return;
    }
    
    //m_clickedElement.clear();
        
    
    QEvent::Type eType = (QEvent::Type) pSettings->value( QLatin1String( "type" ) ).toInt();
    
    bool bLoaded = false;
    if( isMouseEvent( eType ) )
    {
        m_pEvent = new QMouseEvent( eType
        ,pSettings->value( QLatin1String( "pos" ) ).toPoint()
        ,(Qt::MouseButton)pSettings->value( QLatin1String( "button" ) ).toInt()
        ,(Qt::MouseButtons)pSettings->value( QLatin1String( "buttons" ) ).toInt()
        ,(Qt::KeyboardModifiers)pSettings->value( QLatin1String( "modifiers" ) ).toInt()
        );
        
        /*int nodesCount = pSettings->beginReadArray( "ClickedElement" );
            for( int nodeCounter = 0; nodeCounter < nodesCount; ++nodeCounter )
            {
                pSettings->setArrayIndex( nodeCounter );
                m_clickedElement += pSettings->value( QLatin1String( "node" ), 0 ).toInt();
            }
        pSettings->endArray();*/
        
        bLoaded = true;
    }
    else if( isKeyEvent( eType ) )
    {
        m_pEvent = new QKeyEvent( eType
            ,pSettings->value( QLatin1String( "key" ) ).toInt()
            ,(Qt::KeyboardModifiers)pSettings->value( QLatin1String( "modifiers" ) ).toInt()
            ,pSettings->value( QLatin1String( "text" ) ).toString()
            ,pSettings->value( QLatin1String( "autoRepeat" ) ).toBool()
            ,pSettings->value( QLatin1String( "count" ) ).toInt()
            );
        
        bLoaded = true;
    }
    
    if( bLoaded )
    {
        setTime( pSettings->value( QLatin1String( "delay" ) ).toInt() );
        setWebViewSize( pSettings->value( QLatin1String( "sz" ) ).toSize() );
    }
}

void EWAUserAction::execute( QWebView *webViewPtr ) const
{
    if( !webViewPtr )
    {
        return;
    }
    
    //EWAWebView *pEwaWebView = qobject_cast<EWAWebView*>( webViewPtr );
    //bool bNeedSetFocus = false;
    QEvent *pEventCopy = 0;
    
    if( isMouseEvent( m_pEvent->type() ) )
    {
        QMouseEvent *pSrcMouseEvent = static_cast<QMouseEvent *>( m_pEvent );
        
        QPoint clickCoords = pSrcMouseEvent->pos();
        
        pEventCopy = new QMouseEvent( 
            pSrcMouseEvent->type(), 
            clickCoords, 
            webViewPtr->mapToGlobal( clickCoords ), 
            pSrcMouseEvent->button(), 
            pSrcMouseEvent->buttons(),
            pSrcMouseEvent->modifiers() );
        
        
    }
    
    else if( isKeyEvent( m_pEvent->type() ) )
    {
        QKeyEvent *pSrcKeyEvent = static_cast<QKeyEvent*>( m_pEvent );
        
        pEventCopy = new QKeyEvent( 
            pSrcKeyEvent->type(), 
            pSrcKeyEvent->key(), 
            pSrcKeyEvent->modifiers(),
            pSrcKeyEvent->text(), 
            pSrcKeyEvent->isAutoRepeat(), 
            pSrcKeyEvent->count() );
    }
     
     if( pEventCopy )
     {
        QSize dstSz = getWebViewSize();
        if( webViewPtr->page()->preferredContentsSize() != dstSz )
        {
            webViewPtr->page()->setPreferredContentsSize( dstSz );
        }
        
        EWAApplication::postEvent( webViewPtr, pEventCopy );
     }
}

QString EWAUserAction::getActionValue() const
{
    QEvent::Type eType = m_pEvent->type();
    switch( eType )
    {
        case QEvent::KeyPress:
        case QEvent::KeyRelease: 
        {
            return getKeyValue( m_pEvent );
        }
        case QEvent::MouseButtonPress:
        case QEvent::MouseButtonRelease:
        {
            QPoint resPnt = getClickCoords( m_pEvent );
            QString resStr = QObject::tr( "%1x%2" ).arg( resPnt.x() ).arg( resPnt.y() );
            return resStr;
        }
        default:
        {
            break;
        }
    }
    
    return QString();
}

QString EWAUserAction::getKeyValue( const QEvent *pEvent ) const
{
    QString res = QObject::tr( "EWAUserAction::getKeyValue: Not a Key!" );
    if( pEvent )
    {
        const QKeyEvent *pKeyEvent = static_cast<const QKeyEvent *>( pEvent );
        if( pKeyEvent )
        {
            res = EWAApplication::getKeyName( pKeyEvent->key() );
            if( pKeyEvent->modifiers() == Qt::Key_Shift )
            {
                res = res.toUpper();
            }
            else
            {
                res = res.toLower();
            }
        }
    }
    
    return res;
}



QPoint EWAUserAction::getClickCoords( const QEvent *pEvent ) const
{
    QPoint res( -65536,-65536 );
    if( pEvent )
    {
        const QMouseEvent *pMouseEvent = static_cast<const QMouseEvent *>( pEvent );
        if( pMouseEvent )
        {
            res = pMouseEvent->pos();
        }
    }
    
    return res;
}


bool EWAUserAction::isKeyEvent() const
{
    return m_pEvent ? isKeyEvent( m_pEvent->type() ) : false;
}

bool EWAUserAction::isMouseEvent() const
{
    return m_pEvent ? isMouseEvent( m_pEvent->type() ) : false;
}

QPoint EWAUserAction::getClickCoords() const
{
    return m_pEvent ? getClickCoords( m_pEvent ) : QPoint();
}

QString EWAUserAction::getKeyValue() const
{
    return m_pEvent ? getKeyValue( m_pEvent ) : QString();
}