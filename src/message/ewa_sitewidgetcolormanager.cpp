#include "ewa_sitewidgetcolormanager.h"
#include "ewa_colordialog.h"
#include "ewa_sitehandle.h"

#include <QPixmap>
#include <QPainter>
#ifdef Q_OS_LINUX
#include <QColorDialog>
#endif

EWASiteWidgetColorManager::EWASiteWidgetColorManager( QWidget *pParent )
:QWidget( pParent )
{
    ui.setupUi( this );
    m_sitePtr = 0;
    m_pPix = new QPixmap( 16, 16 );
}

EWASiteWidgetColorManager::~EWASiteWidgetColorManager()
{
    delete m_pPix;
}

void EWASiteWidgetColorManager::setSite( EWASiteHandle *pSite )
{
    if( pSite )
    {
        m_sitePtr = pSite;
        slotSetButtonColor( m_sitePtr->getWidget()->getColor() );
        
        int iBaseTransparency = m_sitePtr->getMsgBaseTransparency();
        ui.transparencySpinBox->setValue( iBaseTransparency );
        connectSiteWidget();
    }
}

void EWASiteWidgetColorManager::resetSite()
{
    if( m_sitePtr )
    {
        disconnectSiteWidget();
        m_sitePtr = 0;
    }
}

void EWASiteWidgetColorManager::slotSetButtonColor( const QColor& color )
{
    m_pPix->fill( color );

    QPainter painter( m_pPix );
    painter.setPen( Qt::black );
    painter.drawRect( 1, 1, 13, 13 );
    
    ui.changeColorButton->setIcon( *m_pPix );
}

void EWASiteWidgetColorManager::on_changeColorButton_clicked()
{
    if( !m_sitePtr )
    {
        return;
    }

    QColor newColor;
#ifndef Q_OS_LINUX
    EWAColorDialog *pColorDlg = new EWAColorDialog;
    pColorDlg->setColor( m_sitePtr->getWidget()->getColor() );

    if( pColorDlg->exec() == QDialog::Accepted )
    {
        newColor = pColorDlg->getColor();
    }
    
    delete pColorDlg;
#else
    newColor = QColorDialog::getColor( m_sitePtr->getWidget()->getColor(),
                                              this, tr( "Select widget's color:" ) );
#endif //-- Q_OS_LINUX
    if( newColor.isValid() )
    {
        slotSetButtonColor( newColor );
        m_sitePtr->getWidget()->setColor( newColor );
    }
}

void EWASiteWidgetColorManager::on_transparencySpinBox_valueChanged( int val )
{
    if( m_sitePtr && m_sitePtr->getMsgBaseTransparency() != val )
    {
        disconnectSiteWidget();
        m_sitePtr->setMsgBaseTransparency( val );
        connectSiteWidget();
    }
}

void EWASiteWidgetColorManager::connectSiteWidget()
{
    if( m_sitePtr && m_sitePtr->getWidget() )
    {
        connect( m_sitePtr->getWidget(), SIGNAL( signalOpacityChanged(int) ),
            this, SLOT( slotSiteWidgetOpacityChanged(int) ) );
        connect( m_sitePtr->getWidget(), SIGNAL( signalXMsgColorChanged(const QColor&) ),
            this, SLOT( slotSetButtonColor(const QColor&) ) );
    }
}

void EWASiteWidgetColorManager::disconnectSiteWidget()
{
    if( m_sitePtr && m_sitePtr->getWidget() )
    {
        disconnect( m_sitePtr->getWidget(), SIGNAL( signalOpacityChanged(int) ),
            this, SLOT( slotSiteWidgetOpacityChanged(int) ) );
        disconnect( m_sitePtr->getWidget(), SIGNAL( signalXMsgColorChanged(const QColor&) ),
            this, SLOT( slotSetButtonColor(const QColor&) ) );
        
    }
}

void EWASiteWidgetColorManager::slotSiteWidgetOpacityChanged(int value)
{
    if( value != ui.transparencySpinBox->value() )
    {
        disconnectSiteWidget();
        ui.transparencySpinBox->setValue( value );
        connectSiteWidget();
    }
}
