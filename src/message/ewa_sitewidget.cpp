/*******************************************************************************
**
** file: ewa_sitewidget.cpp
**
** class: EWASiteWidget
**
** description:
** Internal popup message - support sliding, customizible tittle, auto hide,
** "always on top/bottom" modes.
** Custom "close" - through nice transparency :-[
** Create/destroy in viewmanager.
** Contains WebView, used in sitehandle and other.
**
** 23.03.2009
**
** ewauthor@indatray.com
**
*******************************************************************************/

#include "ewa_sitewidget.h"
#include "ewa_sitewidgettitlewidget.h"
#include "ewa_timer.h"
#include "ewa_webview.h"
#include "ewa_siteslistmanager.h"
#include "ewa_sitehandle.h"
#include "ewa_networkaccessmanager.h"
#include "ewa_textlabel.h"


#include <QTimer>
#include <QTime>
#include <QtGui>

#define STATUS_MESSAGE_DURATION_MSEC 3000

/*static*/ int EWASiteWidget::statusMessageDurationMs()
{
    return STATUS_MESSAGE_DURATION_MSEC;
}
                                   
#define OPACITY_REMISSION_DURATION_SECONDS 2
#define OPACITY_REMISSION_INTERVAL_MSECONDS 50

#define OPACITY_MAX 0.05

#define SIZEGRIP_SZ 8

#include <QTime>
EWASiteWidget::EWASiteWidget( QWidget *parent )
:QWidget( parent )
,m_sitePtr(0)
,m_bImPreview( false )
{
    makeInvisible( true );
    construct();

    QStyle *pMyStyle = new QPlastiqueStyle();
    if( pMyStyle )
    {
        setStyle( pMyStyle );
        QList<QWidget *> widgets = qFindChildren<QWidget *>(this);
        foreach (QWidget *w, widgets)
            w->setStyle( pMyStyle );
    }
}

void EWASiteWidget::construct()
{
#ifdef PLAY_WITH_FLAGS
    pFlagsDlg = new FlagsManagerLialog();
#endif //-- PLAY_WITH_FLAGS
    
    eCurrentHideVewMode = EM_HVM_AUTOHIDE;

    setAttribute(Qt::WA_TranslucentBackground, true);
    setAttribute(Qt::WA_Hover, true);

    grantsColor = QColor( 158, 158, 158 );
    grantsColor.setAlpha( 128 );
    grantsPen = QPen( grantsColor );
    grantsPen.setWidth( 2 );
    
    m_pTitleWidget = new EWASiteWidgetTitleWidget( this );
    m_pTitleWidget->m_pSiteWidget = this;
    connectTitleWidgetButtons();
    
    m_pScrollArea = new QScrollArea( this );

    m_pSzGrip = new QSizeGrip( this );
    m_pSzGrip->setFixedSize( SIZEGRIP_SZ,SIZEGRIP_SZ );
    m_pSzGrip->hide();
    
    m_iRX = EWAApplication::settings()->m_pGroupApp->getXMsgCornersRadiusX();
    m_iRY = EWAApplication::settings()->m_pGroupApp->getXMsgCornersRadiusY();
    m_dBorderWidth = EWAApplication::settings()->m_pGroupApp->getXMsgBorderWidth();
    
    m_pGridLayout = new QGridLayout( this );
    m_pGridLayout->setMargin( m_dBorderWidth );
    
    m_pGridLayout->addItem( new QSpacerItem(1,16), 0, 0, 1, 1);
    setScrollArea( m_pScrollArea );

    m_pGridLayout->addWidget( m_pScrollArea, 1, 0 );
    m_pGridLayout->setSizeConstraint( QLayout::SetMinAndMaxSize );
    
    m_pWebView = new EWAWebView( 0, calcPrefferedPageSize() );
    m_pWebView->installEventFilter( this );
    m_pScrollArea->setWidget( m_pWebView );
    
    connect( m_pWebView, SIGNAL( signalStatusMessage(const QString&, int) ),
        this, SLOT( slotStatusMessage(const QString&, int) ) );
    connect( m_pWebView, SIGNAL( statusBarMessage(const QString&) ),
        this, SLOT( slotStatusMessage(const QString&) ) );
    
    connect( this, SIGNAL(signalGeometryChanged(const QRect&)),
        this, SIGNAL( signaSiteWidgetModified() ) );    
    connect( this, SIGNAL(signalScrolled(const QPoint&)),
        this, SIGNAL( signaSiteWidgetModified() ) );    
    connect( this, SIGNAL(signalXMsgColorChanged(const QColor&)),
        this, SIGNAL( signaSiteWidgetModified() ) );    
    connect( this, SIGNAL(signalOpacityChanged(int)),
        this, SIGNAL( signaSiteWidgetModified() ) );    
    
    m_dOriginalOpacity = 1.;
    m_eUIState = UIS_NONE;

    resize( 275, 155 );

    m_iTTLSecs = 60;
	m_ptPrevPos = QPoint( 0, 0 );

    setMouseTracking( true );

    m_dOpacityDelta = ( OPACITY_REMISSION_DURATION_SECONDS * 1000.0 )/OPACITY_REMISSION_INTERVAL_MSECONDS;
    if( m_dOpacityDelta )
    {
        m_dOpacityDelta = 1.0 / m_dOpacityDelta;
    }
    else
    {
        m_dOpacityDelta = 1.0;
    }

	m_iSecondsBeforClose = m_iTTLSecs;

	m_pCloserTimer = new EWATimer( this );
	m_pCloserTimer->setInterval( 1000 );

	m_bQuickShowScrolls = false;

    m_pShowScrollsTimer = new EWATimer( this );
	m_pShowScrollsTimer->setInterval( 3000 );

	m_pHideScrollsTimer = new EWATimer( this );
	m_pHideScrollsTimer->setInterval( 1000 );
	
	
    m_pDecOpacityTimer = new EWATimer( this );
    m_pDecOpacityTimer->setInterval( OPACITY_REMISSION_INTERVAL_MSECONDS );

    m_pIncOpacityTimer = new EWATimer( this );
    m_pIncOpacityTimer->setInterval( OPACITY_REMISSION_INTERVAL_MSECONDS );

    vScroll()->setMinimum( 0 );
    hScroll()->setMinimum( 0 );
    
    m_pStatusBar = new EWATextLabel( this );
    m_pStatusBar->setColor( QColor( 0, 170, 255 ) );
    
    m_pStatusBar->hide();
    m_pStatusCleanTimer = new EWATimer( this );
	m_pStatusCleanTimer->setInterval( 750 );
	connect( m_pStatusCleanTimer, SIGNAL( signalTimeOut() ),
	    this, SLOT( slotClearStatus() ) );

    m_gradient.setCoordinateMode( QGradient::ObjectBoundingMode );
    m_color = Qt::black;
    setColor( QColor( 255, 255, 220, 255 ) );
    
    connectMe();
    setAsPreview( false );

    m_bViewModeNormal = true;
    
    m_bScrollsShown = false;
    setScrollsPolicy( Qt::ScrollBarAlwaysOff );
    
    m_bWasShown = false;
    
#ifdef PLAY_WITH_RADIUSES
    RadiusSelectorDlg *pDialog = new RadiusSelectorDlg();
    pDialog->setAttribute( Qt::WA_DeleteOnClose );
    connect( pDialog, SIGNAL( signalXChanged(int) ), this, SLOT( slotXChanged(int) ));
    connect( pDialog, SIGNAL( signalYChanged(int) ), this, SLOT( slotYChanged(int) ));
    connect( pDialog, SIGNAL( signalWChanged(int) ), this, SLOT( slotWChanged(int) ));
    pDialog->show();
#endif //-- #ifdef PLAY_WITH_RADIUSES
}

void EWASiteWidget::setTTL( const int ttl ) 
{ 
    if( m_iTTLSecs != ttl )
    {
        m_iTTLSecs = ttl; 
        emit signaSiteWidgetModified();
    }
}

void EWASiteWidget::connectMe()
{
    if( m_pCloserTimer )
    {
        connect( m_pCloserTimer, SIGNAL( signalTimeOut() ),
            this, SLOT( slotCloserTicked() ) );
    }

    if( m_pShowScrollsTimer )
    {
        connect( m_pShowScrollsTimer, SIGNAL( signalTimeOut() ),
	        this, SLOT( slotShowScrollBars() ) );
	}
	if( m_pHideScrollsTimer )
	{
	    connect( m_pHideScrollsTimer, SIGNAL( signalTimeOut() ),
	        this, SLOT( slotHideScrollBars() ) );
	}
	if( m_pDecOpacityTimer )
	{
        connect( m_pDecOpacityTimer, SIGNAL( signalTimeOut() ),
	        this, SLOT( slotDecreaseOpacity() ) );
	}
	if( m_pIncOpacityTimer )
	{
        connect( m_pIncOpacityTimer, SIGNAL( signalTimeOut() ),
	        this, SLOT( slotIncreaseOpacity() ) );
	}
	
	connectScrolls();
}

void EWASiteWidget::disconnectMe()
{
    if( m_pCloserTimer )
    {
        disconnect( m_pCloserTimer, SIGNAL( signalTimeOut() ),
            this, SLOT( slotCloserTicked() ) );
    }

    if( m_pShowScrollsTimer )
    {
        disconnect( m_pShowScrollsTimer, SIGNAL( signalTimeOut() ),
	        this, SLOT( slotShowScrollBars() ) );
	}
	if( m_pHideScrollsTimer )
	{
	    disconnect( m_pHideScrollsTimer, SIGNAL( signalTimeOut() ),
	        this, SLOT( slotHideScrollBars() ) );
	}
	if( m_pDecOpacityTimer )
	{
        disconnect( m_pDecOpacityTimer, SIGNAL( signalTimeOut() ),
	        this, SLOT( slotDecreaseOpacity() ) );
	}
	if( m_pIncOpacityTimer )
	{
        disconnect( m_pIncOpacityTimer, SIGNAL( signalTimeOut() ),
	        this, SLOT( slotIncreaseOpacity() ) );
	}
	
    disconnectScrolls();
}

void EWASiteWidget::connectScrolls()
{
    connect( vScroll(), SIGNAL( valueChanged(int) ),
        this, SLOT( slotVerticalScroll(int) ) );
    connect( hScroll(), SIGNAL( valueChanged(int) ),
        this, SLOT( slotHorizontallScroll(int) ) );
}

void EWASiteWidget::disconnectScrolls()
{
    disconnect( vScroll(), SIGNAL( valueChanged(int) ),
        this, SLOT( slotVerticalScroll(int) ) );
    disconnect( hScroll(), SIGNAL( valueChanged(int) ),
        this, SLOT( slotHorizontallScroll(int) ) );
}


EWASiteWidget::~EWASiteWidget()
{
    disconnectMe();

    delete m_pWebView;
	delete m_pCloserTimer;
    delete m_pDecOpacityTimer;
}

void EWASiteWidget::setSite( EWASiteHandle *pSite )
{
    if( pSite && m_sitePtr != pSite )
    {
        m_sitePtr = pSite;
        setEWANetworkManagerPtr( m_sitePtr->getEWANetworkManagerPtr() );

        m_pWebView->setSite( m_sitePtr );
        m_pTitleWidget->setSite( m_sitePtr );
    }
}

void EWASiteWidget::makeInvisible( bool bInvis )
{
    if( bInvis != testAttribute( Qt::WA_DontShowOnScreen ) )
        setAttribute( Qt::WA_DontShowOnScreen, bInvis );
        
    if( bInvis != testAttribute( Qt::WA_WState_ExplicitShowHide ) )
        setAttribute( Qt::WA_WState_ExplicitShowHide, bInvis );
}

bool EWASiteWidget::isInvisible() const
{
    return testAttribute( Qt::WA_DontShowOnScreen );
}

void EWASiteWidget::showSexy()
{
#ifdef PLAY_WITH_FLAGS
    pFlagsDlg->setTarget( this );
    pFlagsDlg->show();
#endif //-- PLAY_WITH_FLAGS    

    if( isViewNormal() )
    {
        slotScrollTo( getCurrnetScrollPoint() );
    }
    
    if( isPreview() )
    {
        m_bWasShown = true;
        return;
    }

    stopOpacityChanges();


    makeInvisible( false );

    if( !isPreview() && getWebView()->isBlankPage() )
    {
        return;
    }
    
    m_bQuickShowScrolls = false;

    m_iSecondsBeforClose = m_iTTLSecs;
    
    if( EWAApplication::settings()->m_pGroupApp->useRichWM() && isViewNormal() )
    {
        setBaseOpacity( getBaseOpacity() );
    }
    
    if( ( !m_bWasShown )
        &&
        ( !isPreview() && isViewNormal() ) )
    {
        if( EWAApplication::settings()->m_pGroupApp->useRichWM() )
        {
            setWindowOpacity( .0 );
            m_pIncOpacityTimer->start();
        }
	}
	qApp->processEvents();
	
	show();
	
    if( eCurrentHideVewMode == EM_HVM_ALWAYS_ON_DESKTOP && isViewNormal())
	{
	    lower();
	}
	
    if( !isViewNormal() )
	{   
	    //-- we are here from SiteHandle - 
	    //-- it seems that user-navigated-by-link page
	    //-- is downloaded.
	    //-- resoze scroll bars ranges:
	    setViewMaximized();
	}
	//else
	//{
 //           //restoreLocation();
	//    slotScrollTo( getCurrnetScrollPoint() );
	//}
	
	m_bWasShown = true;
}

void EWASiteWidget::stopOpacityChanges()
{
    m_pDecOpacityTimer->stop();
    m_iSecondsBeforClose = m_iTTLSecs;
    
    if( EWAApplication::settings()->m_pGroupApp->useRichWM() )
    {
        setBaseOpacity( getBaseOpacity() );
        if( m_bScrollsShown )
        {
            setWindowOpacity( 1.0 );
        }
    }
}

void EWASiteWidget::slotSetViewAlwaysOnTop( bool bAndShow )
{
    if( !isPreview() )
    {
        eCurrentHideVewMode = EM_HVM_ALWAYS_ON_TOP;
        setWindowFlags( getFlagsNormal() );

        if( bAndShow )
		{
		    stopOpacityChanges();
		    show();
            m_pCloserTimer->stop();
#ifdef PLAY_WITH_FLAGS
            pFlagsDlg->updateCheckboxes();
#endif //-- PLAY_WITH_FLAGS                
        }
        else
            hide();
    }
}

void EWASiteWidget::slotSetViewAlwaysOnDesktop( bool bAndShow )
{
    Q_UNUSED( bAndShow );
    
    if( !isPreview() )
    {
        stopOpacityChanges();
        
        m_pCloserTimer->stop();
        
        eCurrentHideVewMode = EM_HVM_ALWAYS_ON_DESKTOP;
        setWindowFlags( getFlagsNormal() );}
#ifdef PLAY_WITH_FLAGS
        pFlagsDlg->updateCheckboxes();
#endif //-- PLAY_WITH_FLAGS
}

void EWASiteWidget::slotSetViewAutoClose( bool bAndShow )
{
    if( !isPreview() )
    {
        eCurrentHideVewMode = EM_HVM_AUTOHIDE;
        setWindowFlags( getFlagsNormal() );
        
        if( bAndShow )
		{
		    stopOpacityChanges();
		    show();
            m_pCloserTimer->start();
#ifdef PLAY_WITH_FLAGS
            pFlagsDlg->updateCheckboxes();
#endif //-- PLAY_WITH_FLAGS
        }
    }
}

void EWASiteWidget::slotNeedGoBack()
{
    if( m_sitePtr && !isOnAddWizard() )
    {
        m_pWebView->triggerPageAction( QWebPage::Back, true );
    }
}

void EWASiteWidget::slotNeedRefresh()
{
    if( m_sitePtr && !isOnAddWizard() )
    {
        QString strFavIconBtnToolTip;
        QWebPage::WebAction eWebAction;
        if( m_sitePtr->isDownloadingActive() )
        {
            eWebAction = QWebPage::Stop;
            strFavIconBtnToolTip = tr( "Refresh" );
        }
        else
        {
            eWebAction = QWebPage::Reload;
            strFavIconBtnToolTip = tr( "Stop" );
        }

        m_pTitleWidget->refreshButton()->setToolTip( strFavIconBtnToolTip );
        
        m_pWebView->triggerPageAction( eWebAction, true );      
    }
}

void EWASiteWidget::slotNeedGoForward()
{
    if( m_sitePtr && !isOnAddWizard() )
    {
        m_pWebView->triggerPageAction( QWebPage::Forward, true );
    }
}

void EWASiteWidget::slotNeedShowMaximized()
{
    if( !isPreview() 
        && m_sitePtr 
        && isViewNormal() )
    {
        if( !sender() )
        {
            m_pTitleWidget->viewModeButton()->click();
            return;
        }
        
        scroll2top();
        setViewMaximized();
        
#ifdef PLAY_WITH_FLAGS
        pFlagsDlg->updateCheckboxes();
#endif //-- PLAY_WITH_FLAGS
    }
}

void EWASiteWidget::slotNeedShowNormal()
{
    if( !isPreview() 
        && m_sitePtr
        && !isViewNormal() )
    {
        if( !sender() )
        {
            m_pTitleWidget->viewModeButton()->click();
            return;
        }
        
        setViewNormal();
#ifdef PLAY_WITH_FLAGS
        pFlagsDlg->updateCheckboxes();
#endif //-- PLAY_WITH_FLAGS
    }
}

bool EWASiteWidget::isViewNormal() const 
{
    return m_bViewModeNormal;
}

void EWASiteWidget::slotNeedShowEditor()
{
    EWAApplication::getSitesManager()->showEditorDialogForSite( m_sitePtr );
}

void EWASiteWidget::slotNeedClose()
{
    close();
}

void EWASiteWidget::mousePressEvent( QMouseEvent *event )
{
    m_ptPrevPos = event->globalPos();

    if( m_eUIState == UIS_NONE )
    {
        if( isMovablePoint( m_ptPrevPos ) )
        {
            m_eUIState = UIS_MOVE;
        }
        else
        {
            m_eUIState = UIS_NONE;
        }
    }

    QWidget::mousePressEvent( event );
}
 void EWASiteWidget::mouseReleaseEvent( QMouseEvent *event )
{
    m_eUIState = UIS_NONE;
    setCursor( Qt::ArrowCursor );

    if( event->button() == Qt::MidButton )
    {
        if( isMovablePoint( event->globalPos() ) )
        {
            close();
        }
    }

    QWidget::mouseReleaseEvent( event );
}

 void EWASiteWidget::mouseMoveEvent( QMouseEvent *event )
{
    QPoint click = event->globalPos();

    if( event->buttons() == Qt::LeftButton )
    {
        if( m_eUIState == UIS_MOVE && isViewNormal() )
        {
            QRect currentGeom = geometry();
            QPoint newPos = mapToGlobal( this->pos() );
            QPoint delta = click - m_ptPrevPos;
            newPos += delta;
            currentGeom.moveTo( mapFromGlobal( newPos ) );

            stickToScreenEdges( delta, currentGeom );

            setLocation( currentGeom.topLeft() );
        }
    }
    else
    {
        setCursor( Qt::ArrowCursor );
    }

    m_ptPrevPos = click;

    QWidget::mouseMoveEvent( event );
}

void EWASiteWidget::stickToScreenEdges( const QPoint& ptClickDelta, QRect &widgetGeometry, int iPrecision ) const
{
    const QRect &screenGeometry = EWAApplication::getScreenSize( m_ptPrevPos+ptClickDelta );
    
    bool bEast2West = ptClickDelta.x() > 0; //-- < 0 ? right to left : left toright
    bool bNorth2South = ptClickDelta.y() > 0; //-- < 0 ? down to up : up to dow;
    
    //-- stick to current screen's left border:
    int iXDeltaLeft = widgetGeometry.topLeft().x() - screenGeometry.topLeft().x();
    if( !bEast2West && iXDeltaLeft <= iPrecision )
    {
        widgetGeometry.moveTopLeft( QPoint( screenGeometry.x(),
                                            widgetGeometry.topLeft().y() ) );
    }
    
    //-- stick to current screen's right border:
    int iXDeltaRight = screenGeometry.x() + screenGeometry.width() - widgetGeometry.topRight().x();
    if( bEast2West && iXDeltaRight <= iPrecision )
    {
        widgetGeometry.moveTopRight( QPoint( screenGeometry.x() + screenGeometry.width(),
                                             widgetGeometry.topRight().y() ) );
    }

    //-- stick to current screen's top border:
    int iYDeltaTop = widgetGeometry.topLeft().y() - screenGeometry.y() ;
    if( !bNorth2South && iYDeltaTop <= iPrecision )
    {
        widgetGeometry.moveTopLeft( QPoint( widgetGeometry.topLeft().x(),
                                            screenGeometry.y() ) );
    }
    
    //-- stick to current screen's bottom border:
    int iYDeltaBottom = screenGeometry.y() + screenGeometry.height() - widgetGeometry.bottomLeft().y();
    if( bNorth2South && iYDeltaBottom <= iPrecision )
    {
        widgetGeometry.moveBottomLeft( QPoint( widgetGeometry.bottomLeft().x(),
                                               screenGeometry.y() + screenGeometry.height() ) );
    }
}

void EWASiteWidget::mouseDoubleClickEvent( QMouseEvent *event )
{
    if( isMovablePoint( event->globalPos() ) )
    {
        m_pTitleWidget->viewModeButton()->setChecked( !m_pTitleWidget->viewModeButton()->isChecked() );
    }

    QWidget::mouseDoubleClickEvent( event );
}

void EWASiteWidget::setColor( const QColor& color )
{
    if( m_color != color && m_color.isValid() )
    {
        m_color = color;
        m_gradient.setColorAt( 0.00, Qt::white );
        m_gradient.setColorAt( 0.25, m_color.lighter( 150 ) );
        m_gradient.setColorAt( 0.50, m_color );
        m_gradient.setColorAt( 0.75, m_color.darker( 150 ) );
        m_gradient.setColorAt( 1.00, Qt::white );
        
        m_pStatusBar->setColor( m_color );
        emit signalXMsgColorChanged( m_color ); //-- should emit signaSiteWidgetModified();
        update();
    }
}

void EWASiteWidget::setTitle( const QString &title )
{
    QString strLimited = title.left( 32 );
    QString strCurrTitle = m_pTitleWidget->label()->text().trimmed();
    if( strCurrTitle.compare( strLimited, Qt::CaseInsensitive ) )
    {
        m_pTitleWidget->label()->setText( strLimited );
        m_pTitleWidget->label()->setToolTip( strLimited );
        setWindowTitle( strLimited );
        
        emit signaSiteWidgetModified();
    }
}

QString EWASiteWidget::getTitle() const
{
    return m_pTitleWidget->label()->text();
}

bool EWASiteWidget::amIRaisable() const
{
    if( (m_pTitleWidget && m_pTitleWidget->isMenuVisible() )
        || ( m_pWebView && m_pWebView->isMenuVisible() )
        || EM_HVM_ALWAYS_ON_DESKTOP == eCurrentHideVewMode )
    {
        return false;
    }
    
    return true;
}

void EWASiteWidget::enterEvent( QEvent *event )
{
    Q_UNUSED( event );

    setFocus( Qt::MouseFocusReason );
    m_pScrollArea->setFocus( Qt::MouseFocusReason );
    if( m_pScrollArea->widget() )
        m_pScrollArea->widget()->setFocus( Qt::MouseFocusReason );
    
    if( !isPreview() && isViewNormal() && amIRaisable() )
    {
        activateWindow();
        raise();
    }

    if( m_pHideScrollsTimer->isActive() )
    {

        m_pHideScrollsTimer->stop();
        return slotShowScrollBars();
    }

    if( !m_pShowScrollsTimer->isActive() 
        && scrollsPolicyIs( Qt::ScrollBarAlwaysOff ) )
    {
        m_pShowScrollsTimer->start();
    }
}

void EWASiteWidget::leaveEvent( QEvent *event )
{
    Q_UNUSED( event );
    if( m_pShowScrollsTimer->isActive() )
    {
        m_pShowScrollsTimer->stop();
    }

    if( !m_pHideScrollsTimer->isActive() )
    {
        if( isViewNormal() && m_bWasShown )
        {
            m_pHideScrollsTimer->start();
        }
    }
}

void EWASiteWidget::updateSizeGripGeometry()
{
    QRect szGrpGeom = m_pSzGrip->geometry();
    //-- to place m_pSzGrip to the our bottom right corner
    //-- mapFromParent can be used, but i think that this way 
    //-- more easy:
    QPoint pntDelta( m_dBorderWidth, m_dBorderWidth ); 
    QPoint dstPnt = m_pScrollArea->geometry().bottomRight() + pntDelta;
    
    if( szGrpGeom.center() != dstPnt )
    {
        szGrpGeom.moveBottomRight( dstPnt );
        m_pSzGrip->setGeometry( szGrpGeom );
    }
}

void EWASiteWidget::showScrollBars()
{
    setScrollsPolicy( Qt::ScrollBarAsNeeded );

    hScroll()->show();
    vScroll()->show();
    
    /*if( !m_bScrollsShown )
    {
        qApp->processEvents(); //-- updates scrollbars geometry
        
        int iDeltaX = vScroll()->maximum() ? vScroll()->width() : 0;
        int iDeltaY = hScroll()->maximum() ? hScroll()->height() : 0;
        //-- update webview's geometry to fit it into the scrollview
        //-- wit scrollbars:
        if( iDeltaX || iDeltaY )
        {
            QSize sz( iDeltaX, iDeltaY );
            //qDebug() << "showScrollBars: from " << getWebView()->size() << "to" << getWebView()->size() - sz;
            getWebView()->resize( getWebView()->size() - sz );
        }
    }*/
    
    m_bScrollsShown = true;
}

void EWASiteWidget::hideScrollBars()
{
    setScrollsPolicy( Qt::ScrollBarAlwaysOff );
    
    int iDeltaX = vScroll()->maximum() ? vScroll()->width() : 0;
    int iDeltaY = hScroll()->maximum() ? hScroll()->height() : 0;
    
    hScroll()->hide();
    vScroll()->hide();
    
    //-- restore webview's geometry values as were
    //-- before scrolls show:
    /*if( m_bScrollsShown 
        && ( iDeltaX || iDeltaY ) )
    {
        QSize sz( iDeltaX, iDeltaY );
        //qDebug() << "hideScrollBars: from " << getWebView()->size() << "to" << getWebView()->size() + sz;
        getWebView()->resize( getWebView()->size() + sz );
    }*/
    
    m_bScrollsShown = false;
}
    
bool EWASiteWidget::scrollsPolicyIs( Qt::ScrollBarPolicy policy ) const
{
    return ( m_pScrollArea->horizontalScrollBarPolicy() == policy
             && m_pScrollArea->verticalScrollBarPolicy() == policy );
}

void EWASiteWidget::setScrollsPolicy( Qt::ScrollBarPolicy policy )
{
    m_pScrollArea->setHorizontalScrollBarPolicy( policy );
    m_pScrollArea->setVerticalScrollBarPolicy( policy );
}

void EWASiteWidget::slotShowScrollBars()
{
    if( !m_sitePtr 
    || scrollsPolicyIs( Qt::ScrollBarAsNeeded ) )
    {
        return;
    }
    
    if( m_pShowScrollsTimer->isActive() )
    {
        m_pShowScrollsTimer->stop();
    }
    if( m_pHideScrollsTimer->isActive() )
    {
        m_pHideScrollsTimer->stop();
    }

    showScrollBars();
    
    
    updateSizeGripGeometry();
    m_pSzGrip->show();
    
    //-- if scroll bars are shown it means that user 
    //-- look on the web page, so it'll be usefull
    //-- to make it non-transparent.
    //-- but if message widget is in "preview" mode - we can't
    //-- change transparency, because it'll change values in
    //-- editor dialog.
    if( !isPreview() )
    {
        if( EWAApplication::settings()->m_pGroupApp->useRichWM() )
        {
            setWindowOpacity( 1.0 );
        }
    }
    
    setTitleBarVisible();
}

void EWASiteWidget::setTitleBarVisible( bool bVisible )
{
    bVisible ? m_pTitleWidget->showControlls() : m_pTitleWidget->hideControlls();
}

void EWASiteWidget::slotHideScrollBars()
{
    if(  m_pShowScrollsTimer->isActive() )
    {
        m_pShowScrollsTimer->stop();
    }
    if( m_pHideScrollsTimer->isActive() )
    {
        m_pHideScrollsTimer->stop();
    }

    if( !isViewNormal()
    || isOnAddWizard() 
    || scrollsPolicyIs( Qt::ScrollBarAlwaysOff ) )
    {
        return;
    }
    
    m_pSzGrip->hide();    

    hideScrollBars();

    setTitleBarVisible( false );
    
    //--it's not needed to restore base opacity
    //-- when widgets closing 
    if( !m_pDecOpacityTimer->isActive() )
    {
        if( EWAApplication::settings()->m_pGroupApp->useRichWM() )
        {
            setWindowOpacity( getBaseOpacity() );
        }
    }
}

void EWASiteWidget::slotSwitchOffQuickShowScrolls()
{
    m_bQuickShowScrolls = false;
}

void EWASiteWidget::slotCloserTicked()
{
    if( !isPreview() && !isOnAddWizard() && isViewNormal() )
    {
        --m_iSecondsBeforClose;

        if( m_iSecondsBeforClose <= OPACITY_REMISSION_DURATION_SECONDS )
        {
		    m_pDecOpacityTimer->start();
            m_pCloserTimer->stop();
        }
    }

}

void EWASiteWidget::slotCloseButtonClicked()
{
    if( !isPreview() )
    {
        close();
    }
}

void EWASiteWidget::show()
{
    QWidget::show();
}

bool EWASiteWidget::close()
{
    if( isPreview() )
    {
        return false;
    }
    
    if( !isVisible() )
    {
        realClose();//-- deactivating timers and scrolls
        return false;
    }
    
    if( !isViewNormal() )
    {
        slotNeedShowNormal();
    }
    
    m_pCloserTimer->stop();
    m_pDecOpacityTimer->start();
    
    return true;
}

void EWASiteWidget::slotDecreaseOpacity()
{
    if( EWAApplication::settings()->m_pGroupApp->useRichWM() )
    {
        if( isPreview() 
            || !isVisible() 
            || !isViewNormal() )
        {
            stopOpacityChanges();
            return;
        }

        setWindowOpacity( windowOpacity() - m_dOpacityDelta );
        if( windowOpacity() <= 0. )
        {
            realClose();
        }
    }
    else
    {
        realClose();
    }
}

void EWASiteWidget::slotIncreaseOpacity()
{
    if( EWAApplication::settings()->m_pGroupApp->useRichWM() )
    {
        setWindowOpacity( windowOpacity() + m_dOpacityDelta + m_dOpacityDelta );
        if( windowOpacity() >= getBaseOpacity() )
        {
            setWindowOpacity( getBaseOpacity() );
            m_pIncOpacityTimer->stop();
            
            if( !isPreview() && eCurrentHideVewMode == EM_HVM_AUTOHIDE )
            {
                m_pCloserTimer->start();
            }
        }
    }

    if( !isVisible() )
    {
        m_pIncOpacityTimer->stop();
    }
}


void EWASiteWidget::realClose()
{
#ifdef PLAY_WITH_FLAGS
    pFlagsDlg->setTarget( 0 );
    pFlagsDlg->close();
#endif //-- PLAY_WITH_FLAGS
    
    slotClearStatus();
    slotHideScrollBars();
    
    hide();
    
    if( m_pTitleWidget->viewModeButton()->isChecked() )
    {
        m_pTitleWidget->viewModeButton()->setChecked( false );
    }
    QWidget::close();

    if( m_sitePtr && m_pWebView )
    {//-- emulate "closing tab in browser"
        m_pWebView->clear();
    }
    
    m_pCloserTimer->stop();
    m_pDecOpacityTimer->stop();
    m_pIncOpacityTimer->stop();
    m_pShowScrollsTimer->stop();
    m_pHideScrollsTimer->stop();
    m_pStatusCleanTimer->stop();
    
    m_bWasShown = false;
}

void EWASiteWidget::autoRestore()
{
    showNormal();
    activateWindow();
    raise();
}

void EWASiteWidget::setAsPreview( bool preview )
{
    if( isPreview() == preview )
    {
        return;
    }

    m_bImPreview = preview;
    m_pTitleWidget->setIsOnPreview( m_bImPreview );

    m_pTitleWidget->label()->setEnabled( true );

    if( m_bImPreview )
    {
        if( !isViewNormal() )
        {
            setViewNormal();
        }
        
        if( isOnAddWizard() )
        {
            m_pWebView->setContextMenuPolicy( Qt::NoContextMenu );
            
            slotShowScrollBars();
        }
        else
        {
            m_pWebView->setContextMenuPolicy( Qt::DefaultContextMenu );
        }
    }
}

bool EWASiteWidget::isOnAddWizard() const
{
    return (isPreview() && ( parent() != 0 ) );
}

 void EWASiteWidget::keyPressEvent( QKeyEvent *event )
{
    int iPressedKey = event->key();
    bool bAlt = event->modifiers() & Qt::AltModifier;
    bool bCtrl = event->modifiers() & Qt::ControlModifier;
    
    bool bCloseRequest = false;
    switch( iPressedKey )
    {
        case Qt::Key_Escape:
        {
            bCloseRequest = true;
            break;
        }
        case Qt::Key_F4:
        {
            bCloseRequest = bAlt;
            break;
        }
        case Qt::Key_W:
        {
            bCloseRequest = bCtrl;
            break;
        }
        default :
        {
            bCloseRequest = false;
            break;
        }
    }
    
    QWidget::keyPressEvent( event );
    
    if( bCloseRequest )
    {
        close();
    }
}

bool EWASiteWidget::eventFilter( QObject *obj, QEvent *event )
{
    QEvent::Type iType = event->type();
    
    switch( iType )
    {
        case QEvent::Wheel:
        {
            QWheelEvent *wheelEvent = static_cast<QWheelEvent *>(event);
            customProcessWheel( wheelEvent );
            
            break;
        }
        case QEvent::KeyPress:
        case QEvent::KeyRelease:
        {
            if( isOnAddWizard() )
            {
                QKeyEvent *pKeyEvent = static_cast<QKeyEvent*>(event);
                if( pKeyEvent && pKeyEvent->key() == Qt::Key_Tab )
                {
                    QMessageBox::information( this, tr( "Add Site Wizard" ), 
                    tr( "Tab key is unavailable. Please, use mouse to change input focus." ) );
                    event->accept();
                    return true;
                }
            }
            
            break;
        }
        case QEvent::MouseButtonPress:
        {
            QMouseEvent *pMouseEvent = static_cast<QMouseEvent*>(event);
            if( pMouseEvent->button() == Qt::MidButton )
            {
                if( !isPreview() )
                {
                    close();
                }
                event->accept();
                return true;
            }
            
            break;
        }
        default:
        {
            break;
        }
    }

    //-- standard event processing
    return QWidget::eventFilter(obj, event);
}

void EWASiteWidget::wheelEvent( QWheelEvent *event )
{
    if( m_sitePtr )
        customProcessWheel( event );
        
    QWidget::wheelEvent( event );
}

void EWASiteWidget::customProcessWheel( QWheelEvent *event )
{
    int numDegrees = event->delta() / 8;
    int numSteps = numDegrees / 15;
    switch( qApp->keyboardModifiers() )
    {
        case Qt::NoModifier:
        {//-- verticall scroll:
            int pgStep = vScroll()->singleStep();
            int currentValue = vScroll()->value();
            vScroll()->setValue(  currentValue - numSteps * pgStep );
            
            break;
        }
        case Qt::AltModifier:
        {//-- horizontall scroll:
            int pgStep = hScroll()->singleStep();
            int currentValue = hScroll()->value();
            hScroll()->setValue(  currentValue - numSteps * pgStep );
            
            break;
        }
        case Qt::ShiftModifier:
        {//-- change our opacity:
            double dOpacity = windowOpacity();
            dOpacity += 0.05*( float )numSteps;
            if( dOpacity < OPACITY_MAX )
            {
                dOpacity = OPACITY_MAX;
            }
            else if( dOpacity > 1. )
            {
                dOpacity = 1.;
            }

            setBaseOpacity( dOpacity );
            
            break;
        }
        case Qt::ControlModifier:
        {
            qreal mult = m_pWebView->textSizeMultiplier();
            mult += 0.1*numSteps;
            m_pWebView->setTextSizeMultiplier( mult );

            break;
        }
    }
}

void EWASiteWidget::setViewMaximized()
{
    if( !isViewNormal() )
    {
        return;
    }
    
    m_prevRect = geometry();

    m_prevScroll = m_ptCurrScroll;//m_qpntBaseScoll;
    m_prevWebPageSz = m_sitePtr->getWebPageBaseSize();

    setGeometry( EWAApplication::getScreenSize( geometry().center() ) );
    m_bViewModeNormal = false;
    QSize szNew = scrollArea()->viewport()->size();
    
    if( m_bScrollsShown )
    {
        const int iDeltaX = m_prevWebPageSz.width() > szNew.width() ? vScroll()->width() : 0;
        const int iDeltaY = m_prevWebPageSz.height() > szNew.height() ? hScroll()->height() : 0;
        if( iDeltaX || iDeltaY )
        {
            szNew += QSize( iDeltaX, iDeltaY );
        }
    }
    
    m_sitePtr->setWebPageBaseSize( szNew );
    
    slotShowScrollBars();
    

    bool bYesIsVisible = isVisible();

    setWindowFlags( getFlagsMaximized() );

    if( bYesIsVisible )
    {
        show();
    }
    
    if( EWAApplication::settings()->m_pGroupApp->useRichWM() )
    {
        setWindowOpacity( 1. );
    }

    
    
    m_pTitleWidget->stickButton()->setEnabled( false );
}

void EWASiteWidget::setViewNormal()
{
    if( isPreview() || !m_sitePtr )
    {
        return;
    }

    if( !isViewNormal() )
    {
        m_bViewModeNormal = true;
        bool bYesIsVisible = isVisible();
        setGeometry( m_prevRect );

//-- it sometimes on KDE remains maximized
#ifdef Q_OS_LINUX//-- it sometimes on KDE remains maximized
        qApp->processEvents();
#endif  //-- Q_OS_LINUX//-- it sometimes on KDE remains maximized

        setWindowFlags( getFlagsNormal() );
#ifdef Q_OS_LINUX//-- it sometimes on KDE remains maximized
        setGeometry( m_prevRect );
#endif  //-- Q_OS_LINUX//-- it sometimes on KDE remains maximized
        
        if( bYesIsVisible )
        {
            show();            
        }
#ifdef Q_OS_LINUX//-- it sometimes on KDE remains maximized
        qApp->processEvents();
        setGeometry( m_prevRect );
        qApp->processEvents();
#endif  //-- Q_OS_LINUX//-- it sometimes on KDE remains maximized
        slotScrollTo( m_prevScroll );
        if( EWAApplication::settings()->m_pGroupApp->useRichWM() )
        {
            setWindowOpacity( m_dOriginalOpacity );
        }

        if( eCurrentHideVewMode == EM_HVM_AUTOHIDE )
        {
            m_pCloserTimer->start();
        }

        slotHideScrollBars();
        
        if( !m_prevWebPageSz.isEmpty() )
        {
            m_sitePtr->setWebPageBaseSize( m_prevWebPageSz );
        }

        m_pTitleWidget->stickButton()->setEnabled( true );
        if( m_pTitleWidget->viewModeButton()->isChecked() )
        {
            m_pTitleWidget->viewModeButton()->setChecked( false );
        }
    }
}

void EWASiteWidget::cloneSettings( const EWASiteWidget *pOther )
{
    if( pOther )
    {
        setColor( pOther->getColor() );
        setNewSize( pOther->width(), pOther->height() );
        setLocation( pOther->getLocation() );
        setBaseOpacity( pOther->getBaseOpacity() );
        setBaseScrollPoint( pOther->getCurrnetScrollPoint() );
        m_iSecondsBeforClose = m_iTTLSecs = pOther->m_iTTLSecs;
        setBackButtonShown( pOther->isBackButtonShown() );
        setRefreshButtonShown( pOther->isRefreshButtonShown() );
        setForwardButtonShown( pOther->isForwardButtonShown() );
        setTitleLabelShown( pOther->isTitleLabelShown() );
        setStickButtonShown( pOther->isStickButtonShown() );
        setViewModeButtonShown( pOther->isViewModeButtonShown() );
        setCloseButtonShown( pOther->isCloseButtonShown() );
    }
}

double EWASiteWidget::getBaseOpacity() const
{
    return m_dOriginalOpacity;
}

void EWASiteWidget::setBaseOpacity( const double transparency )
{
    if( EWAApplication::settings()->m_pGroupApp->useRichWM()
    && transparency >= .0
    && transparency <= 1.0
    && getBaseOpacity() != transparency )
    {
        m_dOriginalOpacity = transparency;
        setWindowOpacity( getBaseOpacity() );
        emit signaSiteWidgetModified();
    }
}

void EWASiteWidget::setBaseScrollPoint( const QPoint& pnt )
{
    if( getCurrnetScrollPoint() != pnt )
    {
        m_qpntBaseScoll = pnt;
        if( hScroll()->maximum() < m_qpntBaseScoll.x() )
        {
            hScroll()->setMaximum( m_qpntBaseScoll.x() );
        }
        if( vScroll()->maximum() < m_qpntBaseScoll.y() )
        {
            vScroll()->setMaximum( m_qpntBaseScoll.y() );
        }
        
        slotScrollTo( m_qpntBaseScoll );
        
        emit signaSiteWidgetModified();
    }
}

void EWASiteWidget::slotScrollTo( const QPoint& pnt )
{
    hScroll()->setValue( pnt.x() );
    vScroll()->setValue( pnt.y() );
}

void EWASiteWidget::paintEvent( QPaintEvent *event )
{
    Q_UNUSED( event );
    if( isVisible() && !isInvisible() )//-- lol
    {
        QPainter p( this );

        QRect r = rect();
        if( EWAApplication::settings()->m_pGroupApp->useRichWM() )
        {
            QPoint pnt1 = r.topLeft() + QPoint( 0, getCustomizedVerticalOffset() );
            r = QRect( pnt1, r.bottomRight() );
            p.setOpacity(0.6);
        }

        p.setRenderHints( EWAApplication::getRenderHints() );
        p.setPen( Qt::transparent );
        p.setBrush( m_color );

        p.drawRoundedRect( r, m_iRX, m_iRY );
    }
}

 void EWASiteWidget::resizeEvent( QResizeEvent *event )
{
    QWidget::resizeEvent( event );
    
	m_pTitleWidget->updateGeomtery( rect() );
	updateSizeGripGeometry();
	updateStatusBarGeometry();

    if( isViewNormal() )
    {
        if( !EWAApplication::settings()->m_pGroupApp->useRichWM() )
        {
            QRect r = rect();
            QPixmap maskPixmap(r.size());

            QPainter painter( &maskPixmap );
            painter.setRenderHints( EWAApplication::getRenderHints() );
            painter.setBrush( m_color );

            painter.drawRoundedRect( r, m_iRX, m_iRY );

            setMask( maskPixmap.createMaskFromColor( m_color, Qt::MaskOutColor ) );
        }
        
        if( event->oldSize() != size() )
        {
            update();
            if( !event->oldSize().isEmpty() )
            {
                emit signalGeometryChanged( geometry() );         
            }
        }
    }
}

 void EWASiteWidget::moveEvent( QMoveEvent *event )
{
    if( isViewNormal() )
    {
        QWidget::moveEvent( event );
        
        if( event->oldPos() != pos() )
        {
            emit signalGeometryChanged( geometry() );
        }
    }
}

void EWASiteWidget::slotHorizontallScroll( int x )
{
    m_ptCurrScroll.setX( x );
    emit signalScrolled( m_ptCurrScroll );
}
void EWASiteWidget::slotVerticalScroll( int y )
{
    m_ptCurrScroll.setY( y );
    emit signalScrolled( m_ptCurrScroll );
}

void EWASiteWidget::setWindowOpacity(qreal level)
{
    if( EWAApplication::settings()->m_pGroupApp->useRichWM() )
    {
        if( windowOpacity() != level )
        {
            QWidget::setWindowOpacity(level);
            emit signalOpacityChanged(100 - level*100.);
        }
    }
}

QRect EWASiteWidget::getVisibleWebViewRect() const
{
    return m_pScrollArea->viewport()->geometry();
}

QSize EWASiteWidget::getVisibleWebViewSize() const
{
    return getVisibleWebViewRect().size();
}



void EWASiteWidget::connectTitleWidgetButtons()
{
    connect( m_pTitleWidget, SIGNAL( signalNeedAlwaysOnTop() ), this, SLOT( slotSetViewAlwaysOnTop() ) );
    connect( m_pTitleWidget, SIGNAL( signalNeedAlwaysOnDesktop() ), this, SLOT( slotSetViewAlwaysOnDesktop() ) );
    
    connect( m_pTitleWidget, SIGNAL( signalNeedAutoclose() ), this, SLOT( slotSetViewAutoClose() ) );
    
    connect( m_pTitleWidget, SIGNAL( signalNeedGoBack() ), this, SLOT( slotNeedGoBack() ) );
    connect( m_pTitleWidget, SIGNAL( signalNeedRefresh() ), this, SLOT( slotNeedRefresh() ) );
    connect( m_pTitleWidget, SIGNAL( signalNeedGoForward() ), this, SLOT( slotNeedGoForward() ) );
    connect( m_pTitleWidget, SIGNAL( signalNeedShowMaximized() ), this, SLOT( slotNeedShowMaximized() ) );
    connect( m_pTitleWidget, SIGNAL( signalNeedShowNormal() ), this, SLOT( slotNeedShowNormal() ) );
    connect( m_pTitleWidget, SIGNAL( signalNeedShowEditor() ), this, SLOT( slotNeedShowEditor() ) );
    connect( m_pTitleWidget, SIGNAL( signalNeedClose() ), this, SLOT( slotNeedClose() ) );
}

bool EWASiteWidget::isMovablePoint( const QPoint& globalPoint ) const
{
    if( !isOnAddWizard() )
    {
        bool isOnTitleWidget = m_pTitleWidget->rect().contains( m_pTitleWidget->mapFromGlobal( globalPoint ) );
        bool isOnMe = rect().contains( mapFromGlobal( globalPoint ) );
        bool isOnWebView = m_pScrollArea->viewport()->rect().contains( m_pScrollArea->viewport()->mapFromGlobal( globalPoint ) );
        
        if( isOnTitleWidget || ( isOnMe && !isOnWebView ) )
        {
            return true;
        }
    }
    
    return false;
}

void  EWASiteWidget::setEWANetworkManagerPtr( EWANetworkAccessManager* pManager )
{
    if( pManager )
    {
        m_accessManagerPtr = pManager;
    }
}

bool EWASiteWidget::getWebViewTumbnail( QPixmap& image ) const
{
    image = QPixmap();
    image = QPixmap::grabWidget( m_pScrollArea->viewport() );
    
    return !image.isNull();
}

void EWASiteWidget::initForUpdatesEnabled()
{
    disconnect( m_pTitleWidget, SIGNAL( signalNeedShowEditor() ), this, SLOT( slotNeedShowEditor() ) );

    setBackButtonShown( false );
    setRefreshButtonShown( false );
    setForwardButtonShown( false );
    
    setStickButtonShown( false );
    setViewModeButtonShown( false );
    setCloseButtonShown( true );
    setTitleLabelShown( true );
    
    setTTL( 600 );
    QRect msgRect = geometry();
    msgRect.moveBottomRight( EWAApplication::getScreenSize().bottomRight() );
    setGeometry( msgRect );
    setColor( QColor( "#005990" ) );
}

#ifdef PLAY_WITH_RADIUSES

RadiusSelectorDlg::RadiusSelectorDlg()
:QWidget()
{
    QHBoxLayout *pLayout = new QHBoxLayout( this );
    m_pXSB = new QSpinBox( this );
    m_pXSB->setRange( 0, 1024 );
    
    m_pYSB = new QSpinBox( this );
    m_pYSB->setRange( 0, 1024 );
    
    m_pFrameSB = new QSpinBox( this );
    m_pFrameSB->setRange( 0, 1024 );
    
    pLayout->addWidget( m_pXSB );
    pLayout->addWidget( m_pYSB );
    pLayout->addWidget( m_pFrameSB );
    
    connect( m_pXSB, SIGNAL( valueChanged(int) ), this, SIGNAL( signalXChanged(int) ) );
    connect( m_pYSB, SIGNAL( valueChanged(int) ), this, SIGNAL( signalYChanged(int) ) );
    connect( m_pFrameSB, SIGNAL( valueChanged(int) ), this, SIGNAL( signalWChanged(int) ) );
}

RadiusSelectorDlg::~RadiusSelectorDlg()
{
}


void EWASiteWidget::slotXChanged(int x)
{
    m_iRX = x;
    repaint();
}

void EWASiteWidget::slotYChanged(int y )
{
    m_iRY = y;
    repaint();
}

void EWASiteWidget::slotWChanged(int w )
{
    m_pGridLayout->setMargin( w );
    repaint();
}

#endif //-- PLAY_WITH_RADIUSES

void EWASiteWidget::showDownloader( QNetworkReply *pReply )
{
    if( pReply )
    {
        getWebView()->showDownloader( pReply );
    }
}

void EWASiteWidget::slotStatusMessage( const QString& msg, int iPeriod )
{
    if( isVisible() )
    {
        m_pStatusBar->setText( msg );
        updateStatusBarGeometry();
        
        m_pStatusCleanTimer->stop();
        
        if( msg.isEmpty() )
        {
            m_pStatusBar->hide();
        }
        else
        {
            if( !m_pStatusBar->isVisible() )
            {
                m_pStatusBar->show();
            }
            
            if( iPeriod <= 0 )
            {
                iPeriod = EWASiteWidget::statusMessageDurationMs();
            }
            
            m_pStatusCleanTimer->start( iPeriod );
        }
    }
}

void EWASiteWidget::slotClearStatus()
{
    m_pStatusBar->clear();
    m_pStatusBar->hide();
    
    m_pStatusCleanTimer->stop();
}

void EWASiteWidget::updateStatusBarGeometry()
{
    QRect msgRect = rect();
    
    QRect statLabelRect = m_pStatusBar->rect();
    statLabelRect.moveBottomLeft( QPoint( msgRect.bottomLeft().x()+ m_dBorderWidth+2, msgRect.bottomLeft().y() - m_dBorderWidth-2 ) );
    
    m_pStatusBar->setMaximumWidth( msgRect.width() );
    
    m_pStatusBar->setGeometry( statLabelRect );
    
}

void EWASiteWidget::scroll2top()
{
    disconnectScrolls();
    
    vScroll()->setValue( 0 );
    hScroll()->setValue( 0 );
    
    connectScrolls();
}

QSize EWASiteWidget::calcPrefferedPageSize() const
{
    int iDeltaY = getCustomizedVerticalOffset();
    iDeltaY += m_dBorderWidth + m_dBorderWidth;
    
    int iDeltaX = m_dBorderWidth + m_dBorderWidth;
    
    QSize res = EWAApplication::getScreenSize().size();
    
    res.setWidth( res.width() - iDeltaX );
    res.setHeight( res.height() - iDeltaY );
    
    return res;
}

void EWASiteWidget::setNewSize( int w, int h )
{
    if( w <= 0 )
    {
        w = width();
    }
    
    if( h <= 0 )
    {
        h = height();
    }
    
    QSize dstSz( w, h );
    if( dstSz != size() )
    {
        resize( w, h );
        setMinimumSize(0,0);
    }
}

void EWASiteWidget::setLocation( const QPoint& pnt ) 
{
    m_locationPnt = pnt;
    
    //-- it should generate moveEvent, in which handler
    //-- the signaSiteWidgetModified should be emited
    //-- if position was changed:
    move( m_locationPnt ); 
}

void EWASiteWidget::setBackButtonShown( bool bShown )
{
    if( bShown != isBackButtonShown() )
    {
        m_pTitleWidget->setBackButtonShown( bShown );
        setTitleBarVisible( false );
        
        emit signaSiteWidgetModified();
    }
}
bool EWASiteWidget::isBackButtonShown() const
{
    return m_pTitleWidget->isBackButtonShown();
}

void EWASiteWidget::setRefreshButtonShown( bool bShown )
{
    if( bShown != isRefreshButtonShown() )
    {
        m_pTitleWidget->setRefreshButtonShown( bShown );
        setTitleBarVisible( false );
        
        emit signaSiteWidgetModified();
    }
}

bool EWASiteWidget::isRefreshButtonShown() const
{
    return m_pTitleWidget->isRefreshButtonShown();
}

void EWASiteWidget::setForwardButtonShown( bool bShown )
{
    if( bShown != isForwardButtonShown() )
    {
        m_pTitleWidget->setForwardButtonShown( bShown );
        setTitleBarVisible( false );
        
        emit signaSiteWidgetModified();
    }
}

bool EWASiteWidget::isForwardButtonShown() const
{
    return m_pTitleWidget->isForwardButtonShown();
}

void EWASiteWidget::setTitleLabelShown( bool bShown )
{
    if( bShown != isTitleLabelShown() )
    {
        m_pTitleWidget->setTitleLabelShown( bShown );
        setTitleBarVisible( false );
           
        emit signaSiteWidgetModified();
    }
}
bool EWASiteWidget::isTitleLabelShown() const
{
    return m_pTitleWidget->isTitleLabelShown();
}
void EWASiteWidget::setStickButtonShown( bool bShown )
{
    if( bShown != isStickButtonShown() )
    {
        m_pTitleWidget->setStickButtonShown( bShown );
        setTitleBarVisible( false );
        
        emit signaSiteWidgetModified();
    }
}

bool EWASiteWidget::isStickButtonShown() const
{
    return m_pTitleWidget->isStickButtonShown();
}

void EWASiteWidget::setViewModeButtonShown( bool bShown )
{
    if( bShown != isViewModeButtonShown() )
    {
        m_pTitleWidget->setViewModeButtonShown( bShown );
        setTitleBarVisible( false );
        
        emit signaSiteWidgetModified();
    }
}

bool EWASiteWidget::isViewModeButtonShown() const
{
    return m_pTitleWidget->isViewModeButtonShown();
}

void EWASiteWidget::setCloseButtonShown( bool bShown )
{
    if( bShown != isCloseButtonShown() )
    {
        m_pTitleWidget->setCloseButtonShown( bShown );
        setTitleBarVisible( false );
        
        emit signaSiteWidgetModified();
    }
}
bool EWASiteWidget::isCloseButtonShown() const
{
    return m_pTitleWidget->isCloseButtonShown();
}

Qt::WindowFlags EWASiteWidget::getFlagsNormal() const
{
    return windowFlags()/*m_iFlagsNorm*/;
}
Qt::WindowFlags EWASiteWidget::getFlagsMaximized() const
{   
    return windowFlags()/*m_iFlagsMax*/;
}

bool EWASiteWidget::closeIfAutoShown()
{
    if( eCurrentHideVewMode == EM_HVM_AUTOHIDE )
    {
        return close();
    }
    return false;
}

bool EWASiteWidget::closeIfOnTop()
{
    if( eCurrentHideVewMode == EM_HVM_ALWAYS_ON_TOP )
    {
        return close();
    }
    return false;
}

bool EWASiteWidget::closeIfOnDesktop()
{
    if( eCurrentHideVewMode == EM_HVM_ALWAYS_ON_DESKTOP )
    {
        return close();
    }
    return false;
}

bool EWASiteWidget::closeByHideViewMode( EWA_EM_HVM mode )
{
    switch( mode )
    {
    case EM_HVM_AUTOHIDE:
        {
            return closeIfAutoShown();
            break;
        }
    case EM_HVM_ALWAYS_ON_TOP:
        {
            return closeIfOnTop();
            break;
        }
    case EM_HVM_ALWAYS_ON_DESKTOP:
        {
            return closeIfOnDesktop();
            break;
        }
    default:
        {
            return false;
            break;
        }
    }
}

QPoint EWASiteWidget::getInverteddPos() const
{
    QSize dp = EWAApplication::desktop()->size();
    return QPoint( m_locationPnt.x() - dp.width() - 16, m_locationPnt.y() - dp.height() - 16 );
}

void EWASiteWidget::loadCurrentMode( EWA_EM_HVM mode )
{
    QAction *pSelectedAct = 0;
    
    switch( mode )
    {
        case EM_HVM_ALWAYS_ON_TOP:
        {
            pSelectedAct = m_pTitleWidget->m_pOnTopAction;
            break;
        }
        case EM_HVM_ALWAYS_ON_DESKTOP:
        {
            pSelectedAct = m_pTitleWidget->m_pOnDesktopAction;
            break;
        }
        default: //-- EM_HVM_AUTOHIDE:
        {
            pSelectedAct = m_pTitleWidget->m_pAutoHidebleAction;
            break;
        }
    }
    
    if( pSelectedAct )
    {
        bool bHideIt = !( isVisible() && wasShown() );
        pSelectedAct->setChecked( true );
        pSelectedAct->trigger();
        //-- it shall call 
        //-- m_pTitleWidget->slotStickButtonActionTriggerd( pSelectedAct );
        //-- and one of the appropriate slots:
        //-- slotSetViewAlwaysOnTop/slotSetViewAlwaysOnDesktop/slotSetViewAutoClose
        //-- with default argument true;
        //-- but now we are on the sites list loading,
        //-- so message shoul be hidden:
        if( bHideIt )
            realClose();
    }
}

QIcon EWASiteWidget::getSticButtonIcon() const
{
    if( m_pTitleWidget )
        return m_pTitleWidget->stickButton()->icon();
    
    return QIcon();
}

void EWASiteWidget::setScrollArea( QScrollArea *pScrollArea )
{
    m_pScrollArea = pScrollArea;
    m_pGridLayout->addWidget( m_pScrollArea, 1, 0 );
}

/*virtual*/ void EWASiteWidget::setWindowFlags( Qt::WindowFlags flags )
{
    QWidget::setWindowFlags( flags );
#ifdef PLAY_WITH_FLAGS
    pFlagsDlg->updateCheckboxes( flags );
#endif //-- PLAY_WITH_FLAGS
}

int EWASiteWidget::getCustomizedVerticalOffset() const
{
    return m_pTitleWidget->height();
}

#ifdef PLAY_WITH_FLAGS

FlagsManagerLialog::FlagsManagerLialog()
{
    previewWindow = 0;

    createTypeGroupBox();
    createHintsGroupBox();

    quitButton = new QPushButton(tr("&Quit"));
    connect(quitButton, SIGNAL(clicked()), qApp, SLOT(quit()));

    QHBoxLayout *bottomLayout = new QHBoxLayout;
    bottomLayout->addStretch();
    bottomLayout->addWidget(quitButton);

    QVBoxLayout *mainLayout = new QVBoxLayout;
    mainLayout->addWidget(typeGroupBox);
    mainLayout->addWidget(hintsGroupBox);
    mainLayout->addLayout(bottomLayout);
    setLayout(mainLayout);

    setWindowTitle(tr("Window Flags"));
    updatePreview();
}

void FlagsManagerLialog::updateCheckboxes( Qt::WindowFlags flags )
{
    if(!previewWindow)
        return;
        
    QList<QCheckBox *> cbs = qFindChildren<QCheckBox *>(this);
    foreach (QCheckBox *cb, cbs)
    {
        disconnect(cb, SIGNAL(clicked()), this, SLOT(updatePreview()));
    }
    QList<QRadioButton *> rbs = qFindChildren<QRadioButton *>(this);
    foreach (QRadioButton* rb, rbs)
    {
        disconnect(rb, SIGNAL(clicked()), this, SLOT(updatePreview()));
    }
    
    previewWindow->show();
    qApp->processEvents();
    
    Qt::WindowFlags type = 0;
    if( flags == 0 )
        flags = previewWindow->windowFlags();
    
    type = ( flags & Qt::WindowType_Mask);
        
    windowRadioButton->setChecked( (type == Qt::Window) );
    dialogRadioButton->setChecked( (type == Qt::Dialog) );
    sheetRadioButton->setChecked( (type == Qt::Sheet) );
    drawerRadioButton->setChecked( (type == Qt::Drawer) );
    popupRadioButton->setChecked( (type == Qt::Popup) );
    toolRadioButton->setChecked( (type == Qt::Tool) );
    toolTipRadioButton->setChecked( (type == Qt::ToolTip) );
    splashScreenRadioButton->setChecked( (type == Qt::SplashScreen) );

    
    msWindowsFixedSizeDialogCheckBox->setChecked( (flags & Qt::MSWindowsFixedSizeDialogHint) );
    x11BypassWindowManagerCheckBox->setChecked( (flags & Qt::X11BypassWindowManagerHint) );
    framelessWindowCheckBox->setChecked( (flags & Qt::FramelessWindowHint) );
    windowTitleCheckBox->setChecked( (flags & Qt::WindowTitleHint) );
    windowSystemMenuCheckBox->setChecked( (flags & Qt::WindowSystemMenuHint) );
    windowMinimizeButtonCheckBox->setChecked( (flags & Qt::WindowMinimizeButtonHint) );
    windowMaximizeButtonCheckBox->setChecked( (flags & Qt::WindowMaximizeButtonHint) );
    windowCloseButtonCheckBox->setChecked( (flags & Qt::WindowCloseButtonHint) );
    windowContextHelpButtonCheckBox->setChecked( (flags & Qt::WindowContextHelpButtonHint) );
    windowShadeButtonCheckBox->setChecked( (flags & Qt::WindowShadeButtonHint) );
    windowStaysOnTopCheckBox->setChecked( (flags & Qt::WindowStaysOnTopHint) );
    windowStaysOnBottomCheckBox->setChecked( (flags & Qt::WindowStaysOnBottomHint) );
    customizeWindowHintCheckBox->setChecked( (flags & Qt::CustomizeWindowHint) );
        
    //QList<QCheckBox *> cbs = qFindChildren<QCheckBox *>(this);
    foreach (QCheckBox *cb, cbs)
    {
        connect(cb, SIGNAL(clicked()), this, SLOT(updatePreview()));
    }
    //QList<QRadioButton *> rbs = qFindChildren<QCheckBox *>(this);
    foreach (QRadioButton* rb, rbs)
    {
        connect(rb, SIGNAL(clicked()), this, SLOT(updatePreview()));
    }
}

void FlagsManagerLialog::updatePreview()
{
    if(!previewWindow)
        return;
        
    Qt::WindowFlags flags = 0;

    if (windowRadioButton->isChecked()) {
        flags = Qt::Window;
    } else if (dialogRadioButton->isChecked()) {
        flags = Qt::Dialog;
    } else if (sheetRadioButton->isChecked()) {
        flags = Qt::Sheet;
    } else if (drawerRadioButton->isChecked()) {
        flags = Qt::Drawer;
    } else if (popupRadioButton->isChecked()) {
        flags = Qt::Popup;
    } else if (toolRadioButton->isChecked()) {
        flags = Qt::Tool;
    } else if (toolTipRadioButton->isChecked()) {
        flags = Qt::ToolTip;
    } else if (splashScreenRadioButton->isChecked()) {
        flags = Qt::SplashScreen;
    }

    if (msWindowsFixedSizeDialogCheckBox->isChecked())
        flags |= Qt::MSWindowsFixedSizeDialogHint;
    if (x11BypassWindowManagerCheckBox->isChecked())
        flags |= Qt::X11BypassWindowManagerHint;
    if (framelessWindowCheckBox->isChecked())
        flags |= Qt::FramelessWindowHint;
    if (windowTitleCheckBox->isChecked())
        flags |= Qt::WindowTitleHint;
    if (windowSystemMenuCheckBox->isChecked())
        flags |= Qt::WindowSystemMenuHint;
    if (windowMinimizeButtonCheckBox->isChecked())
        flags |= Qt::WindowMinimizeButtonHint;
    if (windowMaximizeButtonCheckBox->isChecked())
        flags |= Qt::WindowMaximizeButtonHint;
    if (windowCloseButtonCheckBox->isChecked())
        flags |= Qt::WindowCloseButtonHint;
    if (windowContextHelpButtonCheckBox->isChecked())
        flags |= Qt::WindowContextHelpButtonHint;
    if (windowShadeButtonCheckBox->isChecked())
        flags |= Qt::WindowShadeButtonHint;
    if (windowStaysOnTopCheckBox->isChecked())
        flags |= Qt::WindowStaysOnTopHint;
    if (windowStaysOnBottomCheckBox->isChecked())
        flags |= Qt::WindowStaysOnBottomHint;
    if (customizeWindowHintCheckBox->isChecked())
        flags |= Qt::CustomizeWindowHint;

    
    previewWindow->setWindowFlags(flags);

    QPoint pos = previewWindow->pos();
    if (pos.x() < 0)
        pos.setX(0);
    if (pos.y() < 0)
        pos.setY(0);
    previewWindow->move(pos);
    previewWindow->show();
}

void FlagsManagerLialog::createTypeGroupBox()
{
    typeGroupBox = new QGroupBox(tr("Type"));

    windowRadioButton = createRadioButton(tr("Window"));
    dialogRadioButton = createRadioButton(tr("Dialog"));
    sheetRadioButton = createRadioButton(tr("Sheet"));
    drawerRadioButton = createRadioButton(tr("Drawer"));
    popupRadioButton = createRadioButton(tr("Popup"));
    toolRadioButton = createRadioButton(tr("Tool"));
    toolTipRadioButton = createRadioButton(tr("Tooltip"));
    splashScreenRadioButton = createRadioButton(tr("Splash screen"));
    windowRadioButton->setChecked(true);

    QGridLayout *layout = new QGridLayout;
    layout->addWidget(windowRadioButton, 0, 0);
    layout->addWidget(dialogRadioButton, 1, 0);
    layout->addWidget(sheetRadioButton, 2, 0);
    layout->addWidget(drawerRadioButton, 3, 0);
    layout->addWidget(popupRadioButton, 0, 1);
    layout->addWidget(toolRadioButton, 1, 1);
    layout->addWidget(toolTipRadioButton, 2, 1);
    layout->addWidget(splashScreenRadioButton, 3, 1);
    typeGroupBox->setLayout(layout);
}

void FlagsManagerLialog::createHintsGroupBox()
{
    hintsGroupBox = new QGroupBox(tr("Hints"));

    msWindowsFixedSizeDialogCheckBox =
            createCheckBox(tr("MS Windows fixed size dialog"));
    x11BypassWindowManagerCheckBox =
            createCheckBox(tr("X11 bypass window manager"));
    framelessWindowCheckBox = createCheckBox(tr("Frameless window"));
    windowTitleCheckBox = createCheckBox(tr("Window title"));
    windowSystemMenuCheckBox = createCheckBox(tr("Window system menu"));
    windowMinimizeButtonCheckBox = createCheckBox(tr("Window minimize button"));
    windowMaximizeButtonCheckBox = createCheckBox(tr("Window maximize button"));
    windowCloseButtonCheckBox = createCheckBox(tr("Window close button"));
    windowContextHelpButtonCheckBox =
            createCheckBox(tr("Window context help button"));
    windowShadeButtonCheckBox = createCheckBox(tr("Window shade button"));
    windowStaysOnTopCheckBox = createCheckBox(tr("Window stays on top"));
    windowStaysOnBottomCheckBox = createCheckBox(tr("Window stays on bottom"));
    customizeWindowHintCheckBox= createCheckBox(tr("Customize window"));

    QGridLayout *layout = new QGridLayout;
    layout->addWidget(msWindowsFixedSizeDialogCheckBox, 0, 0);
    layout->addWidget(x11BypassWindowManagerCheckBox, 1, 0);
    layout->addWidget(framelessWindowCheckBox, 2, 0);
    layout->addWidget(windowTitleCheckBox, 3, 0);
    layout->addWidget(windowSystemMenuCheckBox, 4, 0);
    layout->addWidget(windowMinimizeButtonCheckBox, 0, 1);
    layout->addWidget(windowMaximizeButtonCheckBox, 1, 1);
    layout->addWidget(windowCloseButtonCheckBox, 2, 1);
    layout->addWidget(windowContextHelpButtonCheckBox, 3, 1);
    layout->addWidget(windowShadeButtonCheckBox, 4, 1);
    layout->addWidget(windowStaysOnTopCheckBox, 5, 1);
    layout->addWidget(windowStaysOnBottomCheckBox, 6, 1);
    layout->addWidget(customizeWindowHintCheckBox, 5, 0);
    hintsGroupBox->setLayout(layout);
}

QCheckBox *FlagsManagerLialog::createCheckBox(const QString &text)
{
    QCheckBox *checkBox = new QCheckBox(text);
    connect(checkBox, SIGNAL(clicked()), this, SLOT(updatePreview()));
    return checkBox;
}

QRadioButton *FlagsManagerLialog::createRadioButton(const QString &text)
{
    QRadioButton *button = new QRadioButton(text);
    connect(button, SIGNAL(clicked()), this, SLOT(updatePreview()));
    return button;
}

#endif //-- PLAY_WITH_FLAGS
