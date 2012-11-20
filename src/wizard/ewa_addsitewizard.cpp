/*******************************************************************************
**
** file: ewa_addsitewizard.cpp
**
** class: EWAAddSiteWizard
**
** description:
** "Add site" wizard = )
**
** 09.02.2009
**
** ewauthor@indatray.com
**
*******************************************************************************/

#include <QLabel>
#include <QVBoxLayout>
#include <QSysInfo>

#include <QMessageBox>

#include "ewa_addsitewizard.h"

#include "ewa_aswpage_greeting.h"
#include "ewa_aswpage_inputurl.h"
#include "ewa_sitehandle_wizardpage.h"

#include "ewa_application.h"
#include "ewa_sitehandle.h"

#define EWA_WIZARD_GREETINGPAGE_ID 0
#define EWA_WIZARD_INPUTURLGPAGE_ID 1
#define EWA_WIZARD_BROWSERPAGE_ID 2

#include <QInputDialog>
EWAAddSiteWizard::EWAAddSiteWizard( QWidget *parent,EWASiteHandle *pSite )
:QWizard( parent )
{
    this->setOption( QWizard::NoBackButtonOnStartPage, true );
    
    setWindowTitle( tr( "Add Site Wizard" ) );

    m_qstrWizardTitle = tr( "<font color=#000000>Adding new site to the Sites List</font>" );

    QWizardPage *pPage = getGreetingPage();
	addPage( pPage );
    
	pPage = getInputUrlPage();
    addPage( pPage );
    
	pPage = getBrowserPage( pSite );
    addPage( pPage );
    
    //setWizardStyle( (QWizard::)QInputDialog::getInt( 0, "style:", "input:", 0, 0, 3 ) );
    setWizardStyle( EWAAddSiteWizard::ModernStyle );
    
    connect( this, SIGNAL(currentIdChanged(int)), this, SLOT(slotCurrentPageChanged(int)) );
}

EWAAddSiteWizard::~EWAAddSiteWizard()
{
}

QWizardPage *EWAAddSiteWizard::getGreetingPage()
{
    m_pPageGreeting = new EWAASWPage_Greeting( this );
    
    m_pPageGreeting->setTitle( getTitle() );
    m_pPageGreeting->setPixmap( QWizard::WatermarkPixmap, QPixmap( ":/images/loginwizard_1.png" ) );

    return m_pPageGreeting;
}

QWizardPage *EWAAddSiteWizard::getInputUrlPage()
{
    m_pPageInputUrl = new EWAASWPage_InputUrl( this );
    
    m_pPageInputUrl->setTitle( getTitle() );
    m_pPageInputUrl->setPixmap( QWizard::WatermarkPixmap, QPixmap( ":/images/loginwizard_2.png" ) );

    return m_pPageInputUrl;
}

QWizardPage *EWAAddSiteWizard::getBrowserPage( EWASiteHandle *pSite )
{
    m_pPageBrowser = new EWAASWPage_BrowserPage( this );

    m_pPageBrowser->setTitle( getTitle() );
    m_pPageBrowser->setPixmap( QWizard::WatermarkPixmap, QPixmap( ":/images/loginwizard_3.png" ) );

    m_pPageBrowser->setSite( pSite );

    return m_pPageBrowser;
}

QString EWAAddSiteWizard::getUrl() const
{
    return field( "initialUrl" ).toString();
}

void EWAAddSiteWizard::slotCurrentPageChanged( int id )
{
    if( id == EWA_WIZARD_BROWSERPAGE_ID )
    {
        m_pPageBrowser->enableNavButtons( false );
    }
}

void EWAAddSiteWizard::show()
{
    EWAApplication::makeWidgetCentered( this );

    QWizard::show();

#ifdef Q_OS_WIN
    //-- on win7 with enabled aero there is bug with 
    //-- controlls visibility
    //-- (http://bugreports.qt.nokia.com/browse/QTBUG-11815?page=com.atlassian.jira.plugin.system.issuetabpanels:worklog-tabpanel)
    if( QSysInfo::WinVersion::WV_6_0 >= QSysInfo::windowsVersion() )
        resize( size().width()+1, size().height() );
#endif //-- Q_OS_WIN
}

void EWAAddSiteWizard::accept()
{
    if( m_pPageBrowser )
    {
        m_pPageBrowser->releaseSite();
    }
    
    QWizard::accept();
}
