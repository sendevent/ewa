/*******************************************************************************
**
** file: ewa_sitewidgettitlewidget.cpp
**
** class: EWASiteWidgetTitleWidget
**
** description:
** Widget which handle space for EWASiteWidgetLabel and controll buttons.
** 01.07.2009
**
** sendevent@gmail.com
**
*******************************************************************************/

#include "ewa_sitewidgettitlewidget.h"
#include "ewa_sitewidget.h"
#include "ewa_application.h"
#include "ewa_sitehandle.h"
#include "ewa_timer.h"

#include <QPainter>
#include <QPaintEvent>
#include <QActionGroup>
#include <QMenu>

const QString INITIATE_IN_CHANGEEVENT;

#define EWA_LOCAL_STRING( x ) EWAApplication::translate( this, x.toUtf8().data() )


EWASiteWidgetTitleWidget::EWASiteWidgetTitleWidget( QWidget *parent )
:QWidget( parent )
,m_sitePtr( 0 )
,m_pSiteWidget( 0 )
,m_bViewModeButtonShown( false )
,m_bRefreshButtonShown( true )
,m_bForwardButtonShown( false )
,m_bCloseButtonShown( false )
,m_bStickButtonShown( false )
,m_bBackButtonShown( false )
,m_bTitleLabelShown( false )
,m_bMsgIsPreview( false )
{
    m_pParent = parent;
    ui.setupUi( this );
    
    setMaximumHeight( ui.refreshButton->height() );

    connect( ui.label, SIGNAL( signalTooltipChanged(const QString&) ),
        this, SLOT( slotChangeTooltip(const QString&) ) );
    
    m_pWindowFlagsMenu = new QMenu( 0 );
    
    m_pActsGroup = new QActionGroup( this );
    
    m_pOnTopAction = m_pActsGroup->addAction( QIcon( ":/images/stick.png" ), INITIATE_IN_CHANGEEVENT );//tr( "Stay on Top" ) );
    m_pOnTopAction->setCheckable( true );
    m_pWindowFlagsMenu->addAction( m_pOnTopAction );
    
    m_pOnDesktopAction = m_pActsGroup->addAction( QIcon( ":/images/unstick.png" ), INITIATE_IN_CHANGEEVENT );//tr( "Stay on Desktop" ) );
    m_pOnDesktopAction->setCheckable( true );
    m_pWindowFlagsMenu->addAction( m_pOnDesktopAction );
    
    m_pAutoHidebleAction = m_pActsGroup->addAction( QIcon( ":/images/hideble.png" ), INITIATE_IN_CHANGEEVENT );//tr( "Auto Hide" ) );
    m_pAutoHidebleAction->setCheckable( true );
    m_pWindowFlagsMenu->addAction( m_pAutoHidebleAction );
    
    QAction *pSeparator = new QAction( this );
    pSeparator->setSeparator( true );
    m_pWindowFlagsMenu->addAction( pSeparator );

    m_pActsGroup->addAction( pSeparator );
    m_pNeedEditAction = m_pActsGroup->addAction( QIcon( ":/images/configure.png" ), INITIATE_IN_CHANGEEVENT );//tr( "Settings" ) );

    m_pWindowFlagsMenu->addAction( m_pNeedEditAction );    

    stickButton()->setMenu( m_pWindowFlagsMenu );

    retranslate();
    
    connect( m_pActsGroup, SIGNAL( triggered(QAction*) ),
        this, SLOT( slotStickButtonActionTriggerd(QAction*) ) );

    connect( m_pOnTopAction, SIGNAL( triggered() ),
        this, SIGNAL( signalNeedAlwaysOnTop() ) );
    connect( m_pOnDesktopAction, SIGNAL( triggered() ),
        this, SIGNAL( signalNeedAlwaysOnDesktop() ) );
    connect( m_pAutoHidebleAction, SIGNAL( triggered() ),
        this, SIGNAL( signalNeedAutoclose() ) );
    connect( m_pNeedEditAction, SIGNAL( triggered() ),
        this, SIGNAL( signalNeedShowEditor() ) );
}

EWASiteWidgetTitleWidget::~EWASiteWidgetTitleWidget()
{
    delete m_pWindowFlagsMenu;
    m_pWindowFlagsMenu = 0;
}

bool EWASiteWidgetTitleWidget::isMenuVisible() const
{
    return m_pWindowFlagsMenu->isVisible();
}

void EWASiteWidgetTitleWidget::slotChangeTooltip( const QString& tooltip )
{
    this->setToolTip( tooltip );
}

void EWASiteWidgetTitleWidget::setSite( EWASiteHandle *pSite )
{
    m_sitePtr = pSite;
    ui.label->setSite( m_sitePtr );
    
    connect( m_sitePtr, SIGNAL( signalFaviconChanged() ),
        this, SLOT( slotFaviconSiteChanged() ) );
}

void EWASiteWidgetTitleWidget::updateGeomtery( const QRect& maxRect )
{
    QRect myRect = rect();
    QRect parentRect = maxRect;
    myRect.setWidth( parentRect.width()-16 );
    QPoint myTopLeft = maxRect.topLeft();
    myTopLeft.setX( myTopLeft.x()+8 );

    if( !EWAApplication::settings()->m_pGroupApp->useRichWM() )
        myTopLeft.setY( myTopLeft.y()+4 );

    myRect.moveTopLeft( myTopLeft );
    setGeometry( myRect );
}

void EWASiteWidgetTitleWidget::resizeEvent( QResizeEvent *event )
{
    QWidget::resizeEvent( event );
}

void EWASiteWidgetTitleWidget::slotFaviconSiteChanged()
{
    ui.refreshButton->setIcon( QIcon( m_sitePtr->getFavicoPixmap().scaled( 16,16 ) ) );
}

void EWASiteWidgetTitleWidget::slotStickButtonActionTriggerd( QAction* act )
{
    if( act && act != m_pNeedEditAction )
    {
        stickButton()->setIcon( act->icon() );
    }
}

void EWASiteWidgetTitleWidget::on_refreshButton_clicked()
{
    if( isNotPreview() )
    {
        emit signalNeedRefresh();
    }
}

void EWASiteWidgetTitleWidget::on_backButton_clicked()
{
    if( isNotPreview() )
    {
        emit signalNeedGoBack();
    }
}

void EWASiteWidgetTitleWidget::on_forwardButton_clicked()
{
    if( isNotPreview() )
    {
        emit signalNeedGoForward();
    }
}

void EWASiteWidgetTitleWidget::on_viewModeButton_toggled( bool on )
{
    if( isNotPreview() )
    {
        if( on )
        {
            viewModeButton()->setIcon( QIcon( QLatin1String( ":/images/small.png" ) ) ) ;
            emit signalNeedShowMaximized();
        }
        else
        {
            viewModeButton()->setIcon( QIcon( QLatin1String( ":/images/big.png" ) ) ) ;
            emit signalNeedShowNormal();
        }
    }
}

void EWASiteWidgetTitleWidget::on_closeButton_clicked()
{
    if( isNotPreview() )
    {
        emit signalNeedClose();
    }
}

void EWASiteWidgetTitleWidget::on_stickButton_clicked()
{
    QAction *pCheckedAction = m_pActsGroup->checkedAction();
    if( isNotPreview() )
    {
        QAction *pRiseAction = ( pCheckedAction == m_pOnTopAction ) ? m_pAutoHidebleAction : m_pOnTopAction;
        pRiseAction->trigger();
    }
}

void EWASiteWidgetTitleWidget::showControlls()
{
    QList<QWidget*> widgets = qFindChildren<QWidget*>(this);
    foreach (QWidget *w, widgets)
        w->show();
}

void EWASiteWidgetTitleWidget::hideControlls()
{
    backButton()->setVisible( isBackButtonShown() );
    refreshButton()->setVisible( isRefreshButtonShown() );
    forwardButton()->setVisible( isForwardButtonShown() );
    label()->setVisible( isTitleLabelShown() );
    stickButton()->setVisible( isStickButtonShown() );
    viewModeButton()->setVisible( isViewModeButtonShown() );
    closeButton()->setVisible( isCloseButtonShown() );
}

void EWASiteWidgetTitleWidget::setIsOnPreview( bool bOnPreview ) 
{ 
    m_bMsgIsPreview = bOnPreview; 
    m_pActsGroup->setDisabled( bOnPreview );
}

bool EWASiteWidgetTitleWidget::isNotPreview()
{
    return !m_bMsgIsPreview;
}

void EWASiteWidgetTitleWidget::retranslate()
{
    ui.retranslateUi( this );

    m_pOnTopAction->setText( EWA_LOCAL_STRING( tr( "Stay on Top" ) ) );
    m_pOnDesktopAction->setText( EWA_LOCAL_STRING( tr( "Stay on Desktop" ) ) );
    m_pAutoHidebleAction->setText( EWA_LOCAL_STRING( tr( "Auto Hide" ) ) );
    m_pNeedEditAction->setText( EWA_LOCAL_STRING( tr( "Settings" ) ) );
}

void EWASiteWidgetTitleWidget::changeEvent( QEvent *event )
{
    if( event->type() == QEvent::LanguageChange )
    {
        retranslate();
    }

    return QWidget::changeEvent( event );
}
