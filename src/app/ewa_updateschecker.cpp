/*******************************************************************************
**
** file: ewa_updateschecker.cpp
**
** class: EWAUpdatesChecker
**
** description:
** Updates checker =)
**
** 14.11.2009
**
** ewauthor@indatray.com
**
*******************************************************************************/

#include "ewa_updateschecker.h"

#include "ewa_sitehandle.h"
#include "ewa_networkaccessmanager.h"
#include "ewa_application.h"
#include "ewa_timer.h"

#include <QRegExp>

EWAUpdatesChecker::EWAUpdatesChecker( QObject *pParent )
:QObject( pParent )
{
    m_bUsed = true;
    
    m_strUpdatesUrl = "http://cv.indatray.com/";

    createSite();
#ifdef EWA_DBG
    qDebug( "EWAUpdatesChecker::EWAUpdatesChecker:UA: %s",qPrintable( m_pSite->getWebPage()->getUserAgent() ) );
#endif //-- EWA_DBG
    m_pTimer = new EWATimer( this );
    m_pTimer->setInterval( 1000 );
    connect( m_pTimer, SIGNAL( signalTimeOut() ), this, SLOT( slotOneSecondTicked() ) );
}

void EWAUpdatesChecker::initDefaultPrevMetchedValue()
{
    m_strPrevMatchedValue = QString( "%1" ).arg( EWAApplication::getCurrentVersion() );
}

void EWAUpdatesChecker::createSite()
{
    m_pNetworkManager = new EWANetworkAccessManager( this );
    m_pSite = new EWASiteHandle();
    
    m_pSite->setEWANetworkManagerPtr( m_pNetworkManager );
    m_pNetworkManager->setSite( m_pSite );
    
    m_pSite->setUsed( true );

    m_pSite->setUrl( m_strUpdatesUrl );

    m_pSite->setPeriodAndUnits( 3600,//-- one hour
        0 ); //-- seconds 

    initChangeRules();

    intiSiteWidget();
    
    m_pSite->getWebPage()->setUserAgent( m_pSite->getWebPage()->getOriginalUserAgent() );

    initDefaultPrevMetchedValue();
    m_pSite->setPrevMatched( m_strPrevMatchedValue );

    connect( m_pSite, SIGNAL( signalDownloaded(const EWASitePtr*) ),
            this, SLOT( slotSiteDownloaded(const EWASitePtr*) ) );
}

void EWAUpdatesChecker::intiSiteWidget()
{
    m_pSite->setMsgTitle( tr( "EWA - update" ) );
    m_pSite->getWidget()->initForUpdatesEnabled();
}

EWAUpdatesChecker::~EWAUpdatesChecker()
{
    delete m_pSite;
    m_pSite = 0;
}

void EWAUpdatesChecker::setUsed( bool bUsed )
{
    m_bUsed = bUsed;
    if( m_bUsed )
    {
        startChecking();
    }
    else
    {
        stopChecking();
        initDefaultPrevMetchedValue();
        m_pSite->setPrevMatched( m_strPrevMatchedValue );
    }
}

void EWAUpdatesChecker::startChecking()
{
    stopChecking();
    
    initChangeRules();
    
    m_pSite->slotDownload();
    m_pTimer->start();
}

void EWAUpdatesChecker::stopChecking()
{
    if( m_pTimer->isActive() || m_pSite->isStarted() )
    {
        m_pSite->stop();
        m_pTimer->stop();
        
        if( m_pSite->getWidget()->wasShown() )
        {
            m_pSite->getWidget()->close();
        }
    }
}

void EWAUpdatesChecker::slotOneSecondTicked()
{
    if( !m_pSite->isDownloadingActive() 
        && m_pSite->isUsed() 
        && m_pSite->isStarted() 
        && (m_pSite->getWidget()->isViewNormal() || !m_pSite->getWidget()->wasShown() ) )
    {
        if( !m_pSite->getCountdownCurrent() )
        {
            initChangeRules();
        }
        m_pSite->oneSecondTicked();
    }
}

void EWAUpdatesChecker::slotSiteDownloaded(const EWASitePtr*)
{
    resetChangeRules();
    QString strLastMatchedVersion = m_pSite->getPrevMatched();
    m_strPrevMatchedValue = strLastMatchedVersion;
    
    m_pSite->start();
}

QString EWAUpdatesChecker::getPrevMatchedString() const 
{
    return m_strPrevMatchedValue;
}

void EWAUpdatesChecker::updatePrevMatchedString( const QString& str )
{
    if( !str.isEmpty() )
    {
        m_pSite->setPrevMatched( str );
    }
}

void EWAUpdatesChecker::initChangeRules()
{
    QRegExp rx( QString( "<font id=\"ewa_last_version\"color=#0000ff><b>(.*)</b></font>" ) );
    rx.setMinimal( false );
    rx.setCaseSensitivity( Qt::CaseInsensitive );

    m_pSite->getShowPolicy()->setUsed( true );
    m_pSite->getShowPolicy()->setRegExp( rx );
    
    QString strOsNameTmpl = 
#if defined (Q_OS_MAC)
    "macos"
#elif defined (Q_OS_LINUX)
    "linux"
#elif defined (Q_OS_WIN)
    "windows"
#endif //-- Q_OS_MAC Q_OS_LINUX Q_OS_WIN
    ;
    
    QString strDldLinkTmpl = tr( "http://indatray.com/%1/%2.html" )
        .arg( EWAApplication::instance()->getLocalizationName().left( 2 ) )
        .arg( strOsNameTmpl );

    m_pSite->getRulesCollection()->addRule( "<body>(\\d+\\.\\d+\\.\\d+.*)</body>", tr( "The newer version<br><font id=\"ewa_last_version\"color=#0000ff><b>%</b></font><br>is available! Check it out at<br><a href=\"%1\">www.indatray.com</a>" ).arg( strDldLinkTmpl ), true, false );
}

void EWAUpdatesChecker::resetChangeRules()
{
    m_pSite->getShowPolicy()->setUsed( false );
    
    m_pSite->getRulesCollection()->clear();
}
