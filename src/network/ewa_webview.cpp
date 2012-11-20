/*******************************************************************************
**
** file: ewa_webview.cpp
**
** class: EWAWebView
**
** description:
** Inherits QWebKit::QWebView, customizeble view for web documents
** with internal API implementations
**
** 10.02.2009
**
** ewauthor@indatray.com
**
*******************************************************************************/

#include "ewa_webview.h"

#include "ewa_sitehandle.h"
#include "ewa_useractionsplayer.h"
#include "ewa_useractionsrecorder.h"
#include "ewa_networkaccessmanager.h"
#include "ewa_downloaddlg.h"
#include "ewa_sitewidget.h"

#include <QMenu>
#include <QActionGroup>
#include <QShortcut>
#include <QFileDialog>
#include <QDesktopServices>
#include <QWebHistory>

#include <QDebug>

#define MAX_WEBVIEW_WIDTH 65536
#define MAX_WEBVIEW_HEIGHT 65536

EWAWebView::EWAWebView( QWidget *parent, const QSize& maxSz )
:QWebView( parent )
,m_sitePtr( 0 )
,m_iMaxWidth( MAX_WEBVIEW_WIDTH )
,m_iMaxGeight( MAX_WEBVIEW_HEIGHT )
{
    m_siteWidgetPtr = 0;
    recordFinished();
    
    m_pUsrActRecorder = 0;
    m_pUsrActPlayer = 0;
    
    m_pWebPage = new EWAWebPage( this, maxSz );
    setPage( m_pWebPage );
    
    initContextMenu();
    
    m_contextMenuPnt = QPoint( -1, -1 );
    
    m_bLinkStatMsgShown = false;
    m_bInResizeEvent = false;
}

void EWAWebView::setSite( EWASiteHandle *pSite )
{
    if( pSite )
    {
        m_sitePtr = pSite;
        m_siteWidgetPtr = m_sitePtr->getWidget();
        setEWANetworkManagerPtr( m_sitePtr->getEWANetworkManagerPtr() );
        m_pWebPage->setSite( m_sitePtr );
        
        m_pDownloadDisplay = new EWADownloadDlg( m_siteWidgetPtr );
        m_pDownloadDisplay->hide();
        connect( m_siteWidgetPtr, SIGNAL( signalGeometryChanged(const QRect&) ),
            this, SLOT( updateDownloaderDlgGeometry() ) );
    }
}

void EWAWebView::setPage( QWebPage *pPage )
{
    /*
    The parent QObject of the provided page remains the owner of the object. 
    If the current document is a child of the web view, it will be deleted.
    (QAss)
    */
    if( pPage )
    {
        QWebView::setPage( pPage );//-- hope, created in the our c-tor m_pWebPage now cleared
        
        m_pWebPage = qobject_cast<EWAWebPage*>( pPage );

        connect( this, SIGNAL( loadFinished(bool) ),
            this, SLOT( slotLoadFinished(bool) ) );
        connect( this, SIGNAL( loadStarted() ),
            this, SLOT( slotLoadStarted() ) );    
        connect( pPage, SIGNAL( contentsSizeChanged(const QSize&) ),
            this, SLOT( slotAdjustSize(const QSize& ) ) );
    }
}

EWAWebView::~EWAWebView()
{
    delete m_pContextMenu;
}

bool EWAWebView::isMenuVisible() const
{
    return m_pContextMenu->isVisible();
}

void EWAWebView::setUrl( const QString& urlStr )
{
    QUrl url( urlStr );
    if( url.isValid() )
    {
        QWebView::setUrl( url );
    }
}

void EWAWebView::setHtml( const QString & qstrHtml, const QUrl& quBaseUrl,
                          bool bSilentlyOnce, QWebFrame *pFrame )
{
    if( m_pWebPage )
    {
        m_pWebPage->setSilentlyOnce( bSilentlyOnce );
    }

    if( !pFrame )
        pFrame = page()->getCurrentFrame();

   pFrame->setHtml( qstrHtml, quBaseUrl );
}

/**
** Reset content - stop downloads,
** remove content and js.
** (Like Web-browser's tab closed)
*/
void EWAWebView::clear()
{
    triggerPageAction( QWebPage::Stop, true );
    m_pWebPage->setBlank();
    m_pDownloadDisplay->hide();
}

bool EWAWebView::startRecording()
{
    recordStarted();

    m_pUsrActRecorder->reset();
    
    if( m_pWebPage )
    {
        m_pWebPage->downloadRegular();
        return true;
    }
    
    return false;
}

bool EWAWebView::startReplaying()
{
    if( m_sitePtr 
        && (!m_siteWidgetPtr->wasShown() && !m_siteWidgetPtr->isPreview())
        && m_siteWidgetPtr->isViewNormal() )
    {//-- widget must be shown for correct input event processing :
        m_siteWidgetPtr->makeInvisible( true );
        m_siteWidgetPtr->show();
    }
    
    m_bIsRecording = false;
    
    m_pUsrActPlayer->reset();

    if( m_pWebPage )
    {
        m_pWebPage->downloadPresaved();
        return true;
    }
    
    return false;
}

void EWAWebView::slotLoadStarted()
{
    m_pUsrActPlayer->sleep();
    m_pUsrActRecorder->sleep();
    
    if( m_sitePtr )
    {
        m_siteWidgetPtr->scroll2top();
        m_sitePtr->m_bDownloading = true;
    }
}

bool EWAWebView::canAndNeedResizeTo( const QSize& sz ) const
{
    return ( !m_bInResizeEvent 
        && !sz.isEmpty() 
        && sz != size() );
}
void EWAWebView::slotAdjustSize( const QSize& sz )
{
    //-- detection of "isInResizeEvenet" and minimal
    //-- delta step added to allow services like as
    //-- google maps don't cause crash
    //-- (google maps trys to locate map on the corner
    //-- of available viewport and resizes it by few pixels)
    
    const QSize currSz = size();
    const int iDeltaX = qAbs( currSz.width() - sz.width() );
    const int iDeltaY = qAbs( currSz.height() - sz.height() );
    const int iMinDelta = 1;
    if( m_bInResizeEvent
        || qMax( iDeltaX, iDeltaY ) < iMinDelta 
        || currSz == sz )
    {
        return;
    }
    
    QSize szNew = sz;
    if( szNew.isEmpty() )
    {
        szNew= page()->preferredContentsSize();
        QSize szScreen = EWAApplication::getScreenSize().size();
        szNew.setWidth( qMax( szNew.width(), szScreen.width() ) );
        szNew.setHeight( qMax( szNew.height(), szScreen.height() ) );
    }
    
    if( canAndNeedResizeTo( szNew ) )
    {
        
        resize( szNew );
    }
}
    
void EWAWebView::slotLoadFinished(bool ok)
{
    if( !m_sitePtr )
    {
        return;
    }
    
    //-- it seems !ok is received also on redirection,
    //-- so check of error code is needed:
    if( !ok )
    {
		QNetworkReply::NetworkError err = m_sitePtr->getEWANetworkManagerPtr()->lastErrorCode();
		if( QNetworkReply::OperationCanceledError != err 
			&& QNetworkReply::NoError != err )
		{
			//-- if error code is not checked
			//-- (i.e. wi here after redirect)
			//-- calling site's stop() causes
			//-- to crash somethere in QtWebKit.
			//-- Not tested with other errors.
			//-- m_sitePtr->stop();
			m_sitePtr->slotDownloaded( false );
			return;
		}
    }
    
    m_sitePtr->m_bDownloading = !m_sitePtr->isReplayFinished();
    
    
    if( m_bIsRecording )
    {
        m_pUsrActRecorder->wakeUp();
    }
    else
    {
        if( page()->mainFrame()->url().toString().compare( "about:blank" )
            && !isBlankPage()
            && !m_sitePtr->replayTypeIsHttp()
            && !history()->canGoForward() )
        {
            m_pUsrActPlayer->wakeUp();
        }
    }
}

void EWAWebView::triggerPageAction(QWebPage::WebAction action, bool checked )
{
    if( action == QWebPage::Stop )
    {
        if( m_pUsrActRecorder )
        {
            m_pUsrActRecorder->stop();
        }
        if( m_pUsrActPlayer )
        {
            m_pUsrActPlayer->stop();
        }
    }
    
    QWebView::triggerPageAction( action, checked );
}

void EWAWebView::setJSPagesActions( QVector<EWAUserActionsCollection*>* pPages )
{
    m_pUsrActPlayer->setPagesPtr( pPages );
    m_pUsrActRecorder->setPagesPtr( pPages );
}

void EWAWebView::keyPressEvent( QKeyEvent * event )
{
    if( m_bIsRecording )
    {
        m_pUsrActRecorder->rememberEvent( event );
    }
    
    QWebView::keyPressEvent( event );
}
void EWAWebView::keyReleaseEvent( QKeyEvent * event )
{
    if( m_bIsRecording )
    {
        m_pUsrActRecorder->rememberEvent( event );
    }
    
    QWebView::keyReleaseEvent( event );
}

 void EWAWebView::mouseMoveEvent( QMouseEvent *event )
{
    QPoint pnt = event->pos();
    
    if( m_siteWidgetPtr && m_siteWidgetPtr->isPreview() )
    {
        QString msg( "%1x%2" );
        emit signalStatusMessage( msg
            .arg( pnt.x() )
            .arg( pnt.y() )
            , EWASiteWidget::statusMessageDurationMs() );
    }
    bool bOpen = false;
    QString strUrl = getHyperLink( pnt, bOpen );
    if( !strUrl.isEmpty() )
    {
         if( bOpen )
         {
            strUrl.prepend( tr( "Opens in browser: " ) );
         }
         
         emit signalStatusMessage( strUrl, EWASiteWidget::statusMessageDurationMs() );
         m_bLinkStatMsgShown = true;
    }
    else if( m_bLinkStatMsgShown )
    {
        m_bLinkStatMsgShown = false;
        emit signalStatusMessage( "", 0  ); //-- XMessage shall haide statusbar
    }
    
    QWebView::mouseMoveEvent( event );
}

QString EWAWebView::getHyperLink( const QPoint& pnt, bool& bIsExternal, bool bIsImg ) const
{
    QString strRes;
    QWebFrame *pFrame = page()->frameAt( pnt );
    if( pFrame )
    {
        QWebHitTestResult hitTestRes = pFrame->hitTestContent( pnt );
        if( !hitTestRes.isNull() )
        {
            QWebElement linkElem = hitTestRes.linkElement();
            if( !linkElem.isNull() )
            {
                QUrl url = bIsImg ? 
                    EWAApplication::ensureAbsUrl( hitTestRes.imageUrl(), pFrame->baseUrl() )
                     : 
                    EWAApplication::ensureAbsUrl( hitTestRes.linkUrl(), pFrame->baseUrl() );
                    
                strRes = url.toString();
                
                if( !bIsImg )
                {
                    QString strTargetVal = linkElem.attribute( "target" );
                    
                    if( !strTargetVal.compare( "_blank", Qt::CaseInsensitive ) )
                    {
                        bIsExternal = true;
                    }
                }
            }
        }
    }
    
    return strRes;
}

bool EWAWebView::tryOpenLinkInBrowser( const QPoint& pnt ) const
{
    bool bRes = false;
    bool bOpen = false;
    
    QString strUrl = getHyperLink( pnt.isNull() ? m_contextMenuPnt : pnt, bOpen );
    
    if( !strUrl.isEmpty() )
    {
        bRes = true;
    }
    else
    {
    //-- try current frame
        strUrl = EWAApplication::ensureAbsUrl( url(), page()->getCurrentFrame()->baseUrl() ).toString();
    }
    
    if( !strUrl.isEmpty() )
    {
        QDesktopServices::openUrl( QUrl( strUrl ) );
    }
    
    return bRes;
}

bool EWAWebView::tryOpenLinkHere( const QPoint& pnt )
{
    bool bOpen = false;
    QString strUrl = getHyperLink( pnt.isNull() ? m_contextMenuPnt : pnt, bOpen );
    
    if( !strUrl.isEmpty() )
    {
        setUrl( strUrl );
        return true;
    }
    
    return false;
}

void EWAWebView::mousePressEvent ( QMouseEvent * event )
{
    if( m_bIsRecording )
    {
        m_pUsrActRecorder->rememberEvent( event );
    }
    else
    {
        switch( event->button() )
        {
            case Qt::LeftButton:
            {
                QPoint click = event->pos();
                bool bExtLink = false;
                if( event->button() == Qt::LeftButton 
                    && !getHyperLink( click, bExtLink ).isEmpty() 
                    && bExtLink )
                {
                    tryOpenLinkInBrowser( click );
                    
                    event->accept();
                    return;
                }
                
                break;
            }
            default:
            {
                m_contextMenuPnt = QPoint( -1, -1 );
                
                break;
            }
        }
    }
    
    QWebView::mousePressEvent( event );
}

void EWAWebView::mouseReleaseEvent ( QMouseEvent * event )
{
    if( m_bIsRecording )
    {
        m_pUsrActRecorder->rememberEvent( event );
    }
    
    QWebView::mouseReleaseEvent( event );
}


void EWAWebView::setEWANetworkManagerPtr( EWANetworkAccessManager *pManager )
{
    m_netAccessManagerPtr = pManager;
    if( m_netAccessManagerPtr )
    {
        m_pUsrActRecorder = m_netAccessManagerPtr->getGesturesRecorder();
        m_pUsrActPlayer = m_netAccessManagerPtr->getGesturesPlayer();
    }
}

void EWAWebView::recordStarted()
{
    m_bIsRecording = true;
}

void EWAWebView::recordFinished()
{
    m_bIsRecording = false;
}

void EWAWebView::initContextMenu()
{
    m_pContextMenu = new QMenu( 0 );
    
    initNavigationActions();
    initEditActions();
    initLinkActions();
    initImageActions();
    
    initNavigationActionsShortcuts();
    initEditActionsShortcuts();
    
    m_pContextMenu->addActions( m_pNavActionsGroup->actions() );
    m_pContextMenu->addActions( m_pEditActionsGroup->actions() );
    m_pContextMenu->addActions( m_pLinkActionsGroup->actions() );
    m_pContextMenu->addActions( m_pImageActionsGroup->actions() );
}

void EWAWebView::initNavigationActions()
{
    m_pNavActionsGroup = new QActionGroup( m_pContextMenu );
    
    QAction *pSeparator = new QAction( m_pContextMenu );
    pSeparator->setSeparator( true );
    pSeparator->setText( tr( "Navigation:" ) );
    m_pNavActionsGroup->addAction( pSeparator );
    
    QAction *pAction = pageAction( QWebPage::Back );//--	8	Navigate back in the history of navigated links.
    pAction->setData( QWebPage::Back );
    pAction->setIcon( QIcon( ":/images/back.png" ) );
    pAction->setToolTip( tr( "Go back one page" ) );
    pAction->setShortcut( QKeySequence::Back );
    m_pNavActionsGroup->addAction( pAction );
    
    pAction = pageAction( QWebPage::Forward );//--	9	Navigate forward in the history of navigated links.
    pAction->setData( QWebPage::Forward );
    pAction->setIcon( QIcon( ":/images/forward.png" ) );
    pAction->setToolTip( tr( "Go forward one page" ) );
    pAction->setShortcut( QKeySequence::Forward );
    m_pNavActionsGroup->addAction( pAction );
    
    pAction = pageAction( QWebPage::Stop );//--	10	Stop loading the current page.
    pAction->setIcon( QIcon( ":/images/stop_playing.png" ) );
    pAction->setToolTip( tr( "Abort page loading" ) );
    m_pNavActionsGroup->addAction( pAction );
    
    pAction = pageAction( QWebPage::Reload );//--	11	Reload the current page.
    pAction->setData( QWebPage::Reload );
    pAction->setIcon( QIcon( ":/images/refresh.png" ) );
    pAction->setToolTip( tr( "Reload page" ) );
    pAction->setShortcut( QKeySequence::Refresh );
    m_pNavActionsGroup->addAction( pAction );
}

void EWAWebView::initEditActions()
{
    m_pEditActionsGroup = new QActionGroup( m_pContextMenu );
    
    QAction *pSeparator = new QAction( m_pContextMenu );
    pSeparator->setSeparator( true );
    pSeparator->setText( tr( "Edit:" ) );
    m_pEditActionsGroup->addAction( pSeparator );
    
    QAction *pAction = pageAction( QWebPage::SelectAll );
    pAction->setData( QWebPage::SelectAll );
    pAction->setIcon( QIcon( ":/images/select_all.png" ) );
    pAction->setToolTip( tr( "Select all" ) );
    pAction->setShortcuts( QKeySequence::SelectAll );
    m_pEditActionsGroup->addAction( pAction );
    
    pAction = pageAction( QWebPage::Cut );
    pAction->setData( QWebPage::Cut );
    pAction->setIcon( QIcon( ":/images/cut.png" ) );
    pAction->setToolTip( tr( "Go back one page" ) );
    pAction->setShortcut( QKeySequence::Cut );
    m_pEditActionsGroup->addAction( pAction );
    
    pAction = pageAction( QWebPage::Paste );
    pAction->setData( QWebPage::Paste );
    pAction->setIcon( QIcon( ":/images/paste.png" ) );
    pAction->setToolTip( tr( "Paste from clipboard" ) );
    pAction->setShortcut( QKeySequence::Paste );
    m_pEditActionsGroup->addAction( pAction );

    pAction = pageAction( QWebPage::Copy );
    pAction->setData( QWebPage::Copy );
    pAction->setIcon( QIcon( ":/images/copy.png" ) );
    pAction->setToolTip( tr( "Copy selected" ) );
    pAction->setShortcut( QKeySequence::Copy );
    pAction->setShortcutContext( Qt::ApplicationShortcut );
    m_pEditActionsGroup->addAction( pAction );
    
    pAction = pageAction( QWebPage::Undo );
    pAction->setData( QWebPage::Undo );
    pAction->setIcon( QIcon( ":/images/undo.png" ) );
    pAction->setToolTip( tr( "Undo changes" ) );
    pAction->setShortcut( QKeySequence::Undo );
    m_pEditActionsGroup->addAction( pAction );
    
    pAction = pageAction( QWebPage::Redo );
    pAction->setData( QWebPage::Redo );
    pAction->setIcon( QIcon( ":/images/redo.png" ) );
    pAction->setToolTip( tr( "Redo changes" ) );
    pAction->setShortcut( QKeySequence( tr( "Ctrl+Y" ) ) );
    m_pEditActionsGroup->addAction( pAction );
}

void EWAWebView::initLinkActions()
{
    m_pLinkActionsGroup = new QActionGroup( m_pContextMenu );
    
    QAction *pSeparator = new QAction( m_pContextMenu );
    pSeparator->setSeparator( true );
    pSeparator->setText( tr( "Link:" ) );
    m_pLinkActionsGroup->addAction( pSeparator );
    
    m_pOpenInBrowserAction = m_pLinkActionsGroup->addAction( QIcon( ":/images/open_external.png" ), tr( "Open in Browser" ) );
    connect( m_pOpenInBrowserAction, SIGNAL( triggered() ), this, SLOT( tryOpenLinkInBrowser() ) );
    m_pOpenInBrowserAction->setToolTip( tr( "Open link in default web browser" ) );
    
    QAction *pAction = m_pLinkActionsGroup->addAction( QIcon( ":/images/open_internal.png" ), tr( "Open Here" ) );
    connect( pAction, SIGNAL( triggered() ), this, SLOT( tryOpenLinkHere() ) );
    pAction->setToolTip( tr( "Open link here" ) );
    
    pAction = pageAction( QWebPage::DownloadLinkToDisk );
    connect( pAction, SIGNAL( triggered() ), this, SLOT( slotSaveLink() ) );
    pAction->setIcon( QIcon( ":/images/save_object.png" ) );
    pAction->setText( tr( "Save object" ) );
    pAction->setToolTip( tr( "Save document to the local file" ) );
    m_pLinkActionsGroup->addAction( pAction );
    
    pAction = pageAction( QWebPage::CopyLinkToClipboard );
    pAction->setText( tr( "Copy link" ) );
    pAction->setIcon( QIcon( ":/images/copy.png" ) );
    pAction->setToolTip( tr( "Copy link location to the clipboard" ) );
    m_pLinkActionsGroup->addAction( pAction );
    
    pAction = m_pLinkActionsGroup->addAction( QIcon( ":/images/copy.png" ), tr( "Copy Links's text" ) );
    connect( pAction, SIGNAL( triggered() ), this, SLOT( slotCopyLinkText2Clipboard() ) );
    pAction->setToolTip( tr( "Copy link's text to clipboard" ) );
}

void EWAWebView::initImageActions()
{
    m_pImageActionsGroup = new QActionGroup( m_pContextMenu );
    
    QAction *pSeparator = new QAction( m_pContextMenu );
    pSeparator->setSeparator( true );
    pSeparator->setText( tr( "Image:" ) );
    m_pImageActionsGroup->addAction( pSeparator );
    
    QAction *pAction = pageAction( QWebPage::DownloadImageToDisk );
    connect( pAction, SIGNAL( triggered() ), this, SLOT( slotSaveImage() ) );
    pAction->setIcon( QIcon( ":/images/save_object.png" ) );
    pAction->setText( tr( "Save image" ) );
    pAction->setToolTip( tr( "Save image to the local file" ) );
    m_pImageActionsGroup->addAction( pAction );
    
    pAction = m_pImageActionsGroup->addAction( QIcon( ":/images/copy.png" ), tr( "Copy image's URL" ) );
    connect( pAction, SIGNAL( triggered() ), this, SLOT( tryGetImageSource() ) );
    pAction->setToolTip( tr( "Copy image's URL to the clipboard" ) );
    pAction->setIcon( QIcon( ":/images/copy.png" ) );
    
    pAction = pageAction( QWebPage::CopyImageToClipboard );
    pAction->setText( tr( "Copy image" ) );
    pAction->setIcon( QIcon( ":/images/copy.png" ) );
    pAction->setToolTip( tr( "Copy image to the clipboard" ) );
    m_pImageActionsGroup->addAction( pAction );
}

QShortcut* EWAWebView::createShrtcut( const QKeySequence& keySeq, const QWebPage::WebAction& act )
{
    QShortcut *pShortcut = 0;
    QAction *pWebAct = pageAction( act );
    if( pWebAct )
    {
        //-- will be deleted with this web view:
        pShortcut = new QShortcut( keySeq, 
            this,
            0,
            SLOT( slotWebPageActionShortcutActivatedAmbiguously() ) );
            
        connect( pShortcut, SIGNAL( activated() ),
            pWebAct, SLOT( trigger() ) );
            
        connect( pWebAct, SIGNAL( changed() ),
            this, SLOT( slotUpdateWebActShrtctState() ) );
    }
    
    return pShortcut;
}

void EWAWebView::initNavigationActionsShortcuts()
{
    m_pGoBackShct = createShrtcut( QKeySequence::Back, QWebPage::Back );
    m_pGoFwdShct = createShrtcut( QKeySequence::Forward, QWebPage::Forward );
    m_pReloadShct = createShrtcut( QKeySequence::Refresh, QWebPage::Reload );
}
                    
void EWAWebView::initEditActionsShortcuts()
{
    m_pSelectAllShct = createShrtcut( QKeySequence::SelectAll, QWebPage::SelectAll );
    m_pCutShct = createShrtcut( QKeySequence::Cut, QWebPage::Cut );
    m_pPasteShct = createShrtcut( QKeySequence::Paste, QWebPage::Paste );    
    m_pCopyShct = createShrtcut( QKeySequence::Copy, QWebPage::Copy );    
    m_pUndoShct = createShrtcut( QKeySequence::Undo, QWebPage::Undo );    
    m_pRedoShct = createShrtcut( QKeySequence::Redo, QWebPage::Redo );    
}

QShortcut* EWAWebView::getShorcutForAction( const QWebPage::WebAction& webAct ) const
{
    switch( webAct )
    {
        case QWebPage::Back:
        {
            return m_pGoBackShct;
        }
        case QWebPage::Forward:
        {
            return m_pGoFwdShct;
        }
        case QWebPage::Reload:
        {
            return m_pReloadShct;
        }
        case QWebPage::SelectAll:
        {
            return m_pSelectAllShct;
        }
        case QWebPage::Cut:
        {
            return m_pCutShct;
        }
        case QWebPage::Paste:
        {
            return m_pPasteShct;
        }
        case QWebPage::Copy:
        {
            return m_pCopyShct;
        }
        case QWebPage::Undo:
        {
            return m_pUndoShct;
        }
        case QWebPage::Redo:
        {
            return m_pRedoShct;
        }
        default:
        {
            return 0;
        }
    }
}

void EWAWebView::slotUpdateWebActShrtctState() const
{
    QAction *pWebPageAct = qobject_cast<QAction*>( sender() );
    if( pWebPageAct )
    {
        QWebPage::WebAction webAct = (QWebPage::WebAction) pWebPageAct->data().toInt();
        QShortcut *pShortcut = getShorcutForAction( webAct );
        
        if( pShortcut )
        {
            pShortcut->setEnabled( pWebPageAct->isEnabled() );
        }
    }
}


void EWAWebView::setActionGroupDisabled( QActionGroup *pGroup, bool bDisabled ) const
{
    if( pGroup )
    {
        foreach( QAction *pAction, pGroup->actions() )
        {
            pAction->setDisabled( bDisabled );
        }
    }
}

void EWAWebView::contextMenuEvent( QContextMenuEvent *event )
{
    m_contextMenuPnt = event->pos();
    //-- try current frame
    m_hitTestRes = page()->getCurrentFrame()->hitTestContent( m_contextMenuPnt );
    
    prepareContextMenu();
    
    m_pContextMenu->exec( event->globalPos() );
    event->accept();
}

void EWAWebView::prepareContextMenu()
{
    bool bLinkGroupDisabled = true;
    bool bImageGroupDisabled = true;
    
    if( !m_hitTestRes.isNull() )
    {
        bLinkGroupDisabled = m_hitTestRes.linkElement().isNull();
        bImageGroupDisabled = m_hitTestRes.pixmap().isNull();
    }
    
    m_pImageActionsGroup->setDisabled( bImageGroupDisabled );
    
    setActionGroupDisabled( m_pLinkActionsGroup, bLinkGroupDisabled );
    m_pOpenInBrowserAction->setEnabled( true );
}

void EWAWebView::resizeEvent( QResizeEvent *event )
{
    m_bInResizeEvent = true;
    
    QWebView::resizeEvent( event );
    
    
    updateDownloaderDlgGeometry();
    
    /*setUpdatesEnabled( false );
    qApp->processEvents();
    setUpdatesEnabled( true );*/
    
    m_bInResizeEvent = false;
}

void EWAWebView::slotSaveImage()
{
    emit signalStatusMessage( saveImage( m_contextMenuPnt ), EWASiteWidget::statusMessageDurationMs() );
}

QString EWAWebView::getFileNameFromUrl( const QString& strFullUrl ) const
{
    QString strPath = QUrl( strFullUrl ).path();
    QFileInfo info( strPath );
    
    QString strName = info.completeBaseName();
    if( strName.isEmpty() ) 
    {
        strName = QLatin1String( "uncknown" );
    }
    
    QString strExtension = info.suffix();
    if( strExtension.isEmpty() ) 
    {
        strExtension = QLatin1String("file");
    }

    return strName + "." + strExtension;
}

bool EWAWebView::tryGetImageSource() const
{
    QWebFrame *pFrame = page()->frameAt( m_contextMenuPnt );
    if( pFrame )
    {
        QWebHitTestResult hitTestRes = pFrame->hitTestContent( m_contextMenuPnt );
        if( !hitTestRes.isNull() )
        {
            if( !hitTestRes.pixmap().isNull() )
            {
                QString src = EWAApplication::ensureAbsUrl( hitTestRes.imageUrl(), pFrame->baseUrl() ).toString();
            
                if( !src.isEmpty() )
                {
                    EWAApplication::add2Clipboard( src );
                    return true;
                }
            }
        }
    }
    
    return false;
}

QString EWAWebView::saveImage( const QPoint& pnt ) 
{
    QWebFrame *pFrame = page()->frameAt( pnt );
    if( !pFrame )
    {
        return tr( "Invalid image coordinates." );
    }
    
    QWebHitTestResult hitTestRes = pFrame->hitTestContent( pnt );
    if( hitTestRes.isNull() )
    {
        return tr( "Can't find target DOM element." );
    }
    
    QPixmap pixmap = hitTestRes.pixmap();
    if( pixmap.isNull() )
    {
        return tr( "Can't find pixmap." );
    }
    
    QString src = EWAApplication::ensureAbsUrl( hitTestRes.imageUrl(), pFrame->baseUrl() ).toString();

    QString imgLocalFileName = getFileNameFromUrl( src );
    QString imgExt = EWAApplication::getImgFormats().join( "\n" );
	imgExt = imgExt.append( "\n*.*" );
	QString currentExt = "*.*";
    
    QString imgFileName = QFileDialog::getSaveFileName( this, tr( "%1: Save image as..." ).arg( m_siteWidgetPtr->getTitle() ),
	    imgLocalFileName, imgExt, &currentExt );
    
    if( !imgFileName.isEmpty() )
    {
        QFile imgFile( imgFileName );
        if( !imgFile.open( QIODevice::WriteOnly ) )
        {
            return tr( "Error opening file %1:\n%2" ).arg( imgFileName ).arg( imgFile.errorString() );
        }
        
        if( !pixmap.save( &imgFile ) )
        {
            imgFile.close();
            return tr( "Error saving file %1:\n%2" ).arg( imgFileName ).arg( imgFile.errorString() );
        }
        
        imgFile.close();
        
        return tr( " <b>%1</b> saved." ).arg( QFileInfo( imgFileName ).fileName() );
    }
    
    return QString( tr( "Image saving canceled." ) );
}

void EWAWebView::showDownloader( QNetworkReply *pReply )
{
    if( !pReply )
    {
        return;
    }
    
    QString strFileName = QFileDialog::getSaveFileName( this, tr( "%1: Save file as..." ).arg( m_siteWidgetPtr->getTitle() ), getFileNameFromUrl( pReply->url().toString() ) );
                                                                                        
    if( !strFileName.isEmpty() )
    {
        m_pDownloadDisplay->show();
        updateDownloaderDlgGeometry();
        m_pDownloadDisplay->init( pReply, strFileName );
    }
}

void EWAWebView::slotSaveLink()
{
    QWebFrame *pFrame = page()->frameAt( m_contextMenuPnt );
    if( pFrame )
    {
        QWebHitTestResult hitTestRes = pFrame->hitTestContent( m_contextMenuPnt );
        if( !hitTestRes.isNull() )
        {
            QUrl dstUrl = hitTestRes.linkUrl();
            if( !dstUrl.toString().isEmpty() )
            {
                QString strUa = EWAApplication::getEWAUserAgent();
                QNetworkRequest myRequest( dstUrl );
                myRequest.setRawHeader( "User-Agent", strUa.toUtf8() );
                showDownloader( page()->networkAccessManager()->get( myRequest ) );
            }
        }
    }
}

QWebElement EWAWebView::getWebElementFromPath( const QList<int>& path2Frame, const QList<int>& path2Element ) const
{
    QWebElement el;
    int iSz = path2Element.size();
    if( iSz )
    {
        QWebFrame *pFrame = getFrameFromPath( path2Frame );
        el = pFrame->documentElement();
        
            
        if( iSz == 1 && path2Element.first() == 0 )
        {
            return el;
        }
        
        for( int i = 0; i< path2Element.size(); i++ )
        {
            el = getByNumberChildOf( el, path2Element.at( i ) );
        }
    }
    
    return el;
}

QWebElement EWAWebView::getByNumberChildOf( const QWebElement& parent, const int& num ) const
{
    QWebElement res;
    if( parent.isNull() || num < 0 )
    {
        return res;
    }
    
    res = parent.firstChild();
    for( int i =0; i<num; i++ )
    {
        res = res.nextSibling();
        
        if( res == parent.lastChild() )
        {
            break;
        }
    }
    
    return res;
}

bool EWAWebView::isBlankPage() const
{
    return page()->isBlank();
}

void EWAWebView::slotCopyLinkText2Clipboard()
{
    m_hitTestRes = page()->getCurrentFrame()->hitTestContent( m_contextMenuPnt );
    //-- try current frame
    if( !m_hitTestRes.isNull() )
    {
        EWAApplication::add2Clipboard( m_hitTestRes.linkText() );
    }
}

void EWAWebView::updateDownloaderDlgGeometry()
{
    if( m_pDownloadDisplay 
        && m_pDownloadDisplay->isVisible()
        && m_sitePtr 
        && m_siteWidgetPtr 
        && !m_bInResizeEvent )
    {
        QRect msgRect = m_siteWidgetPtr->rect();
        
        QRect dlgRect = m_pDownloadDisplay->rect();
        dlgRect.moveBottomRight( msgRect.bottomRight() );
            
        m_pDownloadDisplay->setGeometry( dlgRect );
    }
}

QList<int> EWAWebView::getPathFromWebElement( const QPoint& click ) const
{
    QWebFrame *pFrame = page()->frameAt( click );
    if( pFrame )
    {
        QWebHitTestResult hitTestRes = pFrame->hitTestContent( click );
        if( !hitTestRes.isNull() )
        {
            return getPathFromWebElement( hitTestRes.element() );
        }
    }
    
    return QList<int>();
}

QList<int> EWAWebView::getPath2ElementsFrame( const QWebElement& element ) const
{
    QList<int> list;
    QWebFrame *pFrame = element.webFrame();
    QWebFrame *pMainFrame = page()->mainFrame();
    
    if( !element.isNull() 
        && pFrame 
        && pMainFrame )
    {
        QWebFrame *pParentFrame = pFrame->parentFrame();
        if( pFrame == pMainFrame
            || !pParentFrame )
        {
            list.append( 0 );
        }
        else
        {
            while( pParentFrame )
            {
                int id = getFrameIndex( pParentFrame, pFrame );
                if( id < 0 )
                    break;
                    
                list.append( id );
                
                pFrame = pParentFrame;
                pParentFrame = pParentFrame->parentFrame();
            }
        }
    }
    
    return list;
}

int EWAWebView::getFrameIndex( const QWebFrame *pParent, const QWebFrame *pFrame ) const
{
    int iRes = -1;
    
    if( !pFrame )
    {
        iRes = -1;
    }
    else if( !pParent )
    {
        iRes = 0;
    }
    else
    {
        QList<QWebFrame *> childs = pParent->childFrames();
        
        iRes = childs.lastIndexOf( (QWebFrame *)pFrame );
    }
    
    return iRes;
}

QWebFrame* EWAWebView::getFrameFromPath( const QList<int> list ) const
{
    QWebFrame *pFrame = page()->mainFrame();
    if( list.count() == 1 
        && list.at( 0 ) == 0 )
    {
        return pFrame;
    }
    
    QWebFrame *pTarget = pFrame;
    for( int i = 0; i < list.count(); ++i )
    {
        pTarget = getChildFrame( pFrame, list.at( i ) );
        
        if( pTarget )
        {
            pFrame = pTarget;
        }
        else
        {
            break;
        }
    }
    
    if( !pTarget )
        pTarget = page()->mainFrame();
    
    return pTarget;
}

QWebFrame* EWAWebView::getChildFrame( const QWebFrame *pParent, int id ) const
{
    if( pParent )
    {
        QList<QWebFrame*> childs = pParent->childFrames();
        if( id >= 0 && id < childs.count() )
        {
            return childs.at( id );
        }
    }
    
    return 0;
}

QList<int> EWAWebView::getPathFromWebElement( const QWebElement& element ) const
{
    QList<int> numbersPath;
    if( !element.isNull() )
    {
        QWebElement elParent = element.parent();
        if( elParent.isNull() )
        {
            numbersPath.prepend( 0 );
        }
        else
        {
            QWebElement root = element.document();
            QWebElement elCurrent = element;
            
            while( !elParent.isNull() )
            {
                int index = getNumberUnderParent( elCurrent );
                if( index >= 0 )
                {
                    elCurrent = elParent;
                    elParent = elCurrent.parent();
                    
                    numbersPath.prepend( index );
                }
            }
        }
    }
    
    return numbersPath;
}

int EWAWebView::getNumberUnderParent( const QWebElement& element ) const
{
    int iRes = -1;
    
    if( !element.isNull() 
        && !element.parent().isNull() 
        && !element.parent().firstChild().isNull() )
    {
        QWebElement parent = element.parent();
        QWebElement firstChild = parent.firstChild();
        
        if( firstChild == element )
            return 0;
            
        QWebElement cwe = firstChild;
        
        while( element != cwe )
        {
            iRes++;
            cwe = cwe.nextSibling();
        }
        
        iRes += 1;//-- it was -1 at start
    }
    
    return iRes;
}

void EWAWebView::slotWebPageActionShortcutActivatedAmbiguously()
{
    qWarning( "WebPage's action's shortcut's ambiguously activation processing not implemented yet." );
}


void EWAWebView::paintEvent( QPaintEvent *event )
{
    if( isVisible() && !m_siteWidgetPtr->isInvisible() )
    {
        QWebView::paintEvent( event );
    }
}


void EWAWebView::setContent(const QByteArray& data, const QString& mimeType, const QUrl& baseUrl )
{
    QWebView::setContent(data, mimeType, baseUrl );
}