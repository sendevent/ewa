/*******************************************************************************
**
** file: ewa_sitehandle.cpp
**
** class: EWASiteHandle
**
** description:
** Class to handle web sites.
**
** 09.02.2009
**
** sendevent@gmail.com
**
*******************************************************************************/

#include "ewa_sitehandle.h"
#include "ewa_sitewidget.h"
#include "ewa_networkaccessmanager.h"
#include "ewa_useractionsrecorder.h"
#include "ewa_useractionsplayer.h"

#include "ewa_webview.h"
#include "ewa_trayiconmanager.h"
#include "ewa_webpage.h"
#include "ewa_mainwindow.h"
#include "ewa_sitethumbnailfilesystem.h"
#include "ewa_sitedrawer.h"
#include "ewa_timer.h"

#include <QVBoxLayout>
#include <QPainter>
#include <QIcon>

#define CHANGES_MARKER_TAG "span"
#define CHANGES_MARKER_TAG_CLASS "EWA_changes_marker"

#include <QtGlobal>

#if defined (Q_OS_MAC)
    #include "ewa_sitewidgetmacos.h"
#elif defined (Q_OS_LINUX)
    #include "ewa_sitewidgetlinux.h"
#elif defined (Q_OS_WIN)
    #include "ewa_sitewidgetwindows.h"
#endif //-- Q_OS_MAC Q_OS_LINUX Q_OS_WIN

EWASiteHandle::EWASiteHandle( QObject *parent )
:QObject( parent )
{
    construct();
}

EWASiteHandle::EWASiteHandle( const EWASiteHandle& other )
:QObject()
{
    construct();

    this->operator =( other );
}

EWASiteHandle::EWASiteHandle( const EWASiteHandle *pOther )
:QObject()
{
    construct();

    this->operator =( pOther );
}

EWASiteHandle::~EWASiteHandle()
{
    disconnectMe();

    stop();
    
    m_sitesThumbnailsFSPtr->removeFile( m_thumbnailFileName, &m_siteTumbnailBuffer );
    
    delete m_pShowPolicy;

    if( m_pSiteWidget )
    {
        delete m_pSiteWidget;
    }
    m_pSiteWidget = 0;

    delete m_pRulesCollection;
//#ifdef EWA_FAVICONS
    if( m_pFaviconLoader->isRunning() )
    {
        m_pFaviconLoader->quit();
    }
//#endif
    
}

void EWASiteHandle::connectMe()
{
    connect( getWebPage(), SIGNAL( loadProgress(int) ),
        this, SLOT( slotTrackDownloadProgress(int) ) );
    connect( getWebPage(), SIGNAL( signalTargetLoaded(bool) ),
        this, SLOT( slotDownloaded(bool) ) );
    
    if( getWebPage() )
    {
        connect( getWebPage(), SIGNAL( signalNeedDownload(QNetworkReply*) ),
            this, SLOT( slotWantDownload(QNetworkReply*) ) );
            
        if( getWebPage()->mainFrame() )
        {
            connect( getWebPage()->mainFrame(), SIGNAL( iconChanged() ),
                this, SLOT( slotFaviconChanged() ) );
        }
    }
}

void EWASiteHandle::disconnectMe()
{
    disconnect( getWebPage(), SIGNAL( loadProgress(int) ),
        this, SLOT( slotTrackDownloadProgress(int) ) );
    disconnect( getWebPage(), SIGNAL( signalTargetLoaded(bool) ),
        this, SLOT( slotDownloaded(bool) ) );
    
    if( getWebPage() )
    {
        disconnect( getWebPage(), SIGNAL( signalNeedDownload(QNetworkReply*) ),
            this, SLOT( slotWantDownload(QNetworkReply*) ) );
            
        if( getWebPage()->mainFrame() )
        {
            disconnect( getWebPage()->mainFrame(), SIGNAL( iconChanged() ),
                this, SLOT( slotFaviconChanged() ) );
        }
    }
}

void EWASiteHandle::construct()
{
    m_favicon = QWebSettings::globalSettings()->webGraphic( QWebSettings::DefaultFrameIconGraphic );
    
    m_pShowPolicy = new EWAShowPolicy( true, QRegExp(), false, this );
    m_networkAccessManagerPtr = 0;
    
#if defined (Q_OS_MAC)
    m_pSiteWidget = new EWASiteWidgetMacos();
#elif defined (Q_OS_LINUX)
    m_pSiteWidget = new EWASiteWidgetLinux();
#elif defined (Q_OS_WIN)
    m_pSiteWidget = new EWASiteWidgetWindows();
#endif //-- Q_OS_MAC Q_OS_LINUX Q_OS_WIN
    connect( m_pSiteWidget, SIGNAL( signaSiteWidgetModified() ),
        this, SIGNAL( signalSiteModified() ) );
    
    m_thumbnailFileName = QString( "%1site_thumb_%2.png" )
    .arg( EWASiteThumbnailFileSystem::getFilesPrefix() )
    .arg( QDateTime::currentDateTime().toTime_t() + qrand() );

    m_sitesThumbnailsFSPtr = EWAApplication::siteThumbnailsFS();
    updateTumbnail();
    
    m_qstrMsgTitle = tr( "EWA [%url%]" );
    setMsgTitle( m_qstrMsgTitle );
	getWidget()->setTitle( getVisibleMsgTitle() );
    
    m_bReplayTypeIsHttp = true;
    m_pRulesCollection = new EWAParsingRuleCollection( this );
    
    setUsed( true );

    setExtendedMessages( true );

    m_pntViewScroll = QPoint( 0, 0 );

    m_bIsRecording = false;
    m_bDownloading = false;

    m_qstrUtf8BodyText = m_qstrUtf8BodyHTML = QString();
	
    m_iPeriodUnits = 15;
    m_lDownloadPeriodSec = 60 * m_iPeriodUnits;
    m_eTimeUnits = EWAApplication::TU_Minutes;

	setPeriodInMinutes( 15 );

    m_pointer.m_pSite = this;
    m_iLoadingProgress = 0;
    m_iTypingProgress = 0;

    timerPeriod = 0;
    m_lCountDownSecondsOriginal = m_lCountDownSecondsCurrent =0;

    m_bStarted = false;

    setMsgTTL( 60 );
    m_iSysTrayMsgType = 0;
    m_iMsgLengthLimit = 255;

    m_iDownloadCounter = 0;
    
    setChangesMarkerColor( Qt::yellow );
    slotFaviconChanged();
    
    m_pReqTimeoutTimer = new EWATimer( this );
    m_pReqTimeoutTimer->setInterval( 45000 );
    connect( m_pReqTimeoutTimer, SIGNAL(signalTimeOut()),
        this, SLOT( slotStopDownloadByTimeout() ) );
    m_iLastPtogress = 0;
    
    connectMe();
//#ifdef EWA_FAVICONS
    m_pFaviconLoader = new EWAFaviconLoader( this );
    connect( m_pFaviconLoader, SIGNAL( signalIconRecived(const QIcon&) ),
        this, SLOT( slotFaviconChanged(const QIcon&) ) );
    m_bLookForFavico = true;
//#endif
    
    connect( this, SIGNAL( signalSiteModified() ),
        this, SLOT( slotModifyed() ) );
        
    setFileName( EWAApplication::generateSiteFileName() );
}


EWASiteHandle& EWASiteHandle::operator=( const EWASiteHandle& other )
{
    return operator=( &other );
}

EWASiteHandle& EWASiteHandle::operator=( const EWASiteHandle *pOther )
{
    m_qstrUtf8BodyText = pOther->m_qstrUtf8BodyText;
    m_qstrUtf8BodyHTML = pOther->m_qstrUtf8BodyHTML;
    setUrl( pOther->getUrl() );
    setRulesCollection( pOther->getRulesCollection() );
    setEWANetworkManagerPtr( pOther->getEWANetworkManagerPtr() );
    getWidget()->cloneSettings( pOther->getWidget() );
    setShowPolicy( pOther->getShowPolicy() );
    int period = 0;
    unsigned int units = pOther->getPeriodAndUnits( period );
    setPeriodAndUnits( period, units );
    setExtendedMessages( pOther->extendedMessages() );
    setSysTrayMsgType( pOther->getSysTrayMsgType() );
    setMsgTitle( pOther->getMsgTitle() );
    setMsgTTL( pOther->getMsgTTL() );
    setMsgWidth( pOther->getMsgWidth() );
    setMsgHeight( pOther->getMsgHeight() );
    setMsgLocation( pOther->getMsgLocation() );
    setMsgColor( pOther->getMsgColor() );
    setSysTrayMsgType( pOther->getSysTrayMsgType() );
    setUseMsgLengthLimit( pOther->useMsgLengthLimit() );
    setMsgLengthLimit( pOther->getMsgLengthLimit() );
    setNoMsgWordsWrap( pOther->noMsgWordsWrap() );
    setMsgBaseTransparency( pOther->getMsgBaseTransparency() );
    setReplayTypeIsHttp( pOther->replayTypeIsHttp() );
    setUserAgent( pOther->getWebPage()->getUserAgent() );
    setPageScroll( pOther->getPageScroll() );
    setWebPageBaseSize( pOther->getWebPageBaseSize() );

    return *this;
}

void EWASiteHandle::setWebPageBaseSize( const QSize& sz )
{
    if( m_webPageSize != sz || getWebView()->page()->preferredContentsSize() != sz )
    {
        m_webPageSize = sz;
        getWebView()->page()->setPreferredContentsSize( m_webPageSize );
        getWebView()->resize( getWebView()->page()->mainFrame()->contentsSize() );
    }
    //-- should emit signalSiteModified();
}

QSize EWASiteHandle::getWebPageCurrentSize() const
{
    return getWebView()->page()->preferredContentsSize();
}

const QSize EWASiteHandle::getWebPageBaseSize() const
{
    return getWebView()->page()->preferredContentsSize();
}

EWASiteWidget *EWASiteHandle::getWidget() const
{
 	return m_pSiteWidget;
}

bool EWASiteHandle::isDownloadingActive() const
{
    bool bDownloadingProgress = ( getDownloadProgress() != 0 );
    bool bTypingProgress = ( getTypingProgress() != 0 );
    
    return m_bDownloading || bDownloadingProgress || bTypingProgress;
}


EWAWebView* EWASiteHandle::getWebView() const
{
    return getWidget()->getWebView();
}

bool EWASiteHandle::setPeriodAndUnits( const int& period, const unsigned int& unit )
{
    bool changed = false;
    switch( unit )
    {
        case 0:
        {
            setPeriodInSeconds( period );
            changed = true;
            break;
        }
        case 1:
        {
            setPeriodInMinutes( period );
            changed = true;
            break;
        }
        case 2:
        {
            setPeriodInHours( period );
            changed = true;
            break;
        }
    }
    
    return changed;
}

void EWASiteHandle::setPeriodInSeconds( int count )
{
    if( getPeriodInSeconds() != count )
    {
        m_eTimeUnits = EWAApplication::TU_Seconds;
        m_iPeriodUnits = 1000;
        setPeriod( count );
    }
}

void EWASiteHandle::setPeriodInMinutes( int count )
{
    if( getPeriodInSeconds()/60 != count )
    {
        m_eTimeUnits = EWAApplication::TU_Minutes;
        m_iPeriodUnits = 60000;
        setPeriod( count );
    }
}

void EWASiteHandle::setPeriodInHours( int count )
{
    if( getPeriodInSeconds()/3600 != count )
    {
        m_eTimeUnits = EWAApplication::TU_Hours;
        m_iPeriodUnits = 3600000;
        setPeriod( count );
    }
}

int EWASiteHandle::getPeriodAndUnits( int& period ) const
{
    period = m_lDownloadPeriodSec / m_iPeriodUnits;
    return m_eTimeUnits;
}

void EWASiteHandle::setPeriod( int period )
{
    long lRequestedPeriod = period * m_iPeriodUnits;
    if( m_lDownloadPeriodSec != lRequestedPeriod )
    {
        m_lDownloadPeriodSec = lRequestedPeriod;
        emit signalSiteModified();
    }
}

void EWASiteHandle::setUrl( const QString& url )
{
    bool bUpdateTitle = getMsgTitle().compare( getUrlStrLimit32() );

    m_url = QUrl( url );
    if( m_url.path().isEmpty() )
    {
        m_url.setPath( "/" );
    }

    if( bUpdateTitle )
    {
        setMsgTitle( getUrl() );
    }
}

void EWASiteHandle::slotDownload()
{
    if( !isDownloadingActive() )
    {
        m_lCountDownSecondsCurrent = 0;
        emit signalOneSecond();
        m_bDownloading = true;
        getWebView()->startReplaying();
    }
}

void EWASiteHandle::lookForFavicon()
{
    QString strIconUrl;
    
    QWebElementCollection headerLinks = getWebView()->page()->mainFrame()->findAllElements("head link");
    if( headerLinks.count() )
    {
        foreach( QWebElement link, headerLinks )
        {
            QString strRel = link.attribute( "rel" );
            if( !strRel.compare( "SHORTCUT ICON", Qt::CaseInsensitive ) )
            {
                QUrl faviconUrl = EWAApplication::ensureAbsUrl( QUrl( link.attribute( "href" ) ), 
                    getWebView()->page()->mainFrame()->baseUrl() );
                
                strIconUrl = faviconUrl.toString();
                break;
            }
        }
    }
    
    if( strIconUrl.isEmpty() )
    {
        QUrl icoUrl( getUrl() );
        strIconUrl = QString( icoUrl.scheme() +"://"+ icoUrl.host() +"/favicon.ico");
    }
    
    m_pFaviconLoader->setTargetUrl( strIconUrl );
    m_pFaviconLoader->start();
}

void EWASiteHandle::slotDownloaded( bool ok )
{
    //-- stop the abort req timer:
    if( m_pReqTimeoutTimer->isActive() )
    {
        m_pReqTimeoutTimer->stop();
    }
    m_iLoadingProgress = m_iTypingProgress = 0;
    
    if( m_bLookForFavico && ok )
    {
        m_bLookForFavico = false;
        lookForFavicon();
    }

    m_bDownloading = false;
    m_iDownloadCounter++;
    
    QString html = getWebPage()->mainFrame()->toHtml();
    
    if( !ok 
        && 
        (html.isEmpty() || !html.compare( "<html><head></head><body></body></html>" ) ) )
    {
        getWebPage()->setErrorMessage( getUrl(), m_networkAccessManagerPtr->lastErrorString() );
    
        runExtRoutine();
        getWidget()->scroll2top();
        
        emit signalSiteLoadProgress( m_iLoadingProgress );
        emit signalDownloaded( &( this->m_pointer ) );
        
        return;
    }
    
    //-- restore customized by user web page's size:
    //-- (can changed during executing users actions)
    setWebPageBaseSize( m_webPageSize );
    
    if( m_pRulesCollection->getRulesCount()
         && m_pRulesCollection->parseSourceString( html ) )
    {

        getWebView()->setHtml( html, QUrl( getUrl() ), true );
    }

    m_qstrUtf8BodyText = getBodyAsPlainText();
    m_qstrUtf8BodyHTML = getBodyAsHtml();

    if( !isRecording() )
    {
        processServerAnswer( html );
    }
}

void EWASiteHandle::processServerAnswer( const QString& html )
{
    if( !replayTypeIsHttp() && !isReplayFinished() )
    {
        return;
    }
    
    if( html.isEmpty() || !html.compare( "<html><head></head><body></body></html>" ) )
    {
        getWebView()->getWebPage()->setBlank();
    }
    else
    {
        //-- if the downloading occured by action triggering -
        //-- show the our widget in any case
        if( !m_bForceDownload )
        {
            QString strCode = getWatchedElementSource();
            if( strCode.isEmpty() )
            {
                strCode = html;
            }
            
            bool bChanged = trackChanges( strCode );
            if( !bChanged && !getWidget()->isPreview() && !getWidget()->wasShown() )
            {//-- break next downloads and restart timings
                stop();
                getWidget()->getWebView()->clear();
                start();
                return;
            }
        }
        
        m_bForceDownload = false;
        
        //QWebElement el = getWatchedElement();
    
        markupMonitoredElement();
    }
    
    runExtRoutine();
    
    emit signalSiteLoadProgress( m_iLoadingProgress );
    emit signalDownloaded( &( this->m_pointer ) );
    
}

void EWASiteHandle::runExtRoutine()
{
    
    
    if( EWAApplication::getMainWindow()->needSaveMessage() )
    {
        QFile log( EWAApplication::getMainWindow()->getLogFileName() );
        if( log.open( QIODevice::Append ) )
        {
            QString logMsg = tr( "%1 recived:\n<EWA_MESSAGE>\n" ).arg( QDateTime::currentDateTime().toString( "dd-MM-yyyy HH:mm:ss" ) );
            logMsg.append( getBodyAsHtml() );
            logMsg.append( "\n</EWA_MESSAGE>\n\n" );
            log.write( logMsg.toLocal8Bit(), logMsg.toLocal8Bit().size() );
            log.close();
        }
    }

    if( extendedMessages() )
    {
        getWidget()->setTitle( getVisibleMsgTitle() );
        /*if( getWidget()->isViewNormal() )
            getWidget()->slotScrollTo( m_pntViewScroll );*/
        
        //if( !getWidget()->wasShown() )
        {
            getWidget()->showSexy();
        }
    }
    else
    {
        QString msg = getBodyAsPlainText();
        if( !msg.isEmpty() )
        {
            EWAApplication::getTrayIconManager()->setMessage( getVisibleMsgTitle(), msg, getMsgLengthLimit(),
            noMsgWordsWrap(), useMsgLengthLimit(), getSysTrayMsgType(), getMsgTTL() );
        }
    }
    
    updateTumbnail();
}

void EWASiteHandle::updateTumbnail()
{
    QPixmap tmpImg;
    if( !getWebView()
        || getWebView()->isBlankPage()
        || !getWidget()->getWebViewTumbnail( tmpImg ) )
    {
        tmpImg = QPixmap( ":/images/ewa.png" );
    }
    
    if( !tmpImg.isNull() )
    {
        EWASiteDrawer::makeThumbnailFromImage( tmpImg );
        QBuffer tmpBuff( &m_siteTumbnailBuffer );
        if( tmpBuff.open( QIODevice::WriteOnly ) )
        {
            if( tmpImg.save( &tmpBuff, "PNG" ) )
            {
                m_sitesThumbnailsFSPtr->addFile( m_thumbnailFileName, &m_siteTumbnailBuffer );
            }
        }
    }
}


/**
** Check const QString& html for changes from prev download.
** Return true if:
** ShowPolicy is unused;
** ShowPolicy is set to "Show if Changed" and text changed;
** ShowPolicy is set to "Show if NOT Changed" and text NOT changed.
*/
bool EWASiteHandle::trackChanges( const QString& html )
{
    bool res = true;
    if( html.isEmpty() || !m_pShowPolicy->isUsed()  )
    {
        return true;
    }
    
    if( getWidget()->getWebView()->isBlankPage() )
    {
        qWarning( "EWASiteHandle::trackChanges: it's blank." );
        return false;
    }

    QString currentMatched = m_pShowPolicy->lookInHtml( html );
    
    bool bChanged = ( bool )m_qstrPrevMatched.compare( currentMatched, Qt::CaseInsensitive );

    res = m_pShowPolicy->showIfChanged() == bChanged;

    QString msg = tr( "\n[%1] Changes Monitor:\n" );;
    EWAApplication::instance()->displayInfoMessage( msg.arg(getVisibleMsgTitle()) );
    
    msg = tr( "<old match>:\n%1\n</old match>\n" );
    EWAApplication::instance()->displayInfoMessage( msg.arg(m_qstrPrevMatched ) );
    
    msg = tr( "<current match>:\n%1\n</current match>" );
    EWAApplication::instance()->displayInfoMessage( msg.arg(currentMatched ) );
    
    msg = tr( "\nchanged - %1" );
    EWAApplication::instance()->displayInfoMessage( msg.arg(bChanged ) );
    
    msg = tr( "\nres - %5" );
    EWAApplication::instance()->displayInfoMessage( msg.arg(res) );
    
    m_qstrPrevMatched = currentMatched;
    
    return res;
}

void EWASiteHandle::markupMonitoredElement()
{
    if( !getShowPolicy()->isUsed() 
        || getWebView()->isBlankPage()
        || isDownloadingActive() 
        || !isReplayFinished() )
    {
        return;
    }
    
    QWebElement el = getWatchedElement();
    if( !el.isNull() )
        el.setStyleProperty( "background-color", getChangesMarkerColor().name() );
        
    //qDebug() << el.styleProperty( "background-color", QWebElement::ComputedStyle );
}

void EWASiteHandle::setChangesMarkerColor( const QColor& color )
{
    if( m_changesMarkupColor != color )
    {
        color.isValid() ? m_changesMarkupColor = color : m_changesMarkupColor = Qt::yellow;
        
        emit signalSiteModified();
    }
                                  
    markupMonitoredElement();
}
    
const QString EWASiteHandle::getUrl() const
{
    return m_url.toString();
}

const QString EWASiteHandle::getUrlStrLimit32() const
{
    return prepareDescription( getUrl() );
}

QString EWASiteHandle::getBodyAsHtml()  const
{
    QString hyperText = m_qstrUtf8BodyHTML;
    EWAWebView *tmpViewPtr = getWebView();
    if( tmpViewPtr )
    {
        QWebPage *tmpWebPagePtr = tmpViewPtr->page();
        if( tmpWebPagePtr )
        {
            QWebFrame *tmpWebFrame = tmpWebPagePtr->mainFrame();
            if( tmpWebFrame )
            {
                hyperText = tmpWebFrame->toHtml();
            }
        }
    }

    return hyperText;
}

QString EWASiteHandle::getBodyAsPlainText() const
{
    QString plainText = m_qstrUtf8BodyText;
    EWAWebView *tmpViewPtr = getWebView();
    if( tmpViewPtr )
    {
        QWebPage *tmpWebPagePtr = tmpViewPtr->page();
        if( tmpWebPagePtr )
        {
            QWebFrame *tmpWebFrame = tmpWebPagePtr->mainFrame();
            if( tmpWebFrame )
            {
                plainText = tmpWebFrame->toPlainText();
            }
        }
    }

    return plainText;
}

void EWASiteHandle::slotFaviconChanged( const QIcon& icon )
{
    if( !icon.isNull() )
    {
        m_favicon = icon;
    }
    
    if( m_favicon.isNull() )
    {
        m_favicon = QWebSettings::webGraphic( QWebSettings::DefaultFrameIconGraphic );
    }
    
    getWidget()->setWindowIcon( m_favicon );

    emit signalFaviconChanged();
}

QPixmap EWASiteHandle::getFavicoPixmap() const
{
    return m_favicon.isNull() ? QPixmap( ":/images/ewa.png" ).scaled( 16, 16 ) : m_favicon.pixmap( 16, 16 );
}

void EWASiteHandle::slotTrackDownloadProgress( int value )
{
    if( m_pReqTimeoutTimer->isActive() )
    {
        m_pReqTimeoutTimer->stop();
    }
    
    if( value != 100 )
    {
        m_iLoadingProgress = value;
        m_pReqTimeoutTimer->start();
    }
    else
    {
        m_iLoadingProgress = 0;
    }
    
    m_iTypingProgress = 0;
    
    emit signalSiteLoadProgress( m_iLoadingProgress );
}

void EWASiteHandle::oneSecondTicked()
{
    if( m_bStarted && !isDownloadingActive() && getWidget()->isViewNormal() )
    {
        if( m_lCountDownSecondsCurrent )
        {
            m_lCountDownSecondsCurrent--;
            emit signalOneSecond();
        }
        else
        {
            slotDownload();
            resetCountdown();
        }
    }
}

void EWASiteHandle::start()
{
    stop();

    long idleTime = getPeriodInSeconds();
    m_lCountDownSecondsOriginal = idleTime;
    resetCountdown();
    m_bStarted = true;

}
void EWASiteHandle::stop()
{
    getWebView()->triggerPageAction( QWebPage::Stop, true );
    if( m_networkAccessManagerPtr )
    {
        m_networkAccessManagerPtr->clearFakedRequests();
    }
    m_lCountDownSecondsCurrent = m_iLoadingProgress = m_iTypingProgress = 0;
    m_bStarted = m_bDownloading = m_bForceDownload = false;

    emit signalOneSecond();
    emit signalSiteLoadProgress( m_iLoadingProgress );
}

void EWASiteHandle::stopAndClear()
{
    disconnectMe();
    stop();
    getWebView()->clear();
    connectMe();
}


void EWASiteHandle::setPageScroll( const QPoint& pt )
{
    m_pntViewScroll = pt;

    getWidget()->setBaseScrollPoint( m_pntViewScroll );
    
    //-- should emit signalSiteModified();
}

void EWASiteHandle::stopEx()
{
    if( getWidget() && getWidget()->wasShown() )
    {
        stop();
    }
    else
    {
        stopAndClear();
    }
}

void EWASiteHandle::slotActionDelegatActivated()
{
    
    if( isDownloadingActive() )
    {
        stopEx();
    }
    else
    {
        m_bForceDownload = true;
        slotDownload();
    }
}

QPoint EWASiteHandle::getPageScroll() const 
{
    return getWidget()->getCurrnetScrollPoint();
}

void EWASiteHandle::setHtmlWithoutDownload( const QString &html, const QString& baseUrl )
{
    QString url = baseUrl;
    if( url.isEmpty() )
        url = getUrl();
    else
        setUrl( url );

    m_qstrUtf8BodyHTML = html;

    getWebView()->setHtml( m_qstrUtf8BodyHTML, url, true );
}

QString EWASiteHandle::prepareDescription( const QString& descr ) const
{
    int iLength = 64;
    QString strSuffix( "..." );
    QString res;
    if( !descr.isEmpty() )
    {
        res = descr;
        if( res.length() > iLength )
        {
            res = res.left( iLength-strSuffix.length() ) + strSuffix;
        }

        res = res.left( iLength );
    }

    return res;
}

QString EWASiteHandle::secondsToHMS( long sec ) const
{
    int h = sec/3600;
    int m = ( sec - 3600*h )/60;
    int s = sec - h*3600 - m*60;

    QString res;
    QString prefix;
    if( h>0 )
    {
        if( h<10 )
        {
            prefix = "0";
        }
        else
        {
            prefix = "";
        }
        res.append( tr( "%1%2:" ).arg( prefix ).arg( h ) );
    }
    else
    {
        res.append( "00:" );
    }
    if( m>0 )
    {
        if( m<10 )
        {
            prefix = "0";
        }
        else
        {
            prefix = "";
        }
        res.append( tr( "%1%2:" ).arg( prefix ).arg( m ) );
    }
    else
    {
        res.append( "00:" );
    }
    if( s>0 )
    {
        if( s<10 )
        {
            prefix = "0";
        }
        else
        {
            prefix = "";
        }
        res.append( tr( "%1%2" ).arg( prefix ).arg( s ) );
    }
    else
    {
        res.append( "00" );
    }

    return res;
}

void EWASiteHandle::setUsed( bool on )
{
    if( m_bUsed != on )
    {
        m_bUsed = on;
        if( !isUsed() )
        {
            long lCountDownSecondsCurrent = getCountdownCurrent();
            bool bStarted = isStarted();
            stop();
            m_lCountDownSecondsCurrent = lCountDownSecondsCurrent;
            m_bStarted = bStarted;
        }
        emit signalUsageChanged();
        emit signalSiteModified();
    }
}


QString EWASiteHandle::getVisibleMsgTitle() const
{
    QString msgTitle = getMsgTitle();
    msgTitle = msgTitle.replace( "%url%", getUrlStrLimit32(), Qt::CaseInsensitive );
    return prepareDescription( msgTitle );
}

void EWASiteHandle::closeWidgetByHideViewMode( int mode )
{
    if( getWidget()->isVisible() || getWidget()->wasShown() )
    {
        if( mode < 0 )
        {
            getWidget()->close();
        }
        else
        {
            getWidget()->closeByHideViewMode( (EWASiteWidget::EWA_EM_HVM)mode );
        }
    }
}

int EWASiteHandle::getMsgBaseTransparency() const 
{
    int opacityValue2Percents = 100 - getWidget()->getBaseOpacity()*100.;
    
    return opacityValue2Percents;
}

void EWASiteHandle::setMsgBaseTransparency( const int to )
{
    double opacityPercents2value = 1. - 0.01*to; 
    
    getWidget()->setBaseOpacity( opacityPercents2value );
    
    //-- should emit signalSiteModified();
}

bool EWASiteHandle::isReplayFinished() const 
{
    if( replayTypeIsHttp() )
    {
        return true;
    }
    bool bGestured = getEWANetworkManagerPtr()->getGesturesPlayer()->isReplayed();
    bool bFaked = !getEWANetworkManagerPtr()->hasFakedRequests();
    return bGestured && bFaked;
}

void EWASiteHandle::load( QSettings *pSettings )
{
    if( !pSettings )
    {
        return;
    }
    
    setUrl( pSettings->value( QLatin1String( "site" ) ).toString() );
    setWebPageBaseSize( pSettings->value( QLatin1String( "viewSize" ), QSize( 640, 480 ) ).toSize() );
    setPageScroll( pSettings->value( QLatin1String( "viewScroll" ), QPoint( 0, 0 ) ).toPoint() );

    QString nativeUA = EWAApplication::getEWAUserAgent();
    QString ua = pSettings->value( QLatin1String( "U-A" ), nativeUA ).toString();
    if( ua.contains( "Enhanced Web Assistant" ) 
        || ua.contains( "EnhancedWebAssistant" ) 
        || ua.contains( "EWA" ) )
    {
        ua = nativeUA;
    }
    

    setUserAgent( ua );
    this->setUsed( pSettings->value( QLatin1String( "used" ), true ).toBool() );

    int count;
    unsigned int units;
    count = pSettings->value( QLatin1String( "periodValue" ), 60 ).toInt();
    units = pSettings->value( QLatin1String( "periodUnits" ), 0 ).toUInt();
    this->setPeriodAndUnits( count, units );
    
    this->setReplayTypeIsHttp( pSettings->value( QLatin1String( "replayTypeHttp" ), true ).toBool() );
    
    pSettings->beginGroup( QLatin1String( "message" ) );
        this->setExtendedMessages( EWAApplication::getTrayIconManager()->supportBaloons()
            ? pSettings->value( QLatin1String( "enhanced" ), true ).toBool()
            : true );
/*#ifdef Q_OS_LINUX
        //-- WM (at last, KDE's - KWM) sometime don't place widgets behind desktop's borders,
        //-- thoug all geometry (frame's pos, width/height) seems to be right...
        //-- so, this way works for KDE4:
        if( extendedMessages() )
        {
            getWidget()->resize( QSize( 111, 111 ) );
            setMsgLocation( EWAApplication::getMainWindow()->geometry().center() );
            getWidget()->loadCurrentMode( EWASiteWidget::EM_HVM_ALWAYS_ON_DESKTOP );
            qApp->processEvents();
            getWidget()->show();
        }
#endif //-- Q_OS_LINUX*/
        this->setMsgTitle( pSettings->value( QLatin1String( "title" ), tr( "EWA [%url%]") ).toString() );
        this->setMsgTTL( pSettings->value( QLatin1String( "ttl" ), 15 ).toInt() );

        getWidget()->setNewSize( pSettings->value( QLatin1String( "w" ), 275 ).toInt(), pSettings->value( QLatin1String( "h" ), 175 ).toInt() );
        this->setMsgLocation( pSettings->value( QLatin1String( "location" ) ).toPoint() );
        
        loadFaviconFromData( pSettings->value( QLatin1String( "favicon" ), QByteArray() ).toByteArray() );
        
        int r = pSettings->value( QLatin1String( "colorR" ), 123 ).toInt();
        int g = pSettings->value( QLatin1String( "colorG" ), 123 ).toInt();
        int b = pSettings->value( QLatin1String( "colorB" ), 123 ).toInt();
        this->setMsgColor( QColor( r, g, b ) );

        this->setMsgBaseTransparency( pSettings->value( QLatin1String( "opacity" ), 1. ).toDouble() );

        this->getWidget()->setBackButtonShown( pSettings->value( QLatin1String( "backShown" ), false ).toBool() );
        this->getWidget()->setRefreshButtonShown( pSettings->value( QLatin1String( "refreshShown" ), true ).toBool() );
        this->getWidget()->setForwardButtonShown( pSettings->value( QLatin1String( "forwardShown" ), false ).toBool() );
        this->getWidget()->setTitleLabelShown( pSettings->value( QLatin1String( "titleShown" ), false ).toBool() );
        this->getWidget()->setStickButtonShown( pSettings->value( QLatin1String( "stickShown" ), false ).toBool() );
        this->getWidget()->setViewModeButtonShown( pSettings->value( QLatin1String( "viewmodeShown" ), false ).toBool() );
        this->getWidget()->setCloseButtonShown( pSettings->value( QLatin1String( "closeShown" ), false ).toBool() );

        this->getWidget()->loadCurrentMode( (EWASiteWidget::EWA_EM_HVM)pSettings->value( QLatin1String( "z-index-mode" ), 0 ).toInt() );
        
        this->setSysTrayMsgType( pSettings->value( QLatin1String( "type" ), 0 ).toInt() );
        this->setUseMsgLengthLimit( pSettings->value( QLatin1String( "lengthLimit" ), true ).toBool() );
        this->setMsgLengthLimit( pSettings->value( QLatin1String( "length" ), 255 ).toInt() );
        this->setNoMsgWordsWrap( pSettings->value( QLatin1String( "noWordWrap" ), true ).toBool() );
        
        
    pSettings->endGroup();
    
    pSettings->beginGroup( QLatin1String( "showPolicy" ) );
        EWAShowPolicy *pSiteShowPolicy = this->getShowPolicy();
        pSiteShowPolicy->load( pSettings );
        m_qstrPrevMatched = pSettings->value( QLatin1String( "TrackChanges" ), "" ).toString();
        setChangesMarkerColor( QColor( pSettings->value( QLatin1String( "ChangesMarker" ), "#ffff00" ).toString() ) );
    pSettings->endGroup();
    
    m_pRulesCollection->load( pSettings );
    m_networkAccessManagerPtr->load( pSettings );
    
/*#ifdef Q_OS_LINUX
        //-- WM (at last, KDE's - KWM) sometime don't place widgets behind desktop's borders,
        //-- thoug all geometry (frame's pos, width/height) seems to be right...
        //-- so, this way works for KDE4:
        if( extendedMessages() )
        {
            getWidget()->close();
        }
#endif //-- Q_OS_LINUX*/
    m_bChanged = false;
}

void EWASiteHandle::clearIniFile( QSettings *pSetts ) const
{
    QString steFilePath = pSetts ? pSetts->fileName() : m_strFileName;
    
    if( !steFilePath.isEmpty()  && QFileInfo( steFilePath ).exists() )
    {
        QFile::remove( steFilePath );
    }
}

bool EWASiteHandle::saveFavicon2data( QByteArray& ba  ) const
{
    QPixmap pixmap = getFavicoPixmap();
    QBuffer buffer( &ba );
    if( !buffer.open( QIODevice::WriteOnly )
        || !pixmap.save( &buffer, "PNG" ) )
            return false;
    
    return !ba.isEmpty();
}
void EWASiteHandle::loadFaviconFromData( const QByteArray& data )
{
    slotFaviconChanged( QIcon( QPixmap::fromImage( QImage::fromData( data, "PNG" ) ) ) );
}

void EWASiteHandle::save( QSettings *pSettings, bool bForce )
{
    if( !pSettings || ( !isModifyed() && !bForce ) )
    {
        return;
    }
    
    clearIniFile( pSettings );
    
    pSettings->setValue( QLatin1String( "site" ), getUrl() );
    pSettings->setValue( QLatin1String( "viewSize" ), getWebPageBaseSize() );
    pSettings->setValue( QLatin1String( "viewScroll" ), getPageScroll() );

    pSettings->setValue( QLatin1String( "U-A" ), getWebPage()->getUserAgent() );
    pSettings->setValue( QLatin1String( "used" ), isUsed() );

    int count;
    unsigned int units = getPeriodAndUnits( count );
    pSettings->setValue( QLatin1String( "periodValue" ), count );
    pSettings->setValue( QLatin1String( "periodUnits" ), units );
    
    pSettings->setValue( QLatin1String( "replayTypeHttp" ), replayTypeIsHttp() );
    
    pSettings->beginGroup( QLatin1String( "message" ) );
        pSettings->setValue( QLatin1String( "enhanced" ), extendedMessages() );
        pSettings->setValue( QLatin1String( "title" ), getMsgTitle() );
        pSettings->setValue( QLatin1String( "ttl" ), getMsgTTL() );

        pSettings->setValue( QLatin1String( "w" ), getMsgWidth() );
        pSettings->setValue( QLatin1String( "h" ), getMsgHeight() );
        pSettings->setValue( QLatin1String( "location" ), getMsgLocation() );
        
        QByteArray ba;
        if( !saveFavicon2data( ba ) )
            qWarning() << getMsgTitle() << "favicon not saved!";
            
        pSettings->setValue( QLatin1String( "favicon" ), ba );
        
        pSettings->setValue( QLatin1String( "colorR" ), getMsgColor().red() );
        pSettings->setValue( QLatin1String( "colorG" ), getMsgColor().green() );
        pSettings->setValue( QLatin1String( "colorB" ), getMsgColor().blue() );

        pSettings->setValue( QLatin1String( "opacity" ), getMsgBaseTransparency() );

        pSettings->setValue( QLatin1String( "backShown" ), getWidget()->isBackButtonShown() );
        pSettings->setValue( QLatin1String( "refreshShown" ), getWidget()->isRefreshButtonShown() );
        pSettings->setValue( QLatin1String( "forwardShown" ), getWidget()->isForwardButtonShown() );
        pSettings->setValue( QLatin1String( "titleShown" ), getWidget()->isTitleLabelShown() );
        pSettings->setValue( QLatin1String( "stickShown" ), getWidget()->isStickButtonShown() );
        pSettings->setValue( QLatin1String( "viewmodeShown" ), getWidget()->isViewModeButtonShown() );
        pSettings->setValue( QLatin1String( "closeShown" ), getWidget()->isCloseButtonShown() );
        pSettings->setValue( QLatin1String( "z-index-mode" ), getWidget()->getCurrentMode() );

        pSettings->setValue( QLatin1String( "type" ), getSysTrayMsgType() );
        pSettings->setValue( QLatin1String( "lengthLimit" ), useMsgLengthLimit() );
        pSettings->setValue( QLatin1String( "length" ), getMsgLengthLimit() );
        pSettings->setValue( QLatin1String( "noWordWrap" ), noMsgWordsWrap() );
    pSettings->endGroup();
    
    pSettings->beginGroup( QLatin1String( "showPolicy" ) );
        EWAShowPolicy *pSiteShowPolicy = this->getShowPolicy();
        pSiteShowPolicy->save( pSettings );
        pSettings->setValue( QLatin1String( "TrackChanges" ), m_qstrPrevMatched );
        pSettings->setValue( QLatin1String( "ChangesMarker" ), getChangesMarkerColor().name() );
    pSettings->endGroup();
    
    m_pRulesCollection->save( pSettings );
    m_networkAccessManagerPtr->save( pSettings );
    
    m_bChanged = false;
}

QString EWASiteHandle::getFileName() 
{
    if( m_strFileName.isEmpty() )
        setFileName( EWAApplication::generateSiteFileName() );
    
    int strLength = QString( EWAApplication::getSitesFolder() ).length();
    
    return m_strFileName.right( m_strFileName.length() - strLength );
}
void EWASiteHandle::setFileName( const QString& fn )
{
    if( !fn.isEmpty() )
        m_strFileName = EWAApplication::getSitesFolder() + fn;
}

void EWASiteHandle::load( const QString& fileName )
{
    setFileName( fileName );
    
    QSettings settings( m_strFileName, QSettings::IniFormat );
    
    load( &settings );
}
void EWASiteHandle::save( const QString& fileName, bool bForce )
{
    if( !fileName.isEmpty() )
    {
        setFileName( fileName );
    }
    
    QSettings settings( m_strFileName, QSettings::IniFormat );
    
    if( isModifyed() || bForce )
        save( &settings, bForce );
}

void EWASiteHandle::setEWANetworkManagerPtr( EWANetworkAccessManager *pAccessManager )
{
    m_networkAccessManagerPtr = pAccessManager;
    if( m_networkAccessManagerPtr )
    {
        connect( m_networkAccessManagerPtr->getGesturesPlayer(), SIGNAL( signalUserActivitiesReplayProgress(int) ),
            this, SLOT( slotUserActivitiesReplayProgress(int) ) );
        connect( m_networkAccessManagerPtr->getGesturesPlayer(), SIGNAL( signalUserActivitiesReplayFinished() ),
            this, SLOT( slotUserActivitiesReplayed() ) );
        connect( m_networkAccessManagerPtr->getGesturesPlayer(), SIGNAL( signalUserActivitiesReplayStarted() ),
            this, SIGNAL( signalUserTypingStarted() ) );
            
        getWidget()->setSite( this );
    }
}

void EWASiteHandle::setTypingProgress( int progress )
{
    m_iTypingProgress = progress;
    if( m_iTypingProgress )
        m_iLoadingProgress = 0;
    
    emit signalSiteTypingProgress( m_iTypingProgress );
}





void EWASiteHandle::setMsgTitle( const QString& title )
{
    if( m_qstrMsgTitle.compare( title ) )
    {
        m_qstrMsgTitle = title;
        getWidget()->setTitle( getVisibleMsgTitle() ); 
    }
}

void EWASiteHandle::slotWantDownload( QNetworkReply *pReply )
{
    if( pReply && !pReply->url().isEmpty() )
    {
        getWidget()->showDownloader( pReply );
    }
}

void EWASiteHandle::slotUserActivitiesReplayProgress( int percent )
{
    setTypingProgress( percent );
}

void EWASiteHandle::slotUserActivitiesReplayed()
{
    emit signalUserTypingFinished();
    if( isReplayFinished() )
        slotDownloaded();
}

QWebElement EWASiteHandle::getWatchedElement() const
{
    QWebElement el;
    
    if( m_pShowPolicy->isUsed() 
        && isReplayFinished()
        && !isDownloadingActive()
        && getWidget()
        && getWidget()->getWebView()
        && !getWebView()->isBlankPage() )
    {
        el = getWebView()->getWebElementFromPath( m_pShowPolicy->getPath2ElementsFrame(), 
            m_pShowPolicy->getPath2Element() );
    }
    
    return el;
}

QString EWASiteHandle::getWatchedElementSource() const
{
    if( m_pShowPolicy->isUsed() )
    {
        QWebElement el = getWatchedElement();
        if( !el.isNull() )
        {
            return el.toOuterXml();
        }
    }
    
    return QString();
}

void EWASiteHandle::setExtendedMessages( bool on ) 
{ 
    if( EWAApplication::getTrayIconManager()->supportBaloons() )
    {
        if( m_bExtendedMsg != on )
        {
            m_bExtendedMsg = on;
            emit signalSiteModified();
        }
    }
    else
    {
        m_bExtendedMsg = true;
    }
}

void EWASiteHandle::slotStopDownloadByTimeout()
{
    getWebView()->stop();
    m_pReqTimeoutTimer->stop();
    
    qWarning( "EWASiteHandle::slotStopDownloadByTimeout" );
}

void EWASiteHandle::resetCountdown()
{
    m_lCountDownSecondsCurrent = m_lCountDownSecondsOriginal;
}

void EWASiteHandle::slotModifyed()
{
    m_bChanged = true;
}

void EWASiteHandle::setUserAgent( const QString& strUA ) 
{
    getWebPage()->setUserAgent( strUA );
    m_pFaviconLoader->setUserAgent( strUA );
}