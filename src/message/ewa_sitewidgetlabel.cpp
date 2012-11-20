/*******************************************************************************
**
** file: ewa_sitewidgetlabel.cpp
**
** class: EWASiteWidgetLabel
**
** description:
** Info control for EWASiteWidget. Showing in message title area time/download
** progress and text description, like as in EWASitesModelView.
**
** 29.05.2009
**
** ewauthor@indatray.com
**
*******************************************************************************/

#include "ewa_sitewidgetlabel.h"
#include "ewa_sitehandle.h"
#include "ewa_sitedrawer.h"

#include <QPainter>

#define LITE_GRAY 250

EWASiteWidgetLabel::EWASiteWidgetLabel( QWidget *pParent )
:EWATextLabel( pParent )
,m_sitePtr( 0 )
,m_lTime( 0 )
,m_lProgress( 0 )
{
    QPalette pal = palette();
    pal.setColor( QPalette::WindowText, Qt::black );
    setPalette( pal );
    
    m_ggColor = QColor( LITE_GRAY, LITE_GRAY, LITE_GRAY );
    m_color = Qt::white;
    updateTooltip();
}

EWASiteWidgetLabel::~EWASiteWidgetLabel()
{
    if( m_sitePtr )
    {
        disconnectSite();
        m_sitePtr = 0;
    }
}

void EWASiteWidgetLabel::setSite( EWASiteHandle *pSite )
{
    disconnectSite();

    m_sitePtr = pSite;

    connectSite();
}

void EWASiteWidgetLabel::connectSite()
{
    if( !m_sitePtr )
    {
        return;
    }

    connect( m_sitePtr, SIGNAL( signalSiteLoadProgress(int) ),
        this, SLOT( slotUpdateSiteInfo(int) ) );
    connect( m_sitePtr, SIGNAL( signalSiteTypingProgress(int) ),
        this, SLOT( slotUpdateSiteInfo(int) ) );        
    connect( m_sitePtr, SIGNAL( signalOneSecond() ),
        this, SLOT( slotUpdateSiteInfo() ) );
    connect( m_sitePtr, SIGNAL( signalDescriptionChanged(const QString&) ),
        this, SLOT( slotUpdateSiteInfo(const QString&) ) );
}

void EWASiteWidgetLabel::disconnectSite()
{
    if( !m_sitePtr )
    {
        return;
    }

    disconnect( m_sitePtr, SIGNAL( signalSiteLoadProgress(int) ),
        this, SLOT( slotUpdateSiteInfo(int) ) );
    disconnect( m_sitePtr, SIGNAL( signalSiteTypingProgress(int) ),
        this, SLOT( slotUpdateSiteInfo(int) ) );
    disconnect( m_sitePtr, SIGNAL( signalOneSecond() ),
        this, SLOT( slotUpdateSiteInfo() ) );
    disconnect( m_sitePtr, SIGNAL( signalDescriptionChanged(const QString&) ),
        this, SLOT( slotUpdateSiteInfo(const QString&) ) );
}

void EWASiteWidgetLabel::slotUpdateSiteInfo( int )
{
    slotUpdateSiteInfo();
}

void EWASiteWidgetLabel::slotUpdateSiteInfo( bool )
{
    slotUpdateSiteInfo();
}

void EWASiteWidgetLabel::slotUpdateSiteInfo( const QString& )
{
    slotUpdateSiteInfo();
}

void EWASiteWidgetLabel::slotUpdateSiteInfo()
{
    if( !m_sitePtr )
    {
        return;
    }

    m_lTime = m_sitePtr->getCountdownCurrent();
    m_lProgress = m_sitePtr->getDownloadProgress();
    
    updateTooltip();

    update();
}

void EWASiteWidgetLabel::updateTooltip()
{
    QString stateDescr = tr( "Time/Download progress" );
    QString siteDescr;

    if( m_sitePtr )
    {
        siteDescr = tr( "<b>%1</b><br>" ).arg( m_sitePtr->getUrlStrLimit32() );

        if( m_sitePtr->isDownloadingActive() )
        {
            stateDescr = tr( "Downloading... %1%" )
            .arg( m_sitePtr->getDownloadProgress() );
        }
        else if( m_sitePtr->isStarted() && m_sitePtr->isUsed() )
        {
            stateDescr = tr( "Next check will be in: %1" )
            .arg( m_sitePtr->getCountdownCurrentHMS() );
        }
    }

    QString toolTip = tr( "%1%2" )
    .arg( siteDescr )
    .arg( stateDescr );

    this->setToolTip( toolTip );
    emit signalTooltipChanged( toolTip );
}

void EWASiteWidgetLabel::paintEvent( QPaintEvent *e )
{
    if( m_sitePtr )
    {
        QPainter painter( this );
        if( m_sitePtr )
        {
            QRect rect = fontMetrics().boundingRect( this->rect(), alignment(), text() );
            EWASiteDrawer::drawSiteStateInfo( m_sitePtr, &painter, rect, false, false );
        }
    }
    EWATextLabel::paintEvent( e );
}

void EWASiteWidgetLabel::resizeEvent( QResizeEvent *e )
{
    QLabel::resizeEvent( e );
    updateBackgroundGradient();
}

void EWASiteWidgetLabel::setColor( const QColor& color )
{
    EWATextLabel::setColor( color );
}
