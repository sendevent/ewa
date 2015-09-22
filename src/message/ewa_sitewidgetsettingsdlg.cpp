/*******************************************************************************
**
** file: ewa_sitewidgetsettingsdlg.cpp
**
** class: EWASiteWidgetSettingsDlg
**
** description:
** Settings dialog for "enhanced" ( internal - EWASiteWidget ) messages.
**
** 23.03.2009
**
** sendevent@gmail.com
**
*******************************************************************************/

#include "ewa_sitewidgetsettingsdlg.h"
#include "ewa_sitehandle.h"

#include <QScrollBar>

EWASiteWidgetSettingsDlg::EWASiteWidgetSettingsDlg( QWidget *parent )
:QWidget( parent )
,m_sitePtr(0)
,b1stResizeW(true)
,b1stResizeH(true)
{
    ui.setupUi( this );
    init();
}

void EWASiteWidgetSettingsDlg::connectSiteWidget()
{
    if( m_sitePtr && m_sitePtr->getWidget() )
    {
        connect( m_sitePtr->getWidget(), SIGNAL( signalScrolled(const QPoint&) ),
            this, SLOT( slotWebViewScrolled(const QPoint&) ) );

        connect( m_sitePtr->getWidget(), SIGNAL( signalGeometryChanged(const QRect&) ),
            this, SLOT( slotSiteWidgetGeometryChanged(const QRect&) ) );
    }
}

void EWASiteWidgetSettingsDlg::disconnectSiteWidget()
{
    if( m_sitePtr && m_sitePtr->getWidget() )
    {
        disconnect( m_sitePtr->getWidget(), SIGNAL( signalScrolled(const QPoint&) ),
            this, SLOT( slotWebViewScrolled(const QPoint&) ) );

        disconnect( m_sitePtr->getWidget(), SIGNAL( signalGeometryChanged(const QRect&) ),
            this, SLOT( slotSiteWidgetGeometryChanged(const QRect&) ) );
    }
}
        
void EWASiteWidgetSettingsDlg::init()
{
    m_bSiteReady = false;

    if( m_sitePtr )
    {
        disconnectSiteWidget();
        
        disconnect( ui.msgXSpinBox, SIGNAL( valueChanged(int) ),
            this, SLOT( slotMoveSelection(int) ) );
        disconnect( ui.msgYSpinBox, SIGNAL( valueChanged(int) ),
            this, SLOT( slotMoveSelection(int) ) );
            
        disconnect( m_sitePtr->getWebPage(), SIGNAL( contentsSizeChanged(const QSize&) ),
            this, SLOT(slotWebPageContentsSizeChanged(const QSize&)) );

        updateControlls();
    }

    m_sitePtr = 0;

    m_strContentsSizeLabelTemplate = tr( "Contents size: %1" );

    int iMin = 0;
    int iMax = 65536;
    ui.widthSpinBox->setRange( iMin, iMax );
    ui.heigtSpinBox->setRange( iMin, iMax );
    ui.scrXSpinBox->setRange( iMin, iMax );
    ui.scrYSpinBox->setRange( iMin, iMax );
    ui.pageWidthSpinBox->setRange( iMin, iMax );
    ui.pageHeightSpinBox->setRange( iMin, iMax );
    ui.msgXSpinBox->setRange( iMin, iMax );
    ui.msgYSpinBox->setRange( iMin, iMax );

    //-- update contents size's label text:
    slotWebPageContentsSizeChanged();
}

EWASiteWidgetSettingsDlg::~EWASiteWidgetSettingsDlg()
{
    m_sitePtr = 0;
}

void EWASiteWidgetSettingsDlg::initCombos()
{
    QPoint screensRange = EWAApplication::getScreensRange();

    ui.widthSpinBox->setRange( 0, screensRange.x() );
    ui.heigtSpinBox->setRange( 0, screensRange.x() );

    ui.scrXSpinBox->setRange( -1*screensRange.x(), screensRange.x() );
    ui.scrYSpinBox->setRange( -1*screensRange.y(), screensRange.y() );
}

void EWASiteWidgetSettingsDlg::setSitePtr( EWASiteHandle *ptr )
{
    initCombos();
    
    m_sitePtr = ptr;
    if( !m_sitePtr )
    {
        setEnabled( false );
        return;
    }
    
    setEnabled( true );
    
    b1stResizeW = b1stResizeH = true;

    connectSiteWidget();
    
    connect( ui.msgXSpinBox, SIGNAL( valueChanged(int) ),
        this, SLOT( slotMoveSelection(int) ) );
    connect( ui.msgYSpinBox, SIGNAL( valueChanged(int) ),
        this, SLOT( slotMoveSelection(int) ) );

    connect( m_sitePtr->getWebPage(), SIGNAL( contentsSizeChanged(const QSize&) ),
        this, SLOT(slotWebPageContentsSizeChanged(const QSize&) ) );

    updateControlls();

    ui.pXMsgColorManager->setSite( m_sitePtr );

    
    
    //-- update contents size's label text:
    slotWebPageContentsSizeChanged();

    
    m_bSiteReady = true;
    
    ui.pageWidthSpinBox->setValue( ui.pageWidthSpinBox->value()+1 );
    ui.pageWidthSpinBox->setValue( ui.pageWidthSpinBox->value()-1 );

    ui.backCheckBox->setChecked( m_sitePtr->getWidget()->isBackButtonShown() );
    ui.refreshCheckBox->setIcon( m_sitePtr->getFavicoPixmap() );
    ui.refreshCheckBox->setChecked( m_sitePtr->getWidget()->isRefreshButtonShown() );
    ui.forwardCheckBox->setChecked( m_sitePtr->getWidget()->isForwardButtonShown() );
    ui.titleCheckBox->setChecked( m_sitePtr->getWidget()->isTitleLabelShown() );
    ui.stickCheckBox->setChecked( m_sitePtr->getWidget()->isStickButtonShown() );
    ui.stickCheckBox->setIcon( m_sitePtr->getWidget()->getSticButtonIcon() );
    ui.maxminCheckBox->setChecked( m_sitePtr->getWidget()->isViewModeButtonShown() );
    ui.closeCheckBox->setChecked( m_sitePtr->getWidget()->isCloseButtonShown() );
}

void EWASiteWidgetSettingsDlg::resetSite()
{
    if( m_sitePtr )
    {
        disconnectSiteWidget();
        
        disconnect( ui.msgXSpinBox, SIGNAL( valueChanged(int) ),
            this, SLOT( slotMoveSelection(int) ) );
        disconnect( ui.msgYSpinBox, SIGNAL( valueChanged(int) ),
            this, SLOT( slotMoveSelection(int) ) );

        disconnect( m_sitePtr->getWebPage(), SIGNAL( contentsSizeChanged(const QSize&) ),
            this, SLOT(slotWebPageContentsSizeChanged(const QSize&)) );
    }
    
    ui.pXMsgColorManager->resetSite();
    
    m_sitePtr = 0;
    m_bSiteReady = false;
}

void EWASiteWidgetSettingsDlg::on_widthSpinBox_valueChanged( int val )
{
    if( m_sitePtr && m_bSiteReady )
    {
        disconnectSiteWidget();
        m_sitePtr->getWidget()->setNewSize( val, -1 );
        if( b1stResizeW )
        {
            m_sitePtr->getWidget()->setNewSize( val, -1 );
            b1stResizeW = false;
        }
        connectSiteWidget();
    }
}

void EWASiteWidgetSettingsDlg::on_heigtSpinBox_valueChanged( int val )
{
    if( m_sitePtr && m_bSiteReady )
    {
        disconnectSiteWidget();
        m_sitePtr->getWidget()->setNewSize( -1, val );
        if( b1stResizeH )
        {
            m_sitePtr->getWidget()->setNewSize( -1, val );
            b1stResizeH = false;
        }
        connectSiteWidget();
    }
}


void EWASiteWidgetSettingsDlg::on_scrXSpinBox_valueChanged( int val )
{
    if( m_sitePtr && m_bSiteReady )
    {
        disconnectSiteWidget();
        m_sitePtr->getWidget()->setLocation( QPoint( val, ui.scrYSpinBox->value() ) );
        updateControlls();
        connectSiteWidget();
    }
}

void EWASiteWidgetSettingsDlg::on_scrYSpinBox_valueChanged( int val )
{
    if( m_sitePtr && m_bSiteReady )
    {
        disconnectSiteWidget();
        m_sitePtr->getWidget()->setLocation( QPoint( ui.scrXSpinBox->value(), val ) );
        updateControlls();
        connectSiteWidget();
    }
}

void EWASiteWidgetSettingsDlg::slotWebViewScrolled( const QPoint& pt )
{
    if( !m_sitePtr
    || ( pt.x() == ui.msgXSpinBox->value() && pt.y() == ui.msgYSpinBox->value() ) )
        return;

    disconnect( ui.msgXSpinBox, SIGNAL( valueChanged(int) ),
        this, SLOT( slotMoveSelection(int) ) );
    disconnect( ui.msgYSpinBox, SIGNAL( valueChanged(int) ),
        this, SLOT( slotMoveSelection(int) ) );

    ui.msgXSpinBox->setValue( pt.x() );
    ui.msgYSpinBox->setValue( pt.y() );

    m_sitePtr->setPageScroll( pt );

    connect( ui.msgXSpinBox, SIGNAL( valueChanged(int) ),
        this, SLOT( slotMoveSelection(int) ) );
    connect( ui.msgYSpinBox, SIGNAL( valueChanged(int) ),
        this, SLOT( slotMoveSelection(int) ) );
}

void EWASiteWidgetSettingsDlg::slotMoveSelection( int )
{
    if( !m_sitePtr )
        return;

    QPoint newScroll( ui.msgXSpinBox->value(), ui.msgYSpinBox->value() );

    disconnect( m_sitePtr->getWidget(), SIGNAL( signalScrolled(const QPoint&) ),
            this, SLOT( slotWebViewScrolled(const QPoint&) ) );

    m_sitePtr->setPageScroll( newScroll );

    connect( m_sitePtr->getWidget(), SIGNAL( signalScrolled(const QPoint&) ),
            this, SLOT( slotWebViewScrolled(const QPoint&) ) );
}

void EWASiteWidgetSettingsDlg::selectionChangedByMove( const QPoint& topLeft )
{

    m_sitePtr->setPageScroll( topLeft );
}

void EWASiteWidgetSettingsDlg::on_pageWidthSpinBox_valueChanged( int v )
{
    if( !m_bSiteReady  )
    {
        return;
    }
    
    int prevX = ui.msgXSpinBox->value();
    ui.msgXSpinBox->setRange( 0, v - m_sitePtr->getWidget()->getVisibleWebViewSize().width() );
    //ui.msgXSpinBox->setRange( 0, m_sitePtr->getWidget()->scrollArea()->horizontalScrollBar()->maximum() + m_sitePtr->getWidget()->scrollArea()->verticalScrollBar()->width() );
    ui.msgXSpinBox->setToolTip( tr( "0 - %1" ).arg( ui.msgXSpinBox->maximum() ) );
    
    ui.msgXSpinBox->setValue( prevX );
    
    
    disconnectSiteWidget();
    QSize sz( v, ui.pageHeightSpinBox->value() );
    
    m_sitePtr->setWebPageBaseSize( sz );
    updateControlls();
    
    connectSiteWidget();
}

void EWASiteWidgetSettingsDlg::on_pageHeightSpinBox_valueChanged( int v )
{
    if( !m_bSiteReady )
    {
        return;
    }
    
    QSize sz( ui.pageWidthSpinBox->value(), v );
    
    m_sitePtr->setWebPageBaseSize( sz );
    
    int prevY = ui.msgYSpinBox->value();
    
    ui.msgYSpinBox->setRange( 0, v  - m_sitePtr->getWidget()->getVisibleWebViewSize().height() );
    //ui.msgXSpinBox->setRange( 0, m_sitePtr->getWidget()->scrollArea()->verticalScrollBar()->maximum() + m_sitePtr->getWidget()->scrollArea()->horizontalScrollBar()->height() );
    ui.msgYSpinBox->setToolTip( tr( "0 - %1" ).arg( ui.msgYSpinBox->maximum() ) );
    
    ui.msgYSpinBox->setValue( prevY );
    
    updateControlls();
}

void EWASiteWidgetSettingsDlg::updateSiteWidgetPropertiesFromSite()
{
    if( !m_sitePtr )
    {
        return;
    }
    int x = m_sitePtr->getMsgLocation().x();
    int y = m_sitePtr->getMsgLocation().y();

    ui.scrXSpinBox->setValue( x );
    ui.scrYSpinBox->setValue( y );

    int w = m_sitePtr->getMsgWidth();
    ui.widthSpinBox->setValue( w );
    int h = m_sitePtr->getMsgHeight();
    ui.heigtSpinBox->setValue( h );
    
    QPoint screensRange = EWAApplication::getScreensRange();

    ui.widthSpinBox->setRange( 0, screensRange.x() );
    ui.heigtSpinBox->setRange( 0, screensRange.y() );
    ui.scrXSpinBox->setRange( -1*screensRange.x(), screensRange.x()-w );
    ui.scrYSpinBox->setRange( -1*screensRange.y(), screensRange.y()-h );
    
    ui.widthSpinBox->setToolTip( tr( "%1 - %2" )
        .arg( ui.widthSpinBox->minimum() ).arg( ui.widthSpinBox->maximum() ) );
    
    ui.heigtSpinBox->setToolTip( tr( "%1 - %2" )
        .arg( ui.heigtSpinBox->minimum() ).arg( ui.heigtSpinBox->maximum() ) );
    
    ui.scrXSpinBox->setToolTip( tr( "%1 - %2" )
        .arg( ui.scrXSpinBox->minimum() ).arg( ui.scrXSpinBox->maximum() ) );
    
    ui.scrYSpinBox->setToolTip( tr( "%1 - %2" )
        .arg( ui.scrYSpinBox->minimum() ).arg( ui.scrYSpinBox->maximum() ) );
}

void EWASiteWidgetSettingsDlg::updateWebPagePropertiesFromSite()
{
    if( !m_sitePtr )
    {
        return;
    }

    //QPoint screensRange = EWAApplication::getScreensRange();

    ui.pageWidthSpinBox->setRange( 48, 65536 );
    ui.pageHeightSpinBox->setRange( 48, 65536 );

    int w = m_sitePtr->getWebPageBaseSize().width();
    int h = m_sitePtr->getWebPageBaseSize().height();
    
    ui.pageWidthSpinBox->setValue( w );
    ui.pageWidthSpinBox->setToolTip( tr( "%1 - %2" ).arg( ui.pageWidthSpinBox->minimum() ).arg( ui.pageWidthSpinBox->maximum() ) );
    
    ui.pageHeightSpinBox->setValue( h );
    ui.pageHeightSpinBox->setToolTip( tr( "%1 - %2" ).arg( ui.pageHeightSpinBox->minimum() ).arg( ui.pageHeightSpinBox->maximum() ) );
    
    
    ui.msgXSpinBox->setRange( 0, w- m_sitePtr->getWidget()->getVisibleWebViewSize().width() );
    ui.msgXSpinBox->setToolTip( tr( "%1 - %2" ).arg( ui.msgXSpinBox->minimum() ).arg( ui.msgXSpinBox->maximum() ) );
    
    ui.msgYSpinBox->setRange( 0, h - m_sitePtr->getWidget()->getVisibleWebViewSize().height() );
    ui.msgYSpinBox->setToolTip( tr( "%1 - %2" ).arg( ui.msgYSpinBox->minimum() ).arg( ui.msgYSpinBox->maximum() ) );
    
    int x = m_sitePtr->getPageScroll().x();
    int y = m_sitePtr->getPageScroll().y();
    if( x != ui.msgXSpinBox->value() )
    {
        ui.msgXSpinBox->setValue( x );
    }

    if( y != ui.msgYSpinBox->value() )
    {
        ui.msgYSpinBox->setValue( y );
    }
}

void EWASiteWidgetSettingsDlg::updateControlls()
{
    updateSiteWidgetPropertiesFromSite();
    updateWebPagePropertiesFromSite();
}

void EWASiteWidgetSettingsDlg::slotSiteWidgetGeometryChanged( const QRect& newGeometry )
{
    int newX = newGeometry.x();
    int newY = newGeometry.y();

    int newW = newGeometry.width();
    int newH = newGeometry.height();

    ui.scrXSpinBox->setValue( newX );
    ui.scrYSpinBox->setValue( newY );
    ui.widthSpinBox->setValue( newW );
    ui.heigtSpinBox->setValue( newH );
}

void EWASiteWidgetSettingsDlg::slotWebPageContentsSizeChanged(const QSize& recievedSz )
{
    QSize sz( recievedSz );
    bool bEnabled = false;
    QString strSize = tr( "Uncknown yet" );
    
    if( sz.isEmpty() && m_sitePtr && m_sitePtr->getWebPage() )
    {
        sz = m_sitePtr->getWebPage()->preferredContentsSize();

        if( !sz.isEmpty() )
        {
            strSize = tr( "<b>%1x%2</b>" ).arg( sz.width() ).arg( sz.height() );
            bEnabled = true;
        }
    }

    ui.contentsSizeLabel->setText( m_strContentsSizeLabelTemplate.arg( strSize ) );
    ui.adjustSizeButton->setEnabled( bEnabled );
}

void EWASiteWidgetSettingsDlg::on_adjustSizeButton_clicked()
{
    if( m_sitePtr && m_sitePtr->getWebPage() )
    {
        QSize sz = m_sitePtr->getWebPage()->getContentsSize();
        if( !sz.isEmpty() )
        {
            ui.pageWidthSpinBox->setValue( sz.width() );
            ui.pageHeightSpinBox->setValue( sz.height() );
        }
    }
}

void EWASiteWidgetSettingsDlg::on_backCheckBox_toggled( bool on )
{
    m_sitePtr->getWidget()->setBackButtonShown( on );
}

void EWASiteWidgetSettingsDlg::on_refreshCheckBox_toggled( bool on )
{
    m_sitePtr->getWidget()->setRefreshButtonShown( on );
}

void EWASiteWidgetSettingsDlg::on_forwardCheckBox_toggled( bool on )
{
    m_sitePtr->getWidget()->setForwardButtonShown( on );
}

void EWASiteWidgetSettingsDlg::on_titleCheckBox_toggled( bool on )
{
    m_sitePtr->getWidget()->setTitleLabelShown( on );
}

void EWASiteWidgetSettingsDlg::on_stickCheckBox_toggled( bool on )
{
    m_sitePtr->getWidget()->setStickButtonShown( on );
}

void EWASiteWidgetSettingsDlg::on_maxminCheckBox_toggled( bool on )
{
    m_sitePtr->getWidget()->setViewModeButtonShown( on );
}

void EWASiteWidgetSettingsDlg::on_closeCheckBox_toggled( bool on )
{
    m_sitePtr->getWidget()->setCloseButtonShown( on );
}
