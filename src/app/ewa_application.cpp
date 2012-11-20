/*******************************************************************************
**
** file: ewa_application.cpp
**
** class: EWAApplication
**
** description:
** QApplication with some static additional public members
** ( "global" acces to managers and some properties ).
**
** 09.02.2009
**
** ewauthor@indatray.com
**
*******************************************************************************/

#include <QSettings>
#include <QTranslator>
#include <QLocale>
#include <QTimer>

#include <QDesktopServices>
#include <QWebSettings>
#include <QtNetwork>
#include <QClipboard>

#include "ewa_useractionsplayer.h"

#ifdef Q_OS_LINUX
    #include <pwd.h>
    #include <sys/utsname.h>
#elif defined(Q_OS_WIN)
    #include <windows.h>
    #include "ewa_sitewidgetwindows.h"
    
    //-- process "Show Desktop" for 
    //-- "Stay on Desktop" widgets:
    void CALLBACK winHook( HWINEVENTHOOK hook,
        DWORD event,
        HWND hwnd,
        LONG idObject,
        LONG idChild,
        DWORD dwEventThread,
        DWORD dwmsEventTime )
    {
        Q_UNUSED( hook );
        Q_UNUSED( idObject );
        Q_UNUSED( idChild );
        Q_UNUSED( dwEventThread );
        Q_UNUSED( dwmsEventTime );

        if( ::IsWindow( hwnd ) && event == EVENT_SYSTEM_FOREGROUND )
        {
            TCHAR tmp[255];
            if( 0 != GetClassName(hwnd, tmp, 250)
                && 0 == wcscmp( tmp, L"WorkerW" ) )
            {
                EWASiteWidgetWindows::call4EveryReady( &EWASiteWidgetWindows::onShowDesktop );
            }
        }
    }
#endif //-- Q_OS_LINUX

#include "ewa_application.h"

#include "ewa_versiondef.h"
#include "ewa_sendstatisticdlg.h"
#include "ewa_mainwindow.h"
#include "ewa_siteslistmanager.h"
#include "ewa_sitehandle.h"
#include "ewa_trayiconmanager.h"
#include "ewa_buzzer.h"
#include "ewa_splashscreen.h"
#include "ewa_sitethumbnailfilesystem.h"


/*static*/ EWAMainWindow *EWAApplication::s_pMainWindow = 0;
/*static*/ EWATrayIconManager *EWAApplication::m_pTrayIconManager = 0;
/*static*/ EWASitesListManager *EWAApplication::s_pSitesManager = 0;
/*static*/ QTranslator *EWAApplication::s_translator = 0;
/*static*/ QString EWAApplication::m_qstrEWAVersion = EWA_VERSION_STRING;

/*static*/ QString EWAApplication::m_qstrUniqueId = QString();
/*static*/ QStringList EWAApplication::m_strlstImgFormats = QStringList();
/*static*/ QString EWAApplication::m_qstrUserAgent = QString();
/*static*/ QString EWAApplication::m_qstrMachineName = QString();
/*static*/ QString EWAApplication::m_qstrOSUserName = QString();
/*static*/ QTimer *EWAApplication::m_pAppTimer = 0;
/*static*/ int EWAApplication::s_iTimerUsers = 0;
/*static*/ QList<const EWATimer*> *EWAApplication::s_pTimerListeners = 0;
/*static*/ QPoint EWAApplication::m_pntScreensRange = QPoint( -1, -1 );
QMap< QString, int > EWAApplication::classes = QMap< QString, int >();
/*static*/ EWASplashScreen* EWAApplication::m_pSplashScreen = 0;

/*static*/ QMap<QString, int> EWAApplication::m_KeyNames = QMap<QString, int>();
/*static*/ int EWAApplication::m_iOneTickPeriod = 10;


/*static*/ EWASettings *EWAApplication::m_pSettings = 0;


/*static*/ EWASiteThumbnailFileSystem* EWAApplication::m_pSiteThumbnailsFS = 0;
/*static*/ QString EWAApplication::m_qstrLocalization("en");
/*static*/ QTranslator* EWAApplication::m_pTranslator = new QTranslator( EWAApplication::instance() );


/*static*/ QString EWAApplication::getLocalizationName() {return m_qstrLocalization;}

/*static*/ QList<EWAUserActionsPlayer*>* EWAApplication::s_pGesturePlayers = new QList<EWAUserActionsPlayer*>();

EWAApplication::EWAApplication( int &argc, char **argv )
    : QApplication( argc, argv )
{
    QTextCodec::setCodecForTr( QTextCodec::codecForName( "UTF-8" ) );
    
    EWAApplication::setOrganizationName( QLatin1String( "indatray.com" ) );
    EWAApplication::setApplicationName( QLatin1String( "Enhanced Web Assistant" ) );
    EWAApplication::setApplicationVersion( EWAApplication::getCurrentVersion() );
    
    m_qstrEWAVersion.append( QString(" %1" ).arg( QChar(0x03B2) ) );
#ifdef EWA_DBG
    m_qstrEWAVersion.append("(d)" );
#endif //-- EWA_DBG

#ifdef Q_OS_WIN
    m_hWinHookId = SetWinEventHook( EVENT_SYSTEM_FOREGROUND, EVENT_SYSTEM_FOREGROUND, NULL, winHook, 0, 0, 0 );
#endif //-- Q_OS_WIN
    
    m_pTranslator->load("qt_" + QLocale::system().name(), EWAApplication::settings()->getTranslationsFolder() );
    EWAApplication::instance()->QApplication::installTranslator( m_pTranslator );


    iniUniqueId();
    initAllowedImgFormats();

    m_pHomeBuzzer = new EwaBuzzer();

    moveStuffFoldersToUserDir();

    //m_qstrDefaultContent = translate( this, getBlankPageSourceTemplate().arg( translate( this, "Content not loaded." ) ).toUtf8().data() );
    
    settings()->load();
        
    setWindowIcon( QIcon( settings()->getDefaultIconFile() ) );
    //-- App's icon already actualized:    
#ifndef QT_NO_OPENSSL
    if (!QSslSocket::supportsSsl()) {
    QMessageBox::information(0, applicationName(),
                 translate( this, 
                 tr( "This system does not support OpenSSL &mdash;<br>"
                    "SSL websites (https://) will not be available." ).toUtf8().data() ) );
    }
#endif
    
    
}

EWAApplication::~EWAApplication()
{
#ifdef Q_OS_WIN
    UnhookWinEvent( m_hWinHookId );
#endif //-- Q_OS_WIN
    delete s_pSitesManager;
    delete m_pTrayIconManager;
    delete s_pMainWindow;

    delete m_pAppTimer;
    delete s_pTimerListeners;

    delete m_pHomeBuzzer;
}

EWAApplication *EWAApplication::instance()
{
    return ( static_cast<EWAApplication *>( QCoreApplication::instance() ) );
}

/*static*/ EWAMainWindow* EWAApplication::getMainWindow()
{
    if( !s_pMainWindow )
    {
        s_pMainWindow = new EWAMainWindow();
    }

    return s_pMainWindow;
}


void EWAApplication::firstRun()
{
    QComboBox *pCmbBox = getMainWindow()->ui.langComboBox;
    int id = pCmbBox->findText( setDefaultLocalization() );
    pCmbBox->setCurrentIndex( -1 == id ? 0 : id );
    
    
    /*EWASitesListManager *pSitesManager = getSitesManager();
    int iDeltaY = 0;
    
    for( int i = 0; i < pSitesManager->getSitesCount(); ++i )
    {
        EWASiteHandle *pSite = pSitesManager->getSiteByNumber( i );
        if( !pSite )
        {
            continue;
        }
        
        QPoint loc( pSite->getMsgLocation() );
        
        loc.setY( iDeltaY );
        loc.setX( getScreenSize().width() - pSite->getMsgWidth() );
        pSite->setMsgLocation( loc );
        
        iDeltaY += pSite->getMsgHeight();
    }*/
}


EWASitesListManager *EWAApplication::getSitesManager()
{
    if ( !s_pSitesManager ) {
        s_pSitesManager = new EWASitesListManager();
    }
    return s_pSitesManager;
}

/*static*/ EWATrayIconManager* EWAApplication::getTrayIconManager()
{
    if ( !m_pTrayIconManager ) {
        m_pTrayIconManager = new EWATrayIconManager();
    }
    return m_pTrayIconManager;
}

void EWAApplication::installTranslator( const QString &name )
{
    if( !s_translator )
        s_translator = new QTranslator( this );

    s_translator->load( name, QLibraryInfo::location( QLibraryInfo::TranslationsPath ) );
    QApplication::installTranslator( s_translator );
}

/*static*/ QStringList EWAApplication::getImgFormats()
{
    initAllowedImgFormats();
    return m_strlstImgFormats;
}


/*static*/ void EWAApplication::addIdItem( const QString& strItem )
{
    if( strItem.isEmpty() )
    {
        return;
    }
    
    m_qstrUniqueId += strItem;
}

/*static*/ void EWAApplication::iniUniqueId()
{
    if( m_qstrUniqueId.length() ) return;

    try
    {
        
        
        addIdItem( tr( "%1@%2" ).arg( getUserName() ).arg( getMachineName() ) );
        m_qstrUniqueId += "[eewwaaddeelliimm]";
        QList<QHostAddress> adrs = QNetworkInterface::allAddresses();
        foreach( QHostAddress address, adrs )
        {
            addIdItem( address.toString().toUtf8() );//-- eth & lo addresses
        }
        
        m_qstrUniqueId += "[eewwaaddeelliimm]";

        QList<QNetworkInterface> interfaces = QNetworkInterface::allInterfaces();
        foreach( QNetworkInterface inter, interfaces )
        {
            addIdItem( inter.hardwareAddress().toUtf8() );//-- mac
        }
    }
    catch( ... )
    {
    }
}

/*static*/ void EWAApplication::initAllowedImgFormats()
{
    if( m_strlstImgFormats.count() ) return;

    QList<QByteArray> formatsList = QImageReader::supportedImageFormats();
    formatsList += QMovie::supportedFormats();

    QSet<QByteArray> uniqImgFormats = formatsList.toSet();

    foreach( QByteArray currentFormat, uniqImgFormats )
    {
        m_strlstImgFormats.append( QObject::tr( "*.%1" ).arg( QString::fromUtf8( currentFormat ) ) );
    }
}

/*static*/ const QString& EWAApplication::getUserName()
{
    if( m_qstrOSUserName.isEmpty() )
    {

#ifdef Q_OS_WIN
	QT_WA(
	{
		wchar_t buffer[256];
		DWORD bufferSize = sizeof( buffer ) / sizeof( wchar_t ) - 1;
		GetUserNameW( buffer, &bufferSize );
		m_qstrOSUserName = QString::fromUtf16( ( ushort* )buffer );
	},
	{
		char buffer[256];
		DWORD bufferSize = sizeof( buffer ) / sizeof( char ) - 1;
		GetUserNameA( buffer, &bufferSize );
		m_qstrOSUserName = QString::fromLocal8Bit( buffer );
	}
	 ); //-- QT_WA
#else
	struct passwd* pwd = getpwuid( getuid() );
	if( pwd )
		m_qstrOSUserName = QString( pwd->pw_name );
#endif //-- Q_OS_WIN
    }
    return m_qstrOSUserName;
}

/*static*/ const QString& EWAApplication::getMachineName()
{
    if( m_qstrMachineName.isEmpty() )
    {
        m_qstrMachineName = QHostInfo::localHostName().toUtf8(); //-- localhost name
    }
    
    return m_qstrMachineName;
}

/*static*/ const QString& EWAApplication::getUniqueId()
{
    if( m_qstrUniqueId.isEmpty() )
        iniUniqueId();

    return m_qstrUniqueId;
}

/*static*/ const QString EWAApplication::getCommitHash()
{
    return EWA_VERSION_COMMIT_HASH;
}

/*static*/ const QString& EWAApplication::getCurrentVersion()
{
    return m_qstrEWAVersion;
}

/*static*/ const QString EWAApplication::getCurrentVersionWithHash()
{
    return QString( "%1 [%2]" ).arg( getCurrentVersion() ).arg( getCommitHash() );
}

/*static*/ const QString EWAApplication::getCurrentVersionBuildTime()
{
    return EWA_VERSION_BLDTIME;
}

/*static*/ QString EWAApplication::getEWAUserAgent()
{
    if( m_qstrUserAgent.isEmpty() )
    {
        EWAWebPage p;
        m_qstrUserAgent = p.getOriginalUserAgent();
    }

    return m_qstrUserAgent;
}

/*static*/ void EWAApplication::initUserAgentsList( QMap<QString, QString>& map )
{
    map.clear();

    QString qstrCurrentLocale = getCurrentLocaleName();

	map.insert( QObject::tr( "EWA" ), getEWAUserAgent() );
    map.insert( QObject::tr( "Custom..." ), "" );
    //-- Opera:
    map.insert( QObject::tr( "Opera 8.00 on GNU/Linux Gentoo" ), QObject::tr( "Opera/8.0 (X11; Linux i686; U; %1)" ).arg( qstrCurrentLocale ) );
    map.insert( QObject::tr( "Opera 9.10 on Windows XP" ), QObject::tr( "Opera/9.10 (Windows NT 5.1; U; %1)" ).arg( qstrCurrentLocale ) );
    map.insert( QObject::tr( "Opera 10.00 on Windows Seven" ), QObject::tr( "Opera/9.80 (Windows NT 6.1; U; %1) Presto/2.2.15 Version/10.00" ).arg( qstrCurrentLocale ) );
    //-- MSIE:
    map.insert( QObject::tr( "MS IE 8.0 on Windows Seven" ), QObject::tr( "Mozilla/4.0 ( compatible; MSIE 7.0; Windows NT 6.1; YPC 3.0.1; .NET CLR 1.1.4322; .NET CLR 2.0.50727 )" ) );
    //-- Mozilla Firefox:
    map.insert( QObject::tr( "Firefox 3.5.1 on GNU/Linux Ubuntu" ), QObject::tr( "Mozilla/5.0 (X11; U; Linux x86_64; %1; rv:1.9.1.1) Gecko/20090716 Ubuntu/9.04 (jaunty) Shiretoko/3.5.1" ).arg( qstrCurrentLocale ) );
    map.insert( QObject::tr( "Firefox 3.5 on Windows XP" ), QObject::tr( "Mozilla/5.0 (Windows; U; Windows NT 5.1; %1; rv:1.9.1) Gecko/20090624 Firefox/3.5" ).arg( qstrCurrentLocale ) );
    map.insert( QObject::tr( "Firefox 3.5.1 on Windows Vista" ), QObject::tr( "Mozilla/5.0 (Windows; U; Windows NT 6.0; %1; rv:1.9.1.1) Gecko/20090715 Firefox/3.5.1" ).arg( qstrCurrentLocale ) );
    //-- Safari:
    map.insert( QObject::tr( "Safari 3.0 (v521) on Mac OS X" ), QObject::tr( "Mozilla/5.0 (Macintosh; U; PPC Mac OS X; %1) AppleWebKit/521.25 (KHTML, like Gecko) Safari/521.24" ).arg( qstrCurrentLocale ) );
    map.insert( QObject::tr( "Safari 3.0.3 (522.15.5) on Windows XP" ), QObject::tr( "Mozilla/5.0 (Windows; U; Windows NT 5.1; %1) AppleWebKit/522.15.5 (KHTML, like Gecko) Version/3.03 Safari/522.15.5" ).arg( qstrCurrentLocale ) );
    //-- Google Chrome
    map.insert( QObject::tr( "Google Chrome 0.2.149.27 on Windows XP SP2" ), QObject::tr( "Mozilla/5.0 (Windows; U; Windows NT 5.1; %1) AppleWebKit/525.13 (KHTML, like Gecko) Chrome/0.2.149.27 Safari/525.13" ).arg( qstrCurrentLocale ) );
    map.insert( QObject::tr( "Google Chrome 0.4.154.25 on Windows XP SP2" ), QObject::tr( "Mozilla/5.0 (Windows; U; Windows NT 5.1; %1) AppleWebKit/525.19 (KHTML, like Gecko) Chrome/0.4.154.25 Safari/525.19" ).arg( qstrCurrentLocale ) );
    map.insert( QObject::tr( "Google Chrome 1.0.154.65 on Windows Seven" ), QObject::tr( "Mozilla/5.0 (Windows; U; Windows NT 6.1; %1) AppleWebKit/525.19 (KHTML, like Gecko) Chrome/1.0.154.65 Safari/525.19" ).arg( qstrCurrentLocale ) );
    //-- Links:
    map.insert( QObject::tr( "Links 0.99pre14 Win 2000 (Cygwin)" ), QObject::tr( "Links (0.99pre14; CYGWIN_NT-5.0 1.5.16( 0.128/4/2 ) i686; 80x25)" )  );
    //-- Lynx:
    map.insert( QObject::tr( "Lynx 2.8.6rel.4 on GNU/Linux Gentoo" ), QObject::tr( "Lynx/2.8.6rel.4 libwww-FM/2.14 SSL-MM/1.4.1 OpenSSL/0.9.8g" )  );
}

/*static*/ QRegExp EWAApplication::createRegExp( const QString& pattern )
{
    QRegExp rx( pattern );
    rx.setMinimal( true );
    rx.setPatternSyntax( QRegExp::RegExp2 );
    rx.setCaseSensitivity( Qt::CaseInsensitive );

    return rx;
}

/*static*/ const QRect EWAApplication::getScreenSize( const QPoint& pnt )
{
    if( pnt.isNull() )
    {
        return qApp->desktop()->availableGeometry();
    }
    return qApp->desktop()->availableGeometry( pnt );
}

/*static*/ QTimer *EWAApplication::getTimer()
{
    if( !m_pAppTimer )
    {
        m_pAppTimer = new QTimer();
        connect( m_pAppTimer, SIGNAL( timeout() ), EWAApplication::instance(),
            SLOT( slotAppTicked() ) );
    }

    return m_pAppTimer;
}

void EWAApplication::registerTimer( const EWATimer* pListener )
{
    if( pListener && !getTimerListeners()->contains( pListener ) )
    {
        getTimerListeners()->append( pListener );

        if( !getTimer()->isActive() )
        {
            getTimer()->start( 10 );
        }
    }
}

void EWAApplication::unregisterTimer( const EWATimer* pListener )
{
    if( pListener && getTimerListeners()->contains( pListener ) )
    {
        getTimerListeners()->removeAt( getTimerListeners()->indexOf( pListener ) );

        if( !getTimerListeners()->size() && getTimer()->isActive() )
        {
            getTimer()->stop();
        }
    }
}

/*static*/ QList<const EWATimer*> *EWAApplication::getTimerListeners()
{
    if( !s_pTimerListeners )
    {
        s_pTimerListeners = new QList<const EWATimer*>;
    }
    return s_pTimerListeners;
}

int EWAApplication::exec()
{
    m_pHomeBuzzer->sayHelloHomeServer();
    processEvents();
    return QApplication::exec();
}

void EWAApplication::quit()
{
    getSitesManager()->stopSites( false );
    getSitesManager()->closeSiteWidgets( -1 );
    
    getTrayIconManager()->hideTrayIcon();

    m_pHomeBuzzer->sayGoodbayHomeServer();
    
    processEvents();
    
    //-- settings should be saved before closing 
    //-- mainwindow to get its correct geometry:
    getMainWindow()->saveSettings();
    getMainWindow()->hide();
    
	getSitesManager()->save();
	
	/*EWASendStatisticsDlg *pStatisticDlg = new EWASendStatisticsDlg( getMainWindow() );
	pStatisticDlg->exec();
	delete pStatisticDlg;*/

    QTimer::singleShot( 3000, this, SLOT( realQuit() ) );
}


void slotSendStatistic();
void EWAApplication::realQuit()
{
    QApplication::quit();
}

/*static*/ QByteArray EWAApplication::getXORedString( const QByteArray& src, const QByteArray& key )
{
    QByteArray res( src );

    int kLen = key.length();
    int rLen = res.length();
    int k = 0;

    for( int v = 0; v < rLen; v++ )
    {
        res[ v ] = res.at( v ) ^ key.at( k );
        k = ( ++k < kLen ? k : 0 );
    }

    return res;
}

/*static*/ QPoint EWAApplication::getScreensRange()
{
    QDesktopWidget desktop;
    if( m_pntScreensRange == QPoint( -1, -1 ) )
    {
        for( int i = 0; i < desktop.numScreens(); i++ )
        {
            QRect currentScreenRect = desktop.screenGeometry( i );
            m_pntScreensRange.setX( m_pntScreensRange.x() + currentScreenRect.width() );
            m_pntScreensRange.setY( m_pntScreensRange.y() + currentScreenRect.height() );
        }
    }

    return m_pntScreensRange;
}

/*static*/ QString EWAApplication::getAppCachePath()
{
    return EWAApplication::settings()->getCacheFolder();
}

/*static*/ qint64 EWAApplication::getAppCacheSizeMax()
{
    return EWAApplication::settings()->m_pGroupCache->getLimit()*1024*1024;
}

/*static*/ void EWAApplication::makeWidgetCentered( QWidget *pWidget, QWidget *pBaseWidget )
{
    if( !pWidget )
    {
        return;
    }

    QRect baseRect;
    if( !pBaseWidget )
    {
        baseRect = getScreenSize( QCursor::pos() );
    }
    else
    {
        baseRect = pBaseWidget->rect();
        baseRect.setTopLeft( pBaseWidget->mapToGlobal( baseRect.topLeft() ) );
        baseRect.setBottomRight( pBaseWidget->mapToGlobal( baseRect.bottomRight() ) );
    }

    QRect widgetGeometry = pWidget->geometry();
    widgetGeometry.moveCenter( baseRect.center() );

    pWidget->setGeometry( widgetGeometry );
}

/*static*/ bool EWAApplication::moveStuffFoldersToUserDir()
{
    QString strRulesPathDst = EWAApplication::settings()->getDefaultESLFile();
    if( !QFileInfo( strRulesPathDst ).exists() )
    {
        QString strBinaryFilePath = QApplication::applicationFilePath();
        QFileInfo appBinaryFileInfo( strBinaryFilePath );
        if( appBinaryFileInfo.exists() )
        {
            if( appBinaryFileInfo.isSymLink() )
            {
                strBinaryFilePath = appBinaryFileInfo.symLinkTarget();
                appBinaryFileInfo = QFileInfo( strBinaryFilePath );
            }
        }

        QString strRulesPathSrc = appBinaryFileInfo.absolutePath() + QLatin1String( "/rules/ewa.esl" );
        if( !QFileInfo( strRulesPathSrc ).exists() )
        {
            return false;
        }

        QFileInfo dstRulesFileInfo( strRulesPathDst );
        QDir dstRulesFileDir( dstRulesFileInfo.absolutePath() );
        if( !dstRulesFileDir.exists() )
        {
            if( !dstRulesFileDir.mkpath( dstRulesFileInfo.absolutePath() ) )
            {
                return false;
            }
        }

        return QFile::copy( strRulesPathSrc, strRulesPathDst );
    }
    return true;
}

/*static*/ EWASplashScreen* EWAApplication::splashScreen()
{
    if( !m_pSplashScreen )
    {
        m_pSplashScreen = new EWASplashScreen();
    }
    return m_pSplashScreen;
}

/*static*/ void EWAApplication::slotShowSplashMessage( const QString& msg )
{
    if( EWAApplication::splashScreen()->isVisible() )
    {
        EWAApplication::splashScreen()->showMessage( msg, Qt::AlignBottom|Qt::AlignHCenter, Qt::white );
        qApp->processEvents();
    }
}


/*static*/ void EWAApplication::initKeyNames()
{
    if( m_KeyNames.isEmpty() )
    {
        m_KeyNames.insert( "[Escape]", 0x01000000 );
        m_KeyNames.insert( "[Tab]", 0x01000001 );
        m_KeyNames.insert( "[Backtab]", 0x01000002 );
        m_KeyNames.insert( "[Backspace]", 0x01000003 );
        m_KeyNames.insert( "[Return]", 0x01000004 );
        m_KeyNames.insert( "[Enter]", 0x01000005 );
        m_KeyNames.insert( "[Insert]", 0x01000006 );
        m_KeyNames.insert( "[Delete]", 0x01000007 );
        m_KeyNames.insert( "[Pause]", 0x01000008 );
        m_KeyNames.insert( "[Print]", 0x01000009 );
        m_KeyNames.insert( "[SysReq]", 0x0100000a );
        m_KeyNames.insert( "[Clear]", 0x0100000b );
        m_KeyNames.insert( "[Home]", 0x01000010 );
        m_KeyNames.insert( "[End]", 0x01000011 );
        m_KeyNames.insert( "[Left]", 0x01000012 );
        m_KeyNames.insert( "[Up]", 0x01000013 );
        m_KeyNames.insert( "[Right]", 0x01000014 );
        m_KeyNames.insert( "[Down]", 0x01000015 );
        m_KeyNames.insert( "[PageUp]", 0x01000016 );
        m_KeyNames.insert( "[PageDown]", 0x01000017 );
        m_KeyNames.insert( "[Shift]", 0x01000020 );
        m_KeyNames.insert( "[Control]", 0x01000021 );
        m_KeyNames.insert( "[Meta]", 0x01000022 );
        m_KeyNames.insert( "[Alt]", 0x01000023 );
        m_KeyNames.insert( "[AltGr]", 0x01001103 );
        m_KeyNames.insert( "[CapsLock]", 0x01000024 );
        m_KeyNames.insert( "[NumLock]", 0x01000025 );
        m_KeyNames.insert( "[ScrollLock]", 0x01000026 );
        m_KeyNames.insert( "[F1]", 0x01000030 );
        m_KeyNames.insert( "[F2]", 0x01000031 );
        m_KeyNames.insert( "[F3]", 0x01000032 );
        m_KeyNames.insert( "[F4]", 0x01000033 );
        m_KeyNames.insert( "[F5]", 0x01000034 );
        m_KeyNames.insert( "[F6]", 0x01000035 );
        m_KeyNames.insert( "[F7]", 0x01000036 );
        m_KeyNames.insert( "[F8]", 0x01000037 );
        m_KeyNames.insert( "[F9]", 0x01000038 );
        m_KeyNames.insert( "[F10]", 0x01000039 );
        m_KeyNames.insert( "[F11]", 0x0100003a );
        m_KeyNames.insert( "[F12]", 0x0100003b );
        m_KeyNames.insert( "[F13]", 0x0100003c );
        m_KeyNames.insert( "[F14]", 0x0100003d );
        m_KeyNames.insert( "[F15]", 0x0100003e );
        m_KeyNames.insert( "[F16]", 0x0100003f );
        m_KeyNames.insert( "[F17]", 0x01000040 );
        m_KeyNames.insert( "[F18]", 0x01000041 );
        m_KeyNames.insert( "[F19]", 0x01000042 );
        m_KeyNames.insert( "[F20]", 0x01000043 );
        m_KeyNames.insert( "[F21]", 0x01000044 );
        m_KeyNames.insert( "[F22]", 0x01000045 );
        m_KeyNames.insert( "[F23]", 0x01000046 );
        m_KeyNames.insert( "[F24]", 0x01000047 );
        m_KeyNames.insert( "[F25]", 0x01000048 );
        m_KeyNames.insert( "[F26]", 0x01000049 );
        m_KeyNames.insert( "[F27]", 0x0100004a );
        m_KeyNames.insert( "[F28]", 0x0100004b );
        m_KeyNames.insert( "[F29]", 0x0100004c );
        m_KeyNames.insert( "[F30]", 0x0100004d );
        m_KeyNames.insert( "[F31]", 0x0100004e );
        m_KeyNames.insert( "[F32]", 0x0100004f );
        m_KeyNames.insert( "[F33]", 0x01000050 );
        m_KeyNames.insert( "[F34]", 0x01000051 );
        m_KeyNames.insert( "[F35]", 0x01000052 );
        m_KeyNames.insert( "[Super_L]", 0x01000053 );
        m_KeyNames.insert( "[Super_R]", 0x01000054 );
        m_KeyNames.insert( "[Menu]", 0x01000055 );
        m_KeyNames.insert( "[Hyper_L]", 0x01000056 );
        m_KeyNames.insert( "[Hyper_R]", 0x01000057 );
        m_KeyNames.insert( "[Help]", 0x01000058 );
        m_KeyNames.insert( "[Direction_L]", 0x01000059 );
        m_KeyNames.insert( "[Direction_R]", 0x01000060 );
        m_KeyNames.insert( "[Space]", 0x20 );
        m_KeyNames.insert( "[Any]", Qt::Key_Space );
        m_KeyNames.insert( "[Exclam]", 0x21 );
        m_KeyNames.insert( "[QuoteDbl]", 0x22 );
        m_KeyNames.insert( "[NumberSign]", 0x23 );
        m_KeyNames.insert( "[Dollar]", 0x24 );
        m_KeyNames.insert( "[Percent]", 0x25 );
        m_KeyNames.insert( "[Ampersand]", 0x26 );
        m_KeyNames.insert( "[Apostrophe]", 0x27 );
        m_KeyNames.insert( "[ParenLeft]", 0x28 );
        m_KeyNames.insert( "[ParenRight]", 0x29 );
        m_KeyNames.insert( "[Asterisk]", 0x2a );
        m_KeyNames.insert( "[Plus]", 0x2b );
        m_KeyNames.insert( "[Comma]", 0x2c );
        m_KeyNames.insert( "[Minus]", 0x2d );
        m_KeyNames.insert( "[Period]", 0x2e );
        m_KeyNames.insert( "[Slash]", 0x2f );
        m_KeyNames.insert( "[0]", 0x30 );
        m_KeyNames.insert( "[1]", 0x31 );
        m_KeyNames.insert( "[2]", 0x32 );
        m_KeyNames.insert( "[3]", 0x33 );
        m_KeyNames.insert( "[4]", 0x34 );
        m_KeyNames.insert( "[5]", 0x35 );
        m_KeyNames.insert( "[6]", 0x36 );
        m_KeyNames.insert( "[7]", 0x37 );
        m_KeyNames.insert( "[8]", 0x38 );
        m_KeyNames.insert( "[9]", 0x39 );
        m_KeyNames.insert( "[Colon]", 0x3a );
        m_KeyNames.insert( "[Semicolon]", 0x3b );
        m_KeyNames.insert( "[Less]", 0x3c );
        m_KeyNames.insert( "[Equal]", 0x3d );
        m_KeyNames.insert( "[Greater]", 0x3e );
        m_KeyNames.insert( "[Question]", 0x3f );
        m_KeyNames.insert( "[At]", 0x40 );
        m_KeyNames.insert( "[A]", 0x41 );
        m_KeyNames.insert( "[B]", 0x42 );
        m_KeyNames.insert( "[C]", 0x43 );
        m_KeyNames.insert( "[D]", 0x44 );
        m_KeyNames.insert( "[E]", 0x45 );
        m_KeyNames.insert( "[F]", 0x46 );
        m_KeyNames.insert( "[G]", 0x47 );
        m_KeyNames.insert( "[H]", 0x48 );
        m_KeyNames.insert( "[I]", 0x49 );
        m_KeyNames.insert( "[J]", 0x4a );
        m_KeyNames.insert( "[K]", 0x4b );
        m_KeyNames.insert( "[L]", 0x4c );
        m_KeyNames.insert( "[M]", 0x4d );
        m_KeyNames.insert( "[N]", 0x4e );
        m_KeyNames.insert( "[O]", 0x4f );
        m_KeyNames.insert( "[P]", 0x50 );
        m_KeyNames.insert( "[Q]", 0x51 );
        m_KeyNames.insert( "[R]", 0x52 );
        m_KeyNames.insert( "[S]", 0x53 );
        m_KeyNames.insert( "[T]", 0x54 );
        m_KeyNames.insert( "[U]", 0x55 );
        m_KeyNames.insert( "[V]", 0x56 );
        m_KeyNames.insert( "[W]", 0x57 );
        m_KeyNames.insert( "[X]", 0x58 );
        m_KeyNames.insert( "[Y]", 0x59 );
        m_KeyNames.insert( "[Z]", 0x5a );
        m_KeyNames.insert( "[BracketLeft]", 0x5b );
        m_KeyNames.insert( "[Backslash]", 0x5c );
        m_KeyNames.insert( "[BracketRight]", 0x5d );
        m_KeyNames.insert( "[AsciiCircum]", 0x5e );
        m_KeyNames.insert( "[Underscore]", 0x5f );
        m_KeyNames.insert( "[QuoteLeft]", 0x60 );
        m_KeyNames.insert( "[BraceLeft]", 0x7b );
        m_KeyNames.insert( "[Bar]", 0x7c );
        m_KeyNames.insert( "[BraceRight]", 0x7d );
        m_KeyNames.insert( "[AsciiTilde]", 0x7e );
        m_KeyNames.insert( "[nobreakspace]", 0x0a0 );
        m_KeyNames.insert( "[exclamdown]", 0x0a1 );
        m_KeyNames.insert( "[cent]", 0x0a2 );
        m_KeyNames.insert( "[sterling]", 0x0a3 );
        m_KeyNames.insert( "[currency]", 0x0a4 );
        m_KeyNames.insert( "[yen]", 0x0a5 );
        m_KeyNames.insert( "[brokenbar]", 0x0a6 );
        m_KeyNames.insert( "[section]", 0x0a7 );
        m_KeyNames.insert( "[diaeresis]", 0x0a8 );
        m_KeyNames.insert( "[copyright]", 0x0a9 );
        m_KeyNames.insert( "[ordfeminine]", 0x0aa );
        m_KeyNames.insert( "[guillemotleft]", 0x0ab );
        m_KeyNames.insert( "[notsign]", 0x0ac );
        m_KeyNames.insert( "[hyphen]", 0x0ad );
        m_KeyNames.insert( "[registered]", 0x0ae );
        m_KeyNames.insert( "[macron]", 0x0af );
        m_KeyNames.insert( "[degree]", 0x0b0 );
        m_KeyNames.insert( "[plusminus]", 0x0b1 );
        m_KeyNames.insert( "[twosuperior]", 0x0b2 );
        m_KeyNames.insert( "[threesuperior]", 0x0b3 );
        m_KeyNames.insert( "[acute]", 0x0b4 );
        m_KeyNames.insert( "[mu]", 0x0b5 );
        m_KeyNames.insert( "[paragraph]", 0x0b6 );
        m_KeyNames.insert( "[periodcentered]", 0x0b7 );
        m_KeyNames.insert( "[cedilla]", 0x0b8 );
        m_KeyNames.insert( "[onesuperior]", 0x0b9 );
        m_KeyNames.insert( "[masculine]", 0x0ba );
        m_KeyNames.insert( "[guillemotright]", 0x0bb );
        m_KeyNames.insert( "[onequarter]", 0x0bc );
        m_KeyNames.insert( "[onehalf]", 0x0bd );
        m_KeyNames.insert( "[threequarters]", 0x0be );
        m_KeyNames.insert( "[questiondown]", 0x0bf );
        m_KeyNames.insert( "[Agrave]", 0x0c0 );
        m_KeyNames.insert( "[Aacute]", 0x0c1 );
        m_KeyNames.insert( "[Acircumflex]", 0x0c2 );
        m_KeyNames.insert( "[Atilde]", 0x0c3 );
        m_KeyNames.insert( "[Adiaeresis]", 0x0c4 );
        m_KeyNames.insert( "[Aring]", 0x0c5 );
        m_KeyNames.insert( "[AE]", 0x0c6 );
        m_KeyNames.insert( "[Ccedilla]", 0x0c7 );
        m_KeyNames.insert( "[Egrave]", 0x0c8 );
        m_KeyNames.insert( "[Eacute]", 0x0c9 );
        m_KeyNames.insert( "[Ecircumflex]", 0x0ca );
        m_KeyNames.insert( "[Ediaeresis]", 0x0cb );
        m_KeyNames.insert( "[Igrave]", 0x0cc );
        m_KeyNames.insert( "[Iacute]", 0x0cd );
        m_KeyNames.insert( "[Icircumflex]", 0x0ce );
        m_KeyNames.insert( "[Idiaeresis]", 0x0cf );
        m_KeyNames.insert( "[ETH]", 0x0d0 );
        m_KeyNames.insert( "[Ntilde]", 0x0d1 );
        m_KeyNames.insert( "[Ograve]", 0x0d2 );
        m_KeyNames.insert( "[Oacute]", 0x0d3 );
        m_KeyNames.insert( "[Ocircumflex]", 0x0d4 );
        m_KeyNames.insert( "[Otilde]", 0x0d5 );
        m_KeyNames.insert( "[Odiaeresis]", 0x0d6 );
        m_KeyNames.insert( "[multiply]", 0x0d7 );
        m_KeyNames.insert( "[Ooblique]", 0x0d8 );
        m_KeyNames.insert( "[Ugrave]", 0x0d9 );
        m_KeyNames.insert( "[Uacute]", 0x0da );
        m_KeyNames.insert( "[Ucircumflex]", 0x0db );
        m_KeyNames.insert( "[Udiaeresis]", 0x0dc );
        m_KeyNames.insert( "[Yacute]", 0x0dd );
        m_KeyNames.insert( "[THORN]", 0x0de );
        m_KeyNames.insert( "[ssharp]", 0x0df );
        m_KeyNames.insert( "[division]", 0x0f7 );
        m_KeyNames.insert( "[ydiaeresis]", 0x0ff );
        m_KeyNames.insert( "[Multi_key]", 0x01001120 );
        m_KeyNames.insert( "[Codeinput]", 0x01001137 );
        m_KeyNames.insert( "[SingleCandidate]", 0x0100113c );
        m_KeyNames.insert( "[MultipleCandidate]", 0x0100113d );
        m_KeyNames.insert( "[PreviousCandidate]", 0x0100113e );
        m_KeyNames.insert( "[Mode_switch]", 0x0100117e );
        m_KeyNames.insert( "[Kanji]", 0x01001121 );
        m_KeyNames.insert( "[Muhenkan]", 0x01001122 );
        m_KeyNames.insert( "[Henkan]", 0x01001123 );
        m_KeyNames.insert( "[Romaji]", 0x01001124 );
        m_KeyNames.insert( "[Hiragana]", 0x01001125 );
        m_KeyNames.insert( "[Katakana]", 0x01001126 );
        m_KeyNames.insert( "[Hiragana_Katakana]", 0x01001127 );
        m_KeyNames.insert( "[Zenkaku]", 0x01001128 );
        m_KeyNames.insert( "[Hankaku]", 0x01001129 );
        m_KeyNames.insert( "[Zenkaku_Hankaku]", 0x0100112a );
        m_KeyNames.insert( "[Touroku]", 0x0100112b );
        m_KeyNames.insert( "[Massyo]", 0x0100112c );
        m_KeyNames.insert( "[Kana_Lock]", 0x0100112d );
        m_KeyNames.insert( "[Kana_Shift]", 0x0100112e );
        m_KeyNames.insert( "[Eisu_Shift]", 0x0100112f );
        m_KeyNames.insert( "[Eisu_toggle]", 0x01001130 );
        m_KeyNames.insert( "[Hangul]", 0x01001131 );
        m_KeyNames.insert( "[Hangul_Start]", 0x01001132 );
        m_KeyNames.insert( "[Hangul_End]", 0x01001133 );
        m_KeyNames.insert( "[Hangul_Hanja]", 0x01001134 );
        m_KeyNames.insert( "[Hangul_Jamo]", 0x01001135 );
        m_KeyNames.insert( "[Hangul_Romaja]", 0x01001136 );
        m_KeyNames.insert( "[Hangul_Jeonja]", 0x01001138 );
        m_KeyNames.insert( "[Hangul_Banja]", 0x01001139 );
        m_KeyNames.insert( "[Hangul_PreHanja]", 0x0100113a );
        m_KeyNames.insert( "[Hangul_PostHanja]", 0x0100113b );
        m_KeyNames.insert( "[Hangul_Special]", 0x0100113f );
        m_KeyNames.insert( "[Dead_Grave]", 0x01001250 );
        m_KeyNames.insert( "[Dead_Acute]", 0x01001251 );
        m_KeyNames.insert( "[Dead_Circumflex]", 0x01001252 );
        m_KeyNames.insert( "[Dead_Tilde]", 0x01001253 );
        m_KeyNames.insert( "[Dead_Macron]", 0x01001254 );
        m_KeyNames.insert( "[Dead_Breve]", 0x01001255 );
        m_KeyNames.insert( "[Dead_Abovedot]", 0x01001256 );
        m_KeyNames.insert( "[Dead_Diaeresis]", 0x01001257 );
        m_KeyNames.insert( "[Dead_Abovering]", 0x01001258 );
        m_KeyNames.insert( "[Dead_Doubleacute]", 0x01001259 );
        m_KeyNames.insert( "[Dead_Caron]", 0x0100125a );
        m_KeyNames.insert( "[Dead_Cedilla]", 0x0100125b );
        m_KeyNames.insert( "[Dead_Ogonek]", 0x0100125c );
        m_KeyNames.insert( "[Dead_Iota]", 0x0100125d );
        m_KeyNames.insert( "[Dead_Voiced_Sound]", 0x0100125e );
        m_KeyNames.insert( "[Dead_Semivoiced_Sound]", 0x0100125f );
        m_KeyNames.insert( "[Dead_Belowdot]", 0x01001260 );
        m_KeyNames.insert( "[Dead_Hook]", 0x01001261 );
        m_KeyNames.insert( "[Dead_Horn]", 0x01001262 );
        m_KeyNames.insert( "[Back]", 0x01000061 );
        m_KeyNames.insert( "[Forward]", 0x01000062 );
        m_KeyNames.insert( "[Stop]", 0x01000063 );
        m_KeyNames.insert( "[Refresh]", 0x01000064 );
        m_KeyNames.insert( "[VolumeDown]", 0x01000070 );
        m_KeyNames.insert( "[VolumeMute]", 0x01000071 );
        m_KeyNames.insert( "[VolumeUp]", 0x01000072 );
        m_KeyNames.insert( "[BassBoost]", 0x01000073 );
        m_KeyNames.insert( "[BassUp]", 0x01000074 );
        m_KeyNames.insert( "[BassDown]", 0x01000075 );
        m_KeyNames.insert( "[TrebleUp]", 0x01000076 );
        m_KeyNames.insert( "[TrebleDown]", 0x01000077 );
        m_KeyNames.insert( "[MediaPlay]", 0x01000080 );
        m_KeyNames.insert( "[MediaStop]", 0x01000081 );
        m_KeyNames.insert( "[MediaPrevious]", 0x01000082 );
        m_KeyNames.insert( "[MediaNext]", 0x01000083 );
        m_KeyNames.insert( "[MediaRecord]", 0x01000084 );
        m_KeyNames.insert( "[HomePage]", 0x01000090 );
        m_KeyNames.insert( "[Favorites]", 0x01000091 );
        m_KeyNames.insert( "[Search]", 0x01000092 );
        m_KeyNames.insert( "[Standby]", 0x01000093 );
        m_KeyNames.insert( "[OpenUrl]", 0x01000094 );
        m_KeyNames.insert( "[LaunchMail]", 0x010000a0 );
        m_KeyNames.insert( "[LaunchMedia]", 0x010000a1 );
        m_KeyNames.insert( "[Launch0]", 0x010000a2 );
        m_KeyNames.insert( "[Launch1]", 0x010000a3 );
        m_KeyNames.insert( "[Launch2]", 0x010000a4 );
        m_KeyNames.insert( "[Launch3]", 0x010000a5 );
        m_KeyNames.insert( "[Launch4]", 0x010000a6 );
        m_KeyNames.insert( "[Launch5]", 0x010000a7 );
        m_KeyNames.insert( "[Launch6]", 0x010000a8 );
        m_KeyNames.insert( "[Launch7]", 0x010000a9 );
        m_KeyNames.insert( "[Launch8]", 0x010000aa );
        m_KeyNames.insert( "[Launch9]", 0x010000ab );
        m_KeyNames.insert( "[LaunchA]", 0x010000ac );
        m_KeyNames.insert( "[LaunchB]", 0x010000ad );
        m_KeyNames.insert( "[LaunchC]", 0x010000ae );
        m_KeyNames.insert( "[LaunchD]", 0x010000af );
        m_KeyNames.insert( "[LaunchE]", 0x010000b0 );
        m_KeyNames.insert( "[LaunchF]", 0x010000b1 );
        m_KeyNames.insert( "[MediaLast]", 0x0100ffff );
        m_KeyNames.insert( "[unknown]", 0x01ffffff );
        m_KeyNames.insert( "[Call]", 0x01100004 );
        m_KeyNames.insert( "[Context1]", 0x01100000 );
        m_KeyNames.insert( "[Context2]", 0x01100001 );
        m_KeyNames.insert( "[Context3]", 0x01100002 );
        m_KeyNames.insert( "[Context4]", 0x01100003 );
        m_KeyNames.insert( "[Flip]", 0x01100006 );
        m_KeyNames.insert( "[Hangup]", 0x01100005 );
        m_KeyNames.insert( "[No]", 0x01010002 );
        m_KeyNames.insert( "[Select]", 0x01010000 );
        m_KeyNames.insert( "[Yes]", 0x01010001 );
        m_KeyNames.insert( "[Execute]", 0x01020003 );
        m_KeyNames.insert( "[Printer]", 0x01020002 );
        m_KeyNames.insert( "[Play]", 0x01020005 );
        m_KeyNames.insert( "[Sleep]", 0x01020004 );
        m_KeyNames.insert( "[Zoom]", 0x01020006 );
        m_KeyNames.insert( "[Cancel]", 0x01020001 );
    }
}

/*static*/ QString EWAApplication::getKeyName( int iKey )
{
    if( m_KeyNames.isEmpty() )
    {
        initKeyNames();
    }

    return m_KeyNames.key( iKey );
}

bool EWAApplication::setQtLocalization( const QString& name )
{
    QString localizationsDirPath = EWAApplication::settings()->getTranslationsFolder();
    return m_pTranslator->load( "qt_"+name, localizationsDirPath );
}

bool EWAApplication::setEWALocalization( const QString& name )
{
    QString localizationsDirPath = EWAApplication::settings()->getTranslationsFolder();
    return m_pTranslator->load( "ewa_"+name, localizationsDirPath );
}

QString EWAApplication::setDefaultLocalization()
{
    setLocalization( QLocale::system().name() );

    return QLocale::languageToString( QLocale::system().language() );
}

void EWAApplication::setLocalization( const QString& locale )
{
    QCoreApplication::removeTranslator( m_pTranslator );
    EWAApplication::instance()->setQtLocalization( locale );
    EWAApplication::instance()->setEWALocalization( locale );
    QCoreApplication::installTranslator( m_pTranslator );
    
    m_qstrLocalization = locale;
}

/*static*/ QString EWAApplication::translate( const QObject* pObject, const char* pWord )
{
    if( pObject )
    {
        return QApplication::translate( pObject->metaObject()->className(), pWord, 0, QApplication::UnicodeUTF8);
    }

    return QString();
}

/*static*/ QString EWAApplication::getCurrentLocaleName()
{
    return QLocale::system().name();
}

QString EWAApplication::getBlankPageSourceTemplate() const
{
    return tr( "<html><head></head><body background=\"qrc:/images/notloaded_background.png\">%1</body></html>" );
}
QString EWAApplication::getBlankPageSource() const
{
    return  getBlankPageSourceTemplate().arg( 
        translate( this, tr( "Content not loaded." ).toUtf8().data() ) );
}

/*static*/ void EWAApplication::add2Clipboard( const QString& str )
{
    clipboard()->setText( str, QClipboard::Clipboard );
}

void EWAApplication::slotAppTicked()
{
    emit signalOneTick();
}

/*static*/ int EWAApplication::getOneTickPeriod()
{
    return m_iOneTickPeriod;
}

void EWAApplication::prepareSysTrayUsage()
{
    QString strTrayTitle, strTrayDescr, strTrayBaloonsTitle, strTrayBaloonsDescr;
    bool bSysTray( getTrayIconManager()->isAvailable() )
        ,bSysTrayMsg( getTrayIconManager()->supportBaloons() );
    
    if( !bSysTray )
    {
        strTrayTitle = tr( "Current Desktop Environment does not support system tray.\n" );
        strTrayDescr = tr( "Application's tray icon and it's functionality are disabled.\n" );
    }
    
    if( !bSysTrayMsg )
    {
        strTrayBaloonsTitle = tr( "Native system baloon messages are unavailable.\n" );
        strTrayBaloonsDescr = tr( "You can use only widgets to display sites' content." );
    }
    
    if( !bSysTray || !bSysTrayMsg )
    {
        QString strMsg;
        if( !strTrayTitle.isEmpty() )
        {
            strMsg.append( strTrayTitle );
        }
        if( !strTrayDescr.isEmpty() )
        {
            strMsg.append( strTrayDescr );
        }
        
        if( !strTrayBaloonsTitle.isEmpty() )
        {
            strMsg.append( strTrayBaloonsTitle );
        }
        if( !strTrayBaloonsDescr.isEmpty() )
        {
            strMsg.append( strTrayBaloonsDescr );
        }
        
        if( !strMsg.isEmpty() )
        {
            QMessageBox::information( splashScreen(), applicationName(), strMsg );
        }
    }
}

void EWAApplication::displayInfoMessage( const QString& msg )
{
    if( getMainWindow()->getJSTextDiasplay()->toPlainText().length() > 32000 )
    {
        getMainWindow()->getJSTextDiasplay()->clear();
    }
    
    getMainWindow()->getJSTextDiasplay()->append( QString( "%1 %2" ).arg( QDateTime::currentDateTime().toString( "dd.MM.yy HH:mm:ss:zzz," ) ).arg( msg ) );
}

#include <QDesktopServices>
#include <QtGlobal>
#include <QFileInfo>
#include <QDir>
#include <QMessageBox>
#include <QApplication>
#include <QFileDialog>

/*virtual*/ void EWAApplication::commitData( QSessionManager& manager )
{
    Q_UNUSED( manager );
}

/*static*/ EWASiteThumbnailFileSystem* EWAApplication::siteThumbnailsFS()
{
    if( !m_pSiteThumbnailsFS )
    {
        m_pSiteThumbnailsFS = new EWASiteThumbnailFileSystem();
    }
    
    return m_pSiteThumbnailsFS;
}

/*static*/ QPainter::RenderHints EWAApplication::getRenderHints()
{
    return QPainter::Antialiasing 
    | QPainter::TextAntialiasing;
}


/*static*/ QString EWAApplication::getSitesFolder()
{
    return getSitesManager()->getSitesFolder();
}

/*static*/ QUrl EWAApplication::ensureAbsUrl( const QUrl& url, const QUrl& baseUrl )
{
    if( url.isValid() && url.isRelative() && baseUrl.isValid() )
    {
        return baseUrl.resolved( url );
    }
    
    return url;
}

/*static*/ QString EWAApplication::generateSiteFileName()
{
    QStringList existsNames = QDir( getSitesFolder() ).entryList();
    QString strResUuid = QUuid::createUuid().toString();
    
    while( existsNames.contains( strResUuid ) )
    {
        strResUuid = QUuid::createUuid().toString();
    }
    
    return strResUuid;
}

/*static*/ EWASettings *EWAApplication::settings()
{
    if( !m_pSettings )
    {
        EWAApplication::m_pSettings = new EWASettings( EWAApplication::instance() );
        connect( m_pSettings, SIGNAL( signalSplashScreenMessage(const QString&) ),
            EWAApplication::instance(), SLOT( slotShowSplashMessage(const QString&) ) );
    }
        
    return m_pSettings;
}

/*static*/ bool EWAApplication::isValidFileName( const QString& fn )
{
    return !fn.isEmpty() && QFileInfo( fn ).exists();
}

int EWAApplication::calculateListWidgetsWidth( QListWidget *pListWidget )
{
   int iMaxWidth = 0;
   if( pListWidget )
   {
       QFontMetrics fontMetrics = pListWidget->fontMetrics();
       for( int i = 0; i < pListWidget->count(); i++ )
       {
           QString currentText = pListWidget->item( i )->text();
           iMaxWidth = qMax( iMaxWidth, fontMetrics.width(currentText) );
           //qDebug() <<  currentText <<  fontMetrics.width(currentText);
       }
       QSize iconsSize = pListWidget->iconSize();


       iMaxWidth += iconsSize.width() + 18;//-- spacing between columns, etc
   }
   return iMaxWidth;
}


void EWAApplication::regsterUserActionsReplayer( EWAUserActionsPlayer *pPlayer )
{
    if( pPlayer
        && !s_pGesturePlayers->contains( pPlayer ) )
            s_pGesturePlayers->append( pPlayer );
}

int EWAApplication::getUserActionsReplayerIndex( EWAUserActionsPlayer *pPlayer )
{
    return s_pGesturePlayers->indexOf( pPlayer ) + 1;
}

void EWAApplication::ungetUserActionsReplayerIndex( EWAUserActionsPlayer *pPlayer )
{
    if( pPlayer )
        s_pGesturePlayers->removeAll( pPlayer );
}
