/*******************************************************************************
**
** file: ewa_webpage.cpp
**
** class: EWAWebPage
**
** description:
** QWebPage etension for internalusage
**
** 23.03.2009
**
** ewauthor@indatray.com
**
*******************************************************************************/

#include "ewa_webpage.h"
#include "ewa_networkaccessmanager.h"
#include "ewa_sitehandle.h"

#include <QMessageBox>
#include <QWebHistory>

EWAWebPage::EWAWebPage( QObject *parent, const QSize& maxSz )
:QWebPage( parent )
{
    m_sitePtr = 0;
    m_ENAManagerPtr = 0;

    m_qstrUserAgent = getOriginalUserAgent();

    mainFrame()->setScrollBarPolicy( Qt::Horizontal, Qt::ScrollBarAlwaysOff );
    mainFrame()->setScrollBarPolicy( Qt::Vertical, Qt::ScrollBarAlwaysOff );
    
    setPreferredContentsSize( maxSz );

    setForwardUnsupportedContent( true );

    m_bRecordAllowed = false;
    m_bIsRecording = false;
    m_bSilentlyOnce = false;
    
    connect( mainFrame(), SIGNAL( contentsSizeChanged(const QSize&) ), 
        this, SIGNAL( contentsSizeChanged(const QSize&) ) );
    
    connect( this, SIGNAL( statusBarVisibilityChangeRequested(bool) ),
        this, SLOT(slostStatusBarVisibilityChangeRequested(bool) ) );
    
    connect( this, SIGNAL( toolBarVisibilityChangeRequested(bool) ),
        this, SLOT(slotToolBarVisibilityChangeRequested(bool) ) );
    
    connect( this, SIGNAL(downloadRequested(const QNetworkRequest&)),
        this, SLOT(slotDownloadRequested(const QNetworkRequest&)) );
    
    connect( this, SIGNAL(geometryChangeRequested(const QRect&)),
        this, SLOT(slotGeometryChangeRequested(const QRect&)) );
    
    connect( this, SIGNAL(restoreFrameStateRequested(QWebFrame*)),
        this, SLOT(slotRestoreFrameStateRequested(QWebFrame*)) );
        
    connectMe();

    setBlank();
}

void EWAWebPage::connectMe()
{
    connect( this, SIGNAL( loadStarted() ), this, SLOT( slotLoadStarted() ) );
    connect( this, SIGNAL( loadFinished(bool) ),this, SLOT( slotLoadFinished(bool) ) );
    connect( this, SIGNAL( unsupportedContent(QNetworkReply*) ),
        this, SLOT( handleUnsupportedContent(QNetworkReply*) ) );
    connect( this, SIGNAL(frameCreated(QWebFrame*)),
        this, SLOT(slotFrameCreated(QWebFrame*)) );
}

void EWAWebPage::disconnectMe()
{
    disconnect( this, SIGNAL( loadStarted() ), this, SLOT( slotLoadStarted() ) );
    disconnect( this, SIGNAL( loadFinished(bool) ),this, SLOT( slotLoadFinished(bool) ) );
    disconnect( this, SIGNAL( unsupportedContent(QNetworkReply*) ),
        this, SLOT( handleUnsupportedContent(QNetworkReply*) ) );
}

EWAWebPage::~EWAWebPage()
{
    m_sitePtr = 0;
    m_ENAManagerPtr = 0;
}

void EWAWebPage::setEWANetworkManagerPtr( EWANetworkAccessManager *pManager )
{
    if( pManager )
    {
        m_ENAManagerPtr = pManager;
        setNetworkAccessManager( qobject_cast<QNetworkAccessManager*>(m_ENAManagerPtr) );
    }
}

void EWAWebPage::setSite( EWASiteHandle *pSite )
{
    if( pSite )
    {
        m_sitePtr = pSite;
        setEWANetworkManagerPtr( m_sitePtr->getEWANetworkManagerPtr() );

        m_bIsRecording = true;
        m_bSilentlyOnce = false;
    }
}

QString EWAWebPage::getOriginalUserAgent() const
{
    QString strUA = QWebPage::userAgentForUrl( QUrl() );
    //-- spaces not allowed:
    QString appNameSrc = EWAApplication::applicationName();
    QString appNameDst( appNameSrc );
    appNameDst.replace( " ", "" );//-- "EnhancedWebAssistant";
#ifdef EWA_DBG
    appNameDst += "(d)";
#endif    //-- EWA_DBG
    strUA.replace( appNameSrc, appNameDst );
    
    return strUA;
}

QString EWAWebPage::userAgentForUrl( const QUrl& ) const
{
    
    return getUserAgent();
}

bool EWAWebPage::acceptNavigationRequest( QWebFrame *frame, const QNetworkRequest& request, NavigationType type )
{
    bool bRes = false;
    if( m_sitePtr )
    {
        bool bRememberCan = isCanRemember();
        bool bRememberNeed = isNeedRemember();
        bool bUser = ( type == QWebPage::NavigationTypeLinkClicked 
            || type == QWebPage::NavigationTypeFormSubmitted );
            
        if( bRememberCan && bRememberNeed && bUser )
        {
            m_sitePtr->getEWANetworkManagerPtr()->m_bNeedSaveRequest = true;
            setCanRemember( false );
        }
        
        bRes = QWebPage::acceptNavigationRequest( frame, request, type );
    }
    
    return bRes;
}

bool EWAWebPage::loadNextRequest()
{
    if( m_sitePtr && !history()->canGoForward() )
        return m_sitePtr->getEWANetworkManagerPtr()->loadNextRequest();
    
    return false;
}

void EWAWebPage::javaScriptAlert( QWebFrame *frame, const QString& msg )
{
    if( EWAApplication::settings()->m_pGroupWebkit->getUseJSShowAlerts() )
    {
        QMessageBox::information( frame->page()->view(), tr( "JavaScript on %1 message:" ).arg( frame->url().toString() ),
        msg );
    }
    showJSOutput( tr( "JS on %1:\n%2\n" ).arg( frame->url().toString() ).arg( msg ) );
}
bool EWAWebPage::javaScriptConfirm( QWebFrame *frame, const QString& msg )
{
    showJSOutput( tr( "JS on %1:\n%2\nCONFIRMED\n" ).arg( frame->url().toString() ).arg( msg ) );
    return true;
}

void EWAWebPage::javaScriptConsoleMessage( const QString& message, int lineNumber, const QString& sourceID )
{
    QString msg = tr( "source: %1, line number: %2, message: %3").arg( sourceID ).arg( lineNumber ).arg( message );
    showJSOutput( tr( "JS on %1:\n%2\n" ).arg( this->mainFrame()->url().toString() ).arg( msg ) );
}

void EWAWebPage::handleUnsupportedContent( QNetworkReply *reply )
{
    if( reply->error() == QNetworkReply::NoError )
    {
        EWAWebView *pMyView = static_cast<EWAWebView*>( view() );
        if( pMyView )
        {
            pMyView->showDownloader( reply );
        }
        return;
    }
    
    setBlank();
}

void EWAWebPage::slotLoadStarted()
{
    //-- when page not loaded there can be
    //-- internal redirections and JS submiting -
    //-- so we don't allow to user click anything:

    setCanRemember( false );
}

void EWAWebPage::slotLoadFinished( bool ok )
{
    /*QString currentUrl = mainFrame()->url().toString();
    if( !currentUrl.compare( "qrc:/" ) 
        || !currentUrl.compare( "about:blank" )
        || isBlank() )
    {
        return;
    }*/
    
    if( isNeedRemember() )
    {
        setCanRemember( true );
    }
    else if( ok && !loadNextRequest() )
    {
        if( m_bSilentlyOnce )
        {
            m_bSilentlyOnce = false;
        }
        else
        {
            m_szContentsSize = mainFrame()->contentsSize();
            
            if( m_sitePtr && m_sitePtr->isReplayFinished() )
            {
                emit signalTargetLoaded( ok );
            }
        }
    }
}

bool EWAWebPage::downloadPresaved()
{
    history()->clear();
    
    if( !m_sitePtr )
    {
        return false;
    }

    m_bIsRecording = false;
    m_bSilentlyOnce = false;

    m_ENAManagerPtr->clear4NewSession();
    mainFrame()->setUrl( m_sitePtr->getUrl() );

    return true;
}

bool EWAWebPage::downloadRegular()
{
    history()->clear();
    
    if( !m_sitePtr )
    {
        return false;
    }

    m_bIsRecording = true;
    m_bSilentlyOnce = false;
    setCanRemember( true );
    mainFrame()->setUrl( m_sitePtr->getUrl() );

    return true;
}

EWAWebPage& EWAWebPage::operator=( const EWAWebPage& other )
{
    m_sitePtr = other.m_sitePtr;
    setSite( other.m_sitePtr );

    m_bRecordAllowed = other.m_bRecordAllowed;
    m_ENAManagerPtr = other.m_ENAManagerPtr;
    m_bIsRecording = other.m_bIsRecording;
    m_bSilentlyOnce = other.m_bSilentlyOnce;
    m_szContentsSize = other.m_szContentsSize;
    m_qstrUserAgent = other.m_qstrUserAgent;

    return *this;
}

void EWAWebPage::showJSOutput( const QString& msg )
{
    EWAApplication::instance()->displayInfoMessage( msg );
}

void EWAWebPage::setBlank()
{
    blockSignals( true );
    if( mainFrame() )
        mainFrame()->setHtml( EWAApplication::instance()->getBlankPageSource() );
    blockSignals( false );
}

bool EWAWebPage::isBlank() const
{
    return !EWAApplication::instance()->getBlankPageSource().compare( mainFrame()->toHtml(), Qt::CaseInsensitive );
}

QWebFrame* EWAWebPage::getCurrentFrame() const
{
    QWebFrame* pFrame = currentFrame();
    if( !pFrame )
        pFrame = mainFrame();
    return pFrame;
}

void EWAWebPage::slotFrameCreated( QWebFrame *pFrame )
{
    connect( pFrame, SIGNAL( loadFinished(bool) ),
        this, SLOT( slotFrameLoaded(bool) ) );
}

void EWAWebPage::setErrorMessage( const QString& urlStr, const QString& errStr, QWebFrame *pFrame )
{
    if( !pFrame )
    {
        pFrame = mainFrame();
    }
    
    QString msgStr = EWAApplication::translate( this, tr( "<H3>EWA can't get this webpage contents.</H3>"
        "The document at <a href=\"%1\">%1</a> might be temporarily down<br>"
        "or it may have moved permanently to a new web address.<br>"
        "Below is the original error description:<br>"
        "<b>%2</b>" ).toUtf8().data() )
        .arg( urlStr )
        .arg( errStr );
        
    QString html = EWAApplication::instance()->getBlankPageSourceTemplate()
        .arg( msgStr );

    setSilentlyOnce( true );
    pFrame->setHtml( html, QUrl( urlStr ) );
}

void EWAWebPage::slotFrameLoaded( bool bOk )
{
    if( !bOk && m_ENAManagerPtr && m_ENAManagerPtr->lastErrorCode() != QNetworkReply::NoError )
    {
        QWebFrame *pFrame = qobject_cast<QWebFrame*>( sender() );
        if( pFrame )
        {
            setErrorMessage( pFrame->url().toString(), m_ENAManagerPtr->lastErrorString(), pFrame );
        }
    }
}

bool EWAWebPage::shouldInterruptJavaScript()
{
    bool bRes = QWebPage::shouldInterruptJavaScript();
    qDebug() << "EWAWebPage::shouldInterruptJavaScript():" << bRes;
    return bRes;
}

void EWAWebPage::slostStatusBarVisibilityChangeRequested( bool visible )
{
    qDebug() << "EWAWebPage::slostStatusBarVisibilityChangeRequested():" << visible;
}

void EWAWebPage::slotToolBarVisibilityChangeRequested( bool visible )
{
    qDebug() << "EWAWebPage::slotToolBarVisibilityChangeRequested():" << visible;
}

void EWAWebPage::slotDownloadRequested( const QNetworkRequest& request )
{
    qDebug() << "EWAWebPage::slotDownloadRequested():";
}


void EWAWebPage::slotGeometryChangeRequested( const QRect& geom )
{
    qDebug() << "EWAWebPage::slotGeometryChangeRequested:" << geom;
}

void EWAWebPage::slotRestoreFrameStateRequested( QWebFrame *pFrame )
{
    qDebug() << "EWAWebPage::slotRestoreFrameStateRequested:" << pFrame->title();
}
