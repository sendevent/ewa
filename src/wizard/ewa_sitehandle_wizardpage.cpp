/*******************************************************************************
**
** file: ewa_sitehandle_wizardpage.cpp
**
** class: EWAASWPage_BrowserPage
**
** description:
** WizardPage with SiteHandle object.
** Used in "Add site" wizard.
**
** 09.02.2009
**
** sendevent@gmail.com
**
*******************************************************************************/

#include "ewa_sitehandle_wizardpage.h"
#include "ewa_webview.h"
#include "ewa_sitehandle.h"
#include <QAbstractButton>
#include <QVBoxLayout>
#include <QWebView>
#include <QWebFrame>
#ifdef Q_OS_LINUX
#include <QScrollArea>
#endif

EWAASWPage_BrowserPage::EWAASWPage_BrowserPage( QWidget *parent )
:QWizardPage( parent )
{
    ui.setupUi( this );
    m_pPrevMsgParent = 0;
    m_pSite = 0;
}

void EWAASWPage_BrowserPage::setSite( EWASiteHandle *pSite )
{
    if( !pSite )
    {
        return;
    }

    m_pSite = pSite;
    m_pSite->getWidget()->setAsPreview( true );
    
    prepareSiteWidget( true );
}

void EWAASWPage_BrowserPage::prepareSiteWidget( bool bForWizard )
{
    if( !m_pSite )
        return;
    m_pSite->setRecording( bForWizard );
    m_pSite->getWidget()->setTitleLabelShown( true );
    m_pSite->getWidget()->setBackButtonShown( bForWizard );
    m_pSite->getWidget()->setRefreshButtonShown( bForWizard );
    m_pSite->getWidget()->setForwardButtonShown( bForWizard );
    m_pSite->getWidget()->setStickButtonShown( bForWizard );
    m_pSite->getWidget()->setViewModeButtonShown( bForWizard );
    m_pSite->getWidget()->setCloseButtonShown( bForWizard );

    if( bForWizard )
    {
        prevFlags = m_pSite->getWidget()->windowFlags();
        m_pPrevMsgParent = m_pSite->getWidget()->parentWidget();
		m_pSite->getWidget()->makeInvisible( false );
#ifdef Q_OS_LINUX
        //m_pSite->getWidget()->setWindowFlags( Qt::Window );
        m_pSite->getWidget()->setAsPreview( true );
        //ui.verticalLayout->addWidget( (QWidget*)m_pSite->getWidget() );
        m_pSite->getWidget()->hide();
       ui.verticalLayout->addWidget( (QWidget*)m_pSite->getWidget()->scrollArea() );
#else
        ui.verticalLayout->addWidget( m_pSite->getWidget() );
        m_pSite->getWidget()->show();
#endif

        
        m_PrevMinSz = m_pSite->getWidget()->minimumSize();
        m_PrevMaxSz = m_pSite->getWidget()->maximumSize();
        m_pSite->getWidget()->slotShowScrollBars();

        m_pSite->getWidget()->getWebView()->recordStarted();
        connect( m_pSite->getWebPage(), SIGNAL( loadFinished(bool) ),
            this, SLOT( slotPageLoaded(bool) ) );
        connect( m_pSite->getWebPage(), SIGNAL( loadStarted() ),
            this, SLOT( slotLoadStarted() ) );
    }
    else
    {
#ifdef Q_OS_LINUX
        QWidget *pW = ui.verticalLayout->takeAt( 0 )->widget();
        if( pW )
        {
            ui.verticalLayout->removeWidget( pW );
            QScrollArea *pSA =  qobject_cast<QScrollArea *>(pW);
            if( pSA )
            {
                m_pSite->getWidget()->setScrollArea( pSA );
                //qDebug() << pSA;
            }
            //qDebug() << m_pSite->getWidget()->scrollArea();
        }
        //m_pSite->getWidget()->hide();
        m_pSite->getWidget()->setAsPreview( true );
#else
        ui.verticalLayout->removeWidget( m_pSite->getWidget() );
#endif
        m_pSite->getWidget()->setParent( m_pPrevMsgParent );
        m_pSite->getWidget()->setMinimumSize( m_PrevMinSz );
        m_pSite->getWidget()->setMaximumSize( m_PrevMaxSz );
        m_pSite->getWidget()->setWindowFlags( prevFlags );

        m_pSite->getWidget()->getWebView()->recordFinished();

        disconnect( m_pSite->getWebPage(), SIGNAL( loadFinished(bool) ),
            this, SLOT( slotPageLoaded(bool) ) );
        disconnect( m_pSite->getWebPage(), SIGNAL( loadStarted() ),
            this, SLOT( slotLoadStarted() ) );

        m_pSite = 0;
    }
}

EWAASWPage_BrowserPage::~EWAASWPage_BrowserPage()
{
    //-- ui.verticalLayout->removeWidget( m_pSite->getWidget() );
    //-- void QLayout::removeWidget ( QWidget * widget )
    //-- Removes the widget widget from the layout. After this call, it is the caller's responsibility to give the widget a reasonable geometry or to put the widget back into a layout.
    //-- Note: The ownership of widget remains the same as when it was added.
    //-- but now qDebug( "ParentName: %s", qPrintable( m_pSite->getWidget()->parentWidget()->objectName() ) );
    //-- will print: ParentName: frame
    //-- so, it's needed to set parent manually.
    
    if( m_pSite )
        prepareSiteWidget( false );
}

void EWAASWPage_BrowserPage::releaseSite()
{
    prepareSiteWidget( false );
}

void EWAASWPage_BrowserPage::initializePage()
{
    Q_ASSERT( m_pSite );
    
    enableNavButtons( false );

    if( m_pSite )
    {
        QString ua = field( "userAgent" ).toString();
        m_pSite->setUserAgent( ua );
        m_pSite->setUrl( field( "initialUrl" ).toString() );

        m_pSite->setMsgTitle( field( "siteDescriptionLineEdit" ).toString() );

		m_pSite->getWebView()->startRecording();
    }
}

void EWAASWPage_BrowserPage::slotLoadStarted()
{
    enableNavButtons( false );
}

void EWAASWPage_BrowserPage::slotPageLoaded( bool ok )
{   
    enableNavButtons( true );
    
    if( !ok || !m_pSite )
    {
        return;
    }

    QSize sz = m_pSite->getWebPage()->mainFrame()->contentsSize();
    if( sz.isEmpty() )
    {
        return;
    }

    m_pSite->setWebPageBaseSize( sz );
}

void EWAASWPage_BrowserPage::cleanupPage()
{
    enableNavButtons( false );
}

void EWAASWPage_BrowserPage::enableNavButtons( bool enable )
{
    QWizard *pWizard = wizard();
    if( !pWizard )
    {
        return;
    }
    
    QAbstractButton *pButton = pWizard->button( QWizard::FinishButton );
    if( pButton )
    {
        pButton->setEnabled( enable );
    }
}
