/*******************************************************************************
**
** file: ewa_mainwindow.cpp
**
** class: EWAMainWindow
**
** description:
** Frontend for settings, sessions and event processing.
**
** 09.02.2009
**
** sendevent@gmail.com
**
*******************************************************************************/

#include <QtGui>
#include <QToolBar>
#include <QAction>
#include <QNetworkProxy>
#include <QListWidgetItem>
#include <QDebug>
#include <QStackedWidget>

#include "ewa_mainwindow.h"
#include "ewa_splashscreen.h"
#include "ewa_application.h"
#include "ewa_sitehandle.h"
#include "ewa_networkaccessmanager.h"
#include "ewa_siteslistmanager.h"
#include "ewa_trayiconmanager.h"
#include "ewa_aboutdialog.h"
#include "ewa_updateschecker.h"
#include "ewa_faderwidget.h"
#include "ewa_sitedrawer.h"
#include "ewa_splashscreen.h"


const QString INITIATE_IN_CHANGEEVENT;

#define EWA_LOCAL_STRING( x ) EWAApplication::translate( this, x.toUtf8().data() )


EWAMainWindow::EWAMainWindow( QWidget *parent )
    :QMainWindow( parent )
{
    emit signalShowSplashscreenMessage( tr( "Initializing mainwindow..." ) );
    qApp->processEvents();

    ui.setupUi( this );
    connectStackWidgets();
    
    m_topToolbarsIconsSize = QSize( 48, 48 );
    m_botToolbarsIconsSize = QSize( 24, 24 );

    translateLanguagesComboBox();
    
    m_sitesManagerPtr = EWAApplication::getSitesManager();

    ui.pSitesListManagerLayout->addWidget( ( QWidget* )m_sitesManagerPtr );

    //-- view tab flags:
    m_qstrTrayToolTip = QString();
    
    m_qstrLogFileName = EWAApplication::settings()->getLogFile();
    
    m_qstrTopToolbarActionsTooltipTemplate = "<b>%1</b>:%2%3%4";

    initGui();

    slotSetSkin();
    
//    m_pUpdatesChecker = new EWAUpdatesChecker( this );

    normalizeListviewsWidths();
}

EWAMainWindow::~EWAMainWindow()
{
    delete m_pBottomToolBar;
    delete m_pMinimizeAction;
    delete m_pRestoreAction;
}

void EWAMainWindow::initGui()
{
    emit signalShowSplashscreenMessage( tr( "Initializing UI..." ) );
    qApp->processEvents();

    setWindowFlags( Qt::CustomizeWindowHint|Qt::WindowTitleHint|Qt::WindowSystemMenuHint|Qt::WindowCloseButtonHint );
    
    if( !EWAApplication::getTrayIconManager()->isAvailable() )
    {
        setWindowFlags( windowFlags() | Qt::WindowMinimizeButtonHint );
    }
    setWindowTitle( tr( "EWA v.%1" ).arg( EWAApplication::getCurrentVersion() ) );

    ui.needLogLastMsgCheckBox->setToolTip( tr( "Save recived document to\n%1" ).arg( m_qstrLogFileName ) );
    
    initToolBars();
    initInternalConnections();
    initTrayIcon();
}

void EWAMainWindow::initToolBars()
{
    initTopToolBar();
    initBottomToolBar();
}

void EWAMainWindow::initTopToolBar()
{
    m_pTopToolBar = new QToolBar( this );
    m_pTopToolBar->setContextMenuPolicy( Qt::CustomContextMenu );

    connect( m_pTopToolBar, SIGNAL( customContextMenuRequested(const QPoint&) ),
        this, SLOT( slotHideToolbarContext(const QPoint&) ) );

    m_pTopToolBar->setFloatable( false );
	m_pTopToolBar->setMovable( false );
	m_pTopToolBar->setOrientation( Qt::Horizontal );
	m_pTopToolBar->setToolButtonStyle( Qt::ToolButtonIconOnly );

	m_pTopToolBar->setAllowedAreas( Qt::AllToolBarAreas );
	m_pTopToolBar->setIconSize( m_topToolbarsIconsSize );
	addToolBar( Qt::TopToolBarArea, m_pTopToolBar );

    m_pTopToolBarButtons = new QButtonGroup( this );
    
    createSitesListActionsMenu();
    createSitesListFileMenuButton();
    
    createOptionsActionsMenu();

    createConnectionActionsMenu();

    createEventsActionsMenu();
    
    connect( m_pTopToolBar,SIGNAL( actionTriggered(QAction*) ),
        this, SLOT( slotChangeMainTab(QAction*) ) );
}

void EWAMainWindow::initBottomToolBar()
{
    emit signalShowSplashscreenMessage( tr( "Initializing actions..." ) );
    qApp->processEvents();

    m_pBottomToolBar = new QToolBar( this );
    m_pBottomToolBar->setContextMenuPolicy( Qt::CustomContextMenu );
    connect( m_pBottomToolBar, SIGNAL( customContextMenuRequested(const QPoint&) ),
        this, SLOT( slotHideToolbarContext(const QPoint&) ) );
    m_pBottomToolBar->setFloatable( false );
	m_pBottomToolBar->setMovable( false );
	m_pBottomToolBar->setOrientation( Qt::Horizontal );
	m_pBottomToolBar->setToolButtonStyle( Qt::ToolButtonIconOnly );

	m_pBottomToolBar->setAllowedAreas( Qt::AllToolBarAreas );
	m_pBottomToolBar->setIconSize( m_botToolbarsIconsSize );
	addToolBar( Qt::BottomToolBarArea, m_pBottomToolBar );
    
    QActionGroup *pActsGroup = new QActionGroup( this );
    m_pStartAction = pActsGroup->addAction( QIcon( ":/images/play.png" ), INITIATE_IN_CHANGEEVENT );
    m_pStartAction->setCheckable( true );
    connect( m_pStartAction, SIGNAL( triggered() ),
	    this, SLOT( slotStartSitesDownloading() ) );
	    
    m_pStopAction = pActsGroup->addAction( QIcon( ":/images/stop_playing.png" ), INITIATE_IN_CHANGEEVENT );
    connect( m_pStopAction, SIGNAL( triggered() ),
	    this, SLOT( slotStopSitesDownloading() ) );
	    
    m_pBottomToolBar->addActions( pActsGroup->actions() );
    
    m_pPauseAction = m_pBottomToolBar->addAction( QIcon( ":/images/pause.png" ), INITIATE_IN_CHANGEEVENT );
    m_pPauseAction->setCheckable( true );
    connect( m_pPauseAction, SIGNAL( toggled(bool) ),
	    this, SLOT( slotPauseSitesDownloading(bool) ) );

	m_helpAction = m_pBottomToolBar->addAction( QIcon( ":/images/help.png" ), INITIATE_IN_CHANGEEVENT, this, SLOT( slot_helpAction_triggered( bool ) ) );
	m_helpAction->setShortcut( QKeySequence( Qt::Key_F1 ) );
	m_aboutAction = m_pBottomToolBar->addAction( QIcon( ":/images/about.png" ), INITIATE_IN_CHANGEEVENT, this, SLOT( slot_aboutAction_triggered( bool ) ) );
	m_quitAction = m_pBottomToolBar->addAction( QIcon( ":/images/quit.png" ), INITIATE_IN_CHANGEEVENT, EWAApplication::instance(), SLOT( quit() ) );
	m_quitAction->setShortcut( QKeySequence( Qt::CTRL + Qt::Key_Q ) );

	m_pMinimizeAction = new QAction( INITIATE_IN_CHANGEEVENT, this );
    m_pMinimizeAction->setShortcut( QKeySequence( Qt::CTRL + Qt::Key_M ) );
    connect( m_pMinimizeAction, SIGNAL( triggered() ), this, SLOT( slotProcessMinimizationRequest() ) );

    m_pRestoreAction = new QAction( INITIATE_IN_CHANGEEVENT, this );
    connect( m_pRestoreAction, SIGNAL( triggered() ), this, SLOT( showNormal() ) );

    m_pCloseAllWidgetsAction = new QAction( INITIATE_IN_CHANGEEVENT, this );
    m_pCloseAllWidgetsAction->setData( -1 );
    connect( m_pCloseAllWidgetsAction, SIGNAL( triggered() ), this, SLOT( slotCloseSitesWidgets() ) );
    
    m_pCloseAutohiddenWidgetsAction = new QAction( INITIATE_IN_CHANGEEVENT, this );
    m_pCloseAutohiddenWidgetsAction->setData( 0 );
    connect( m_pCloseAutohiddenWidgetsAction, SIGNAL( triggered() ), this, SLOT( slotCloseSitesWidgets() ) );
    
    m_pCloseTopWidgetsAction = new QAction( INITIATE_IN_CHANGEEVENT, this );
    m_pCloseTopWidgetsAction->setData( 1 );
    connect( m_pCloseTopWidgetsAction, SIGNAL( triggered() ), this, SLOT( slotCloseSitesWidgets() ) );
    
    m_pCloseDesktopWidgetsAction = new QAction( INITIATE_IN_CHANGEEVENT, this );
    m_pCloseDesktopWidgetsAction->setData( 2 );
    connect( m_pCloseDesktopWidgetsAction, SIGNAL( triggered() ), this, SLOT( slotCloseSitesWidgets() ) );
}

void EWAMainWindow::initInternalConnections()
{
    emit signalShowSplashscreenMessage( tr( "Initializing internal connections..." ) );
    qApp->processEvents();

    connect( ( QObject* )m_sitesManagerPtr, SIGNAL( signalSitesListChanged(const QString&) ),
        this, SLOT( slotUpdateSites(const QString&) ) );

    connectStackWidgets();
}

void EWAMainWindow::initTrayIcon()
{
    emit signalShowSplashscreenMessage( tr( "Initializing icon on the tray..." ) );
    qApp->processEvents();

    m_pTrayIconMenu = EWAApplication::getTrayIconManager()->getTrayContextMenu();

    QAction *pSitesListAction = m_pTrayIconMenu->actions().last();

    m_pTrayIconMenu->insertAction( pSitesListAction, m_pMinimizeAction );

    m_pTrayIconMenu->insertAction( pSitesListAction, m_pRestoreAction );
    m_pTrayIconMenu->insertSeparator( pSitesListAction );
    m_pTrayIconMenu->insertAction( pSitesListAction, m_helpAction );
    m_pTrayIconMenu->insertAction( pSitesListAction, m_aboutAction );
    m_pTrayIconMenu->insertSeparator( pSitesListAction );
    m_pTrayIconMenu->addSeparator();
    m_pCloseWidgetsSubMenu = m_pTrayIconMenu->addMenu( INITIATE_IN_CHANGEEVENT );
        m_pCloseWidgetsSubMenu->addAction( m_pCloseAllWidgetsAction );
        m_pCloseWidgetsSubMenu->addAction( m_pCloseDesktopWidgetsAction );
        m_pCloseWidgetsSubMenu->addAction( m_pCloseTopWidgetsAction );
        m_pCloseWidgetsSubMenu->addAction( m_pCloseAutohiddenWidgetsAction );
    m_pTrayIconMenu->addSeparator();
    m_pTrayIconMenu->addAction( m_pStartAction );
    m_pTrayIconMenu->addAction( m_pStopAction );
    m_pTrayIconMenu->addAction( m_pPauseAction );
    m_pTrayIconMenu->addSeparator();
    m_pTrayIconMenu->addAction( m_quitAction );
}

void EWAMainWindow::on_openTargetsListButton_clicked()
{
    m_sitesManagerPtr->loadSitesList();
}

void EWAMainWindow::slotCreateSitesListFile()
{
    m_sitesManagerPtr->createSitesList();
}

/**
** Save/Load our settings.
** Connected to qApp's SIGNAL aboutToQuit() in main.cpp.
*/
void EWAMainWindow::loadSettings()
{
    emit signalShowSplashscreenMessage( tr( "Loading settings..." ) );
    
    initSettingsCache();
    initSettingsProxy();
    initSettingsWebKit();
    
    initSettingsPrefs();
    initSettingsSystray();
    
    //-- it can use OpenFileDialog, so should be called 
    //-- after "show()" (in "showStartup"):
    //-- initSitesList();
    
    qApp->processEvents();
    
    

    prepareSysTrayUsage();
}

void EWAMainWindow::initSettingsCache()
{
    QString strCachePath = EWAApplication::settings()->m_pGroupCache->getStorage();
    if( !EWAApplication::isValidFileName( strCachePath ) )
        strCachePath = EWAApplication::settings()->getCacheFolder();
        
    ui.pCacheDirLineEdit->setText( strCachePath );
    ui.pCacheStorageSizeSpinbox->setValue( EWAApplication::settings()->m_pGroupCache->getLimit() );
    
    connect( ui.pCacheDirLineEdit, SIGNAL( textChanged(const QString&) ),
        EWAApplication::settings()->m_pGroupCache, SLOT(setStorage(const QString&)) );
}

void EWAMainWindow::initSettingsProxy()
{
    connect( EWAApplication::settings()->m_pGroupProxy, SIGNAL(signalSettingsChanged()),
        this, SLOT(updateProxy()) );
        
    ui.proxySupport->setChecked( EWAApplication::settings()->m_pGroupProxy->isEnabled() );
    ui.proxyType->setCurrentIndex( EWAApplication::settings()->m_pGroupProxy->getTypeID() );
    ui.proxyHostName->setText( EWAApplication::settings()->m_pGroupProxy->getHost() );
    ui.proxyPort->setValue( EWAApplication::settings()->m_pGroupProxy->getPort() );
    ui.proxyUserName->setText( EWAApplication::settings()->m_pGroupProxy->getLogin() );
    ui.proxyPassword->setText( EWAApplication::settings()->m_pGroupProxy->getPassword() );
    
    connect( ui.proxySupport, SIGNAL( toggled(bool) ),
        EWAApplication::settings()->m_pGroupProxy, SLOT(setEnabled(bool)) );
    connect( ui.proxyType, SIGNAL( activated(int) ),
        EWAApplication::settings()->m_pGroupProxy, SLOT(setTypeID(int)) );
    connect( ui.proxyHostName, SIGNAL( textChanged(const QString&) ),
        EWAApplication::settings()->m_pGroupProxy, SLOT(setHost(const QString&)) );
    connect( ui.proxyPort, SIGNAL( valueChanged(int) ),
        EWAApplication::settings()->m_pGroupProxy, SLOT(setPort(int)) );
    connect( ui.proxyUserName, SIGNAL( textChanged(const QString&) ),
        EWAApplication::settings()->m_pGroupProxy, SLOT(setLogin(const QString&)) );
    connect( ui.proxyPassword, SIGNAL( textChanged(const QString&) ),
        EWAApplication::settings()->m_pGroupProxy, SLOT(setPassword(const QString&)) );
}

void EWAMainWindow::updateProxy()
{
    QNetworkProxy proxy;
    if( EWAApplication::settings()->m_pGroupProxy->isEnabled() )
    {
        if( EWAApplication::settings()->m_pGroupProxy->getTypeID() == 0 )
            proxy.setType( QNetworkProxy::Socks5Proxy );
        else
            proxy.setType( QNetworkProxy::HttpProxy );

        proxy.setHostName( EWAApplication::settings()->m_pGroupProxy->getHost() );
        proxy.setPort( EWAApplication::settings()->m_pGroupProxy->getPort() );
        proxy.setUser( EWAApplication::settings()->m_pGroupProxy->getLogin() );
        proxy.setPassword( EWAApplication::settings()->m_pGroupProxy->getPassword() );
    }
    
    if( m_sitesManagerPtr )
        m_sitesManagerPtr->setProxyForList( proxy );
}


void EWAMainWindow::initSettingsWebKit()
{
    ui.useJavaCheckBox->setChecked( EWAApplication::settings()->m_pGroupWebkit->getUseJava() );
    ui.useNPAPICheckBox->setChecked( EWAApplication::settings()->m_pGroupWebkit->getUseNPAPI() );
    ui.usePrivateBrowsingCheckBox->setChecked( EWAApplication::settings()->m_pGroupWebkit->getUsePrivateBrowsing() );
    ui.downloadImagesCheckBox->setChecked( EWAApplication::settings()->m_pGroupWebkit->getUseAutogetImages() );
    ui.enableJavascript->setChecked( EWAApplication::settings()->m_pGroupWebkit->getUseJavascript() );
    ui.pShowAlertsCheckBox->setChecked( EWAApplication::settings()->m_pGroupWebkit->getUseJSShowAlerts() );
    ui.jsAccessClipboardCheckBox->setChecked( EWAApplication::settings()->m_pGroupWebkit->getUseJSAccessClipboard() );
    
    QWebSettings::globalSettings()->setAttribute( QWebSettings::JavascriptCanOpenWindows, false );
    QWebSettings::globalSettings()->setAttribute( QWebSettings::DeveloperExtrasEnabled, false );
    QWebSettings::globalSettings()->setAttribute( QWebSettings::LinksIncludedInFocusChain, true );
    QWebSettings::globalSettings()->setAttribute( QWebSettings::ZoomTextOnly, false );
    QWebSettings::globalSettings()->setAttribute( QWebSettings::PrintElementBackgrounds, false );
    QWebSettings::globalSettings()->setAttribute( QWebSettings::OfflineStorageDatabaseEnabled, false );
    //QWebSettings::globalSettings()->setOfflineStoragePath( EWAApplication::settings()->getUserDataFolder() );
    QWebSettings::globalSettings()->setAttribute( QWebSettings::OfflineWebApplicationCacheEnabled, false );
    //QWebSettings::globalSettings()->setOfflineWebApplicationCachePath( EWAApplication::settings()->getUserDataFolder() );
    QWebSettings::globalSettings()->setAttribute( QWebSettings::LocalStorageEnabled, false );
    QWebSettings::globalSettings()->setAttribute( QWebSettings::LocalStorageDatabaseEnabled, false );
    QWebSettings::globalSettings()->setAttribute( QWebSettings::LocalContentCanAccessRemoteUrls, false );
}                                                                              

void EWAMainWindow::on_useJavaCheckBox_toggled( bool on )
{
    EWAApplication::settings()->m_pGroupWebkit->setUseJava( on );
    QWebSettings::globalSettings()->setAttribute( QWebSettings::JavaEnabled, on );
}
void EWAMainWindow::on_useNPAPICheckBox_toggled( bool on )
{
    EWAApplication::settings()->m_pGroupWebkit->setUseNPAPI( on );
    QWebSettings::globalSettings()->setAttribute( QWebSettings::PluginsEnabled, on );
}
void EWAMainWindow::on_usePrivateBrowsingCheckBox_toggled( bool on )
{
    EWAApplication::settings()->m_pGroupWebkit->setUsePrivateBrowsing( on );
    QWebSettings::globalSettings()->setAttribute( QWebSettings::PrivateBrowsingEnabled, on );
}
void EWAMainWindow::on_downloadImagesCheckBox_toggled( bool on )
{
    EWAApplication::settings()->m_pGroupWebkit->setUseAutogetImages( on );
    QWebSettings::globalSettings()->setAttribute( QWebSettings::AutoLoadImages, on );
}
void EWAMainWindow::on_enableJavascript_toggled( bool on )
{
    EWAApplication::settings()->m_pGroupWebkit->setUseJavascript( on );
    QWebSettings::globalSettings()->setAttribute( QWebSettings::JavascriptEnabled, on );
}
void EWAMainWindow::on_pShowAlertsCheckBox_toggled( bool on )
{
    EWAApplication::settings()->m_pGroupWebkit->setUseJSShowAlerts( on );
}
void EWAMainWindow::on_jsAccessClipboardCheckBox_toggled( bool on )
{
    EWAApplication::settings()->m_pGroupWebkit->setUseJSAccessClipboard( on );
    QWebSettings::globalSettings()->setAttribute( QWebSettings::JavascriptCanAccessClipboard, on );
}

void EWAMainWindow::initSettingsPrefs()
{
    connect( ui.autoconnectCheckBox, SIGNAL(toggled(bool)),
        EWAApplication::settings()->m_pGroupPreferences, SLOT(setAutostartDownload(bool)) );
    connect( ui.pUpdatesCheckBox, SIGNAL(toggled(bool)),
        EWAApplication::settings()->m_pGroupPreferences, SLOT(setCheckForUpdates(bool)) );
    connect( ui.insertInClipboardCheckBox, SIGNAL(toggled(bool)),
        EWAApplication::settings()->m_pGroupPreferences, SLOT(setCopyLastMessage(bool)) );
    connect( ui.needLogLastMsgCheckBox, SIGNAL(toggled(bool)),
        EWAApplication::settings()->m_pGroupPreferences, SLOT(setSaveMessageToLog(bool)) );
    connect( ui.closeMsgCheckBox, SIGNAL(toggled(bool)),
        EWAApplication::settings()->m_pGroupPreferences, SLOT(setWarnOnCLose(bool)) );
        
    ui.autoconnectCheckBox->setChecked( EWAApplication::settings()->m_pGroupPreferences->getAutostartDownload() );
    ui.pUpdatesCheckBox->setChecked( EWAApplication::settings()->m_pGroupPreferences->getCheckForUpdates() );
    ui.insertInClipboardCheckBox->setChecked( EWAApplication::settings()->m_pGroupPreferences->needCopyLastMessage() );
    ui.needLogLastMsgCheckBox->setChecked( EWAApplication::settings()->m_pGroupPreferences->needSaveMessageToLog() );
    ui.closeMsgCheckBox->setChecked( EWAApplication::settings()->m_pGroupPreferences->needWarnOnCLose() );
    int iLangId = EWAApplication::settings()->m_pGroupPreferences->getLangId();
    ui.langComboBox->setCurrentIndex( iLangId );
    on_langComboBox_activated( iLangId );
    
    
}
void EWAMainWindow::initSettingsSystray()
{
    QString strIconFileName = EWAApplication::settings()->m_pGroupTrayIcon->getFileName();
    if( !EWAApplication::isValidFileName( strIconFileName ) )
        strIconFileName = EWAApplication::settings()->getDefaultIconFile();
        
    setIcon( strIconFileName );
    ui.toolTipGroupBox->setChecked( EWAApplication::settings()->m_pGroupTrayIcon->getUseToolTip() );
    ui.tooltipLineEdit->setText( EWAApplication::settings()->m_pGroupTrayIcon->getToolTip() );
    
    connect( ui.toolTipGroupBox, SIGNAL( toggled(bool) ),
        EWAApplication::settings()->m_pGroupTrayIcon, SLOT( setUseToolTip(bool) ) );
    connect( ui.tooltipLineEdit, SIGNAL( textChanged(const QString&) ),
        EWAApplication::settings()->m_pGroupTrayIcon, SLOT( setToolTip(const QString&) ) );
}
        
void EWAMainWindow::initSitesList()
{
    on_parallelRadioButton_toggled( EWAApplication::settings()->m_pGroupSites->getDownloadSimultaneously() );
    on_seriallyRadioButton_toggled( !EWAApplication::settings()->m_pGroupSites->getDownloadSimultaneously() );
    
    QString strSitesListFileName = EWAApplication::settings()->m_pGroupSites->getFileName();
    
    if( !EWAApplication::isValidFileName( strSitesListFileName  ) )
        strSitesListFileName = EWAApplication::settings()->getDefaultESLFile();
    
    ui.listFileNameLineEdit->setText( strSitesListFileName );
    m_sitesManagerPtr->loadSitesList( strSitesListFileName );
}

void EWAMainWindow::updateUiWhileLoadingSitesList( bool bLoadFinished )
{
    m_sitesManagerPtr->setEnabled( bLoadFinished );
    m_pStartAction->setEnabled( bLoadFinished );
    m_pStopAction->setEnabled( bLoadFinished );
    m_pPauseAction->setEnabled( bLoadFinished );
    /*m_pCloseAllWidgetsAction->setEnabled( bLoadFinished );
    m_pCloseDesktopWidgetsAction->setEnabled( bLoadFinished );
    m_pCloseTopWidgetsAction->setEnabled( bLoadFinished );
    m_pCloseAutohiddenWidgetsAction->setEnabled( bLoadFinished );*/
    ui.openTargetsListButton->setEnabled( bLoadFinished );
}

void EWAMainWindow::saveSettings()
{
	if( isStarted() )//-- stop:
    {
		m_pStopAction->setChecked( true );
    }
//    m_pUpdatesChecker->stopChecking();
    
    //-- direct update mainwindow's geometry
    //-- in settings storage:
    EWAApplication::settings()->m_pGroupApp->setLastSize( size() );
    EWAApplication::settings()->m_pGroupApp->setLastPos( pos() );
    
    int tab, subTab;
    getCurrentTubAndSubtab( tab, subTab );
    EWAApplication::settings()->m_pGroupApp->setTab( tab );
    EWAApplication::settings()->m_pGroupApp->setSubTab( subTab );
    
    EWAApplication::settings()->save();   
}



void EWAMainWindow::checkUpdatesDelayed( int delay )
{
    QTimer::singleShot( delay, this, SLOT( slotCheckForUpdates() ) );
}

void EWAMainWindow::slotCheckForUpdates()
{
//    m_pUpdatesChecker->setUsed( true );
}

int EWAMainWindow::versionToInt( const QString& strVer ) const
{
    int iRes = -1;
    if( strVer.isEmpty() )
    {
        return iRes;
    }
    
    QRegExp rx( "(\\d+)\\.(\\d+)\\.(\\d+)" );
    QString currentMatched;
    if( rx.isValid() )
    {
        int pos = 0;
        while( ( pos = rx.indexIn( strVer, pos ) ) != -1 )
        {
            int numCaps = rx.numCaptures();
            for( int c = 1; c <= numCaps; c++ )
            {
                currentMatched += rx.cap( c );
            }
            int capLength = rx.matchedLength();
            pos += capLength;
            if( pos == strVer.length() || !pos || capLength < 0 )
                break;
        }
    }
    else
    {
        return iRes;
    }
    
    if( !currentMatched.isEmpty() )
    {
        int iConverted = currentMatched.toInt();
        if( iConverted > 0 )
        {
            iRes = iConverted;
        }
    }
    
    return iRes;
}

void EWAMainWindow::processUpdatesNeeded( QString& prev ) const
{
    if( prev.isEmpty() )
    {
        return;
    }
    
    int iSaved = versionToInt( prev );
    int iCurrent = versionToInt( EWAApplication::getCurrentVersion() );
    
    if( iSaved < iCurrent )
    {
        prev = EWAApplication::getCurrentVersion();
    }
}

void EWAMainWindow::slotStartSitesDownloading()
{
    if( m_sitesManagerPtr->startSites() )
    {
        updateTrayIconToolTip();
        
        ui.openTargetsListButton->setDisabled( true );
        ui.pDownloadTypePage->setDisabled( true );
    }
    else
    {
        if( !m_sitesManagerPtr->isPaused() && !m_sitesManagerPtr->isStarted() )
        {
            m_pStartAction->setChecked( false );
        }
    }
}

void EWAMainWindow::slotSetSkin( const QString& /*fn*/ )
{
    qApp->setStyleSheet( "QMessageBox {messagebox-text-interaction-flags: 5;}QToolTip {background-color: #005990;border-color: white;border-width: 1px;border-style: outset;padding: 3px;color: #fefefe;border-radius: 3px;opacity: 210;}" );
}


void EWAMainWindow::slotStopSitesDownloading()
{
    m_pStartAction->setChecked( false );
    m_pPauseAction->setChecked( false );
    
    m_sitesManagerPtr->stopSites();
    updateTrayIconToolTip();
    
    ui.openTargetsListButton->setEnabled( true );
    ui.pDownloadTypePage->setEnabled( true );
}

void EWAMainWindow::slotPauseSitesDownloading( bool on )
{
    if( m_sitesManagerPtr->isStarted() )
    {
        m_sitesManagerPtr->pauseSites( on );
        updateTrayIconToolTip();
        
        ui.openTargetsListButton->setDisabled( true );
        ui.pDownloadTypePage->setDisabled( true );
    }
    else
    {
        m_pPauseAction->setChecked( false );
    }
}
        
void EWAMainWindow::slot_helpAction_triggered( bool checked )
{
    Q_UNUSED( checked );

    QMessageBox::information( this, tr( "Help" ), tr( "Not implemented yet, sorry." ) );
}

void EWAMainWindow::slot_aboutAction_triggered( bool checked )
{
    Q_UNUSED( checked );
    EWAAboutDialog *pAboutDlg = new EWAAboutDialog( 0 );
    pAboutDlg->setAttribute( Qt::WA_DeleteOnClose );
    EWAApplication::makeWidgetCentered( pAboutDlg );
    pAboutDlg->show();
}

void EWAMainWindow::slotHideToolbarContext( const QPoint& pnt )
{
    Q_UNUSED( pnt );
}

void EWAMainWindow::setIcon( const QString& fileName )
{
    if( EWAApplication::getTrayIconManager()->isAvailable() )
    {
        QSize iconSize;
        if( !fileName.isEmpty() )
        {
            EWAApplication::settings()->m_pGroupTrayIcon->setFileName( fileName );

            m_icon = QIcon( fileName );
            if( m_icon.isNull() )
            {
                QMessageBox::critical( this, tr( "Set icon" ), tr( "Can't load image from file:\n[%1]" )
                        .arg( EWAApplication::settings()->m_pGroupTrayIcon->getFileName() ) );
            }
            else
            {
                iconSize = QSize( 22, 22 );
            }
        }

        if( m_icon.isNull() )
        {
            return setIcon( EWAApplication::settings()->getDefaultIconFile() );
        }
        else
            ui.iconLabel->setPixmap( m_icon.pixmap( iconSize ) );

        EWAApplication::getTrayIconManager()->setIcon( m_icon );
        this->setWindowIcon( m_icon );
        qApp->setWindowIcon( m_icon );
    }
}

void EWAMainWindow::on_iconPushButton_clicked()
{
    if( EWAApplication::getTrayIconManager()->isAvailable() )
    {
        QString imgExt = EWAApplication::getImgFormats().join( "\n" );
	    imgExt = imgExt.append( "\n*.*" );
	    
	    QString currentPath = EWAApplication::settings()->m_pGroupTrayIcon->getFileName();
	    QString currentExt = QFileInfo( currentPath ).suffix();
        if( currentExt.isEmpty() )
            currentExt = "*.*";
        else
            currentExt = currentExt.prepend( "*." );

        if( !currentPath.compare( EWAApplication::settings()->getDefaultIconFile() ) )
        {
            currentPath = EWAApplication::settings()->getIconsCollectionFolder();
        }

	    QString imgFile = QFileDialog::getOpenFileName( this, tr( "Select icon file" ),
	        currentPath, imgExt, &currentExt );

        if( !imgFile.isEmpty() )
        {
            setIcon( imgFile );
        }
    }
}

void EWAMainWindow::on_clearIconButton_clicked()
{
    if( EWAApplication::getTrayIconManager()->isAvailable() )
    {
        setIcon( EWAApplication::settings()->getDefaultIconFile() );
    }
}


void EWAMainWindow::on_toolTipGroupBox_toggled( bool on )
{
    if( EWAApplication::getTrayIconManager()->isAvailable() )
    {
        ui.toolTipGroupBox->setChecked( on );
        updateTrayIconToolTip();
    }
}

void EWAMainWindow::updateTrayIconToolTip()
{
    if( EWAApplication::getTrayIconManager()->isAvailable() )
    {
        if( !ui.toolTipGroupBox->isChecked() )
        {
            return setTrayToolTip( "" );
        }

        QString newToolTip = ui.tooltipLineEdit->text();

        setTrayToolTip( newToolTip );
    }
}

void EWAMainWindow::setTrayToolTip( const QString& toolTip )
{
    if( EWAApplication::getTrayIconManager()->isAvailable() )
    {
        m_qstrTrayToolTip = toolTip;
        QString strText = m_qstrTrayToolTip;
        if( !strText.compare( QLatin1String( "%EWA%" ), Qt::CaseInsensitive ) )
        {
            strText = EWAApplication::applicationName() + " " + EWAApplication::applicationVersion();
        }
        EWAApplication::getTrayIconManager()->setToolTip( tr( "%1" ).arg( strText ) );
    }
}

void EWAMainWindow::on_tooltipLineEdit_textChanged( const QString& )
{
    updateTrayIconToolTip();
}







void EWAMainWindow::on_pUpdatesCheckBox_toggled( bool on )
{
//    m_pUpdatesChecker->setUsed( on );
}

void EWAMainWindow::getCurrentTubAndSubtab( int& iTab, int& iSubTab ) const
{
    iTab = ui.pTopStackedWidget->currentIndex();
    switch( iTab )
    {
        case 0:
        {
            iSubTab = ui.pSitesListWidget->currentRow();
            break;
        }
        case 1:
        {
            iSubTab = ui.pOptionsListWidget->currentRow();
            break;
        }
        case 2:
        {
            iSubTab = ui.pConnectionListWidget->currentRow();
            break;
        }
        default:
        {
            iSubTab = 0;
            break;
        }
    }
}
void EWAMainWindow::setCurrentTubAndSubtab( const int iTab, const int iSubTab )
{
    ui.pTopStackedWidget->setCurrentIndex( iTab );
    
    void (EWAMainWindow::*pt2SubtabSelector)(int) = NULL;
    
    switch( iTab )
    {
        case 0:
        {
            pt2SubtabSelector = &EWAMainWindow::showSitesListSubTab;
            break;
        }
        case 1:
        {
            pt2SubtabSelector = &EWAMainWindow::showOptionsSubTab;
            break;
        }
        case 2:
        {
            pt2SubtabSelector = &EWAMainWindow::showConnectionSubTab;
            break;
        }
        default:
        {
            m_pEventsActionToolButton->setChecked( true );
            break;
        }
    }
    
    if( pt2SubtabSelector )
    {
        (this->*pt2SubtabSelector)( iSubTab );
    }
}
        
void EWAMainWindow::showStartup()
{
    emit signalShowSplashscreenMessage( tr( "Starting up..." ) );
    qApp->processEvents();

    if( EWAApplication::settings()->m_pGroupApp->isHidden() )
        showMinimized();
    else
        showNormal();
    
    setCurrentTubAndSubtab( EWAApplication::settings()->m_pGroupApp->getTab(), EWAApplication::settings()->m_pGroupApp->getSubTab() );
    
    QSize oldSz = EWAApplication::settings()->m_pGroupApp->getLastSize();
    if( !oldSz.isEmpty() )
        resize( oldSz );
    
    QPoint oldPos = EWAApplication::settings()->m_pGroupApp->getLastPos();
    if( !oldPos.isNull() )
        move( oldPos );
    
    initSitesList();
    
    updateTrayIconToolTip();
    greetUser();

    if( EWAApplication::settings()->m_pGroupPreferences->getAutostartDownload() )
    {
        m_pStartAction->setChecked( EWAApplication::settings()->m_pGroupPreferences->getAutostartDownload() );
        slotStartSitesDownloading();
    }
    
    translateActionsText();

    qApp->processEvents();
}

void EWAMainWindow::greetUser() const
{
    if( !EWAApplication::settings()->m_pGroupApp->isHidden() )
    {
        return;
    }

    qApp->processEvents();

    if( EWAApplication::getTrayIconManager()->supportBaloons() )
    {
        EWAApplication::getTrayIconManager()->showMessageBalloon(
            tr( "EWA" ),
            tr( "Started in minimized mode." ) );
    }
    qApp->processEvents();
}

void EWAMainWindow::showMinimized()
{
    QMainWindow::showMinimized();
    hide();
    m_pMinimizeAction->setEnabled( false );
    m_pRestoreAction->setEnabled( true );
}

void EWAMainWindow::show()
{
    QMainWindow::showNormal();
    EWAApplication::makeWidgetCentered( this );
}

void EWAMainWindow::showNormal()
{
    this->show();
    
    //-- in KDE mainwindow does not shown if
    //-- app was in "minimized" state at prev quit.
    //-- yet, just test:
    if( isHidden() )
    {
        QMainWindow::show();
    }
    
    this->setFocus();
    m_pMinimizeAction->setEnabled( true );
    m_pRestoreAction->setEnabled( false );
}

void EWAMainWindow::closeEvent( QCloseEvent *e )
{
    Q_UNUSED( e );
    
    slotProcessMinimizationRequest();
}

void EWAMainWindow::changeEvent( QEvent * event )
{
    if( event->type() == QEvent::LanguageChange )
    {
        ui.retranslateUi( this );
        
        ui.needLogLastMsgCheckBox->setToolTip( tr( "Save recived document to\n%1" ).arg( m_qstrLogFileName ) );
        translateLanguagesComboBox();
        translateActionsText();
        normalizeListviewsWidths();

        EWAApplication::getTrayIconManager()->translateCloseMenuAction();
    }

    return QWidget::changeEvent( event );
}

void EWAMainWindow::slotUpdateSites( const QString& fileName )
{
    ui.listFileNameLineEdit->setText( fileName );
    QString toolTip = tr( "Sites list file name" );
    if( !fileName.isEmpty() )
    {
        toolTip.append( tr( ":\n%1" ).arg( fileName ) );
    }
    
    EWAApplication::settings()->m_pGroupSites->setFileName( fileName );
    
    ui.listFileNameLineEdit->setToolTip( toolTip );
}

void EWAMainWindow::slotCloseSitesWidgets()
{
    QAction *pAction = qobject_cast<QAction*>( sender() );

    if( pAction )
    {
        EWAApplication::getSitesManager()->closeSiteWidgets( pAction->data().toInt() );
    }
}

void EWAMainWindow::on_pCacheDirLineEdit_textChanged( const QString& path )
{
    if( !path.isEmpty() )
    {
        EWAApplication::getSitesManager()->setCacheDir( path );
    }
}

void EWAMainWindow::on_pChangeCacheDirButton_clicked()
{
    QString currentCachePath = ui.pCacheDirLineEdit->text();
    QString newCachePath = QFileDialog::getExistingDirectory( this, tr( "Choose new cache storage directory" ), currentCachePath );
    
    if( !newCachePath.isEmpty() )
    {
        ui.pCacheDirLineEdit->setText( newCachePath );
        ui.pCacheDirLineEdit->setToolTip( newCachePath );
    }
}

void EWAMainWindow::on_pCacheStorageSizeSpinbox_valueChanged( int val )
{
    if( !val )
    {
        QWebSettings::clearMemoryCaches();
        QWebSettings::setMaximumPagesInCache( 0 );
    }
    else
    {
        QWebSettings::setMaximumPagesInCache( 16 );
    }
    EWAApplication::settings()->m_pGroupCache->setLimit( val );
    EWAApplication::getSitesManager()->setCacheCapacity( 1024*1024*val );
}

void EWAMainWindow::slotChangeMainTab( QAction *pAction )
{
    if( pAction )
    {
        int iNum = pAction->data().toInt();
        if( iNum >= 0 && iNum <= 3 )
        {
            ui.pTopStackedWidget->setCurrentIndex( iNum );
        }
    }
}

void EWAMainWindow::on_parallelRadioButton_toggled( bool on )
{
    if( on )
    {
        m_sitesManagerPtr->setDownloadTypeParallely();
    }
    else
    {
        m_sitesManagerPtr->setDownloadTypeSerial();
    }
    
    EWAApplication::settings()->m_pGroupSites->setDownloadSimultaneously( on );
}

void EWAMainWindow::on_seriallyRadioButton_toggled( bool on )
{
    if( on )
    {
        m_sitesManagerPtr->setDownloadTypeSerial();
    }
    else
    {
        m_sitesManagerPtr->setDownloadTypeParallely();
    }
    
    EWAApplication::settings()->m_pGroupSites->setDownloadSimultaneously( !on );
}

int EWAMainWindow::on_langComboBox_activated( int iRow )
{
    EWAApplication::settings()->m_pGroupPreferences->setLangId( iRow );
    
    QString strLangName;
    if( iRow < 0 || iRow >= ui.langComboBox->count() )
    {
        strLangName = EWAApplication::instance()->setDefaultLocalization();
    }
    else
    {
        strLangName = ui.langComboBox->itemText( iRow );
        
        QLocale::Language localLanguage( (QLocale::Language)ui.langComboBox->itemData( iRow ).toInt() );
        QLocale dstLocale( localLanguage );
        EWAApplication::instance()->setLocalization( dstLocale.name() );
    }
    
    return ui.langComboBox->findText( strLangName );
}

void EWAMainWindow::translateLanguagesComboBox()
{
    if( ui.langComboBox->count() )
    {
        return;
    }
    ui.langComboBox->clear();
    QString localizationsDirPath = EWAApplication::settings()->getTranslationsFolder();
    QDir localizationsDir( localizationsDirPath );
    QFileInfoList translationsList = localizationsDir.entryInfoList( QDir::Files|QDir::NoSymLinks|QDir::NoDotAndDotDot|QDir::Readable );

    for( int i = 0; i < translationsList.count(); i++ )
    {
        QString languageName;
        QString fileName = translationsList.at( i ).baseName();
        QLocale::Language iLocaleLanguage;
        if( fileName.startsWith( QLatin1String( "ewa_" ) )  )
        {
            fileName = fileName.right( fileName.length()-4 );
            iLocaleLanguage = QLocale( fileName ).language();
            languageName = QLocale::languageToString( iLocaleLanguage );
        }
        else
        {
            continue;
        }

        ui.langComboBox->addItem( tr( "%1" ).arg( languageName ) );
        ui.langComboBox->setItemData( ui.langComboBox->count()-1, iLocaleLanguage );
    }

    if( !ui.langComboBox->count() )
    {
        ui.langComboBox->addItem( "English" );
    }
}

void EWAMainWindow::translateActionsText()
{
    m_pCloseWidgetsSubMenu->setTitle( EWA_LOCAL_STRING( tr( "Close Widgets" ) ) );
    m_pStartAction->setText( EWA_LOCAL_STRING( tr( "Start" ) ) );
    m_pStopAction->setText( EWA_LOCAL_STRING( tr( "Stop" ) ) );
    m_pPauseAction->setText( EWA_LOCAL_STRING( tr( "Pause" ) ) );
    
    m_helpAction->setText( EWA_LOCAL_STRING( tr( "Help" ) ) );
    m_aboutAction->setText( EWA_LOCAL_STRING( tr( "About" ) ) );
    m_quitAction->setText( EWA_LOCAL_STRING( tr( "Quit" ) ) );
	m_pMinimizeAction->setText( EWA_LOCAL_STRING( tr( "Hide" ) ) );
    m_pRestoreAction->setText( EWA_LOCAL_STRING( tr( "Restore" ) ) );
    m_pCloseAllWidgetsAction->setText( EWA_LOCAL_STRING( tr( "All" ) ) );
    m_pCloseAutohiddenWidgetsAction->setText( EWA_LOCAL_STRING( tr( "Auto Hides" ) ) );
    m_pCloseDesktopWidgetsAction->setText( EWA_LOCAL_STRING( tr( "On Desktop" ) ) );
    m_pCloseTopWidgetsAction->setText( EWA_LOCAL_STRING( tr( "On Top" ) ) );

    m_pSitesListToolButton->setText( EWA_LOCAL_STRING( tr( "Sites" ) ) );
    translateSitesListActionsMenu();

    m_pOptionsActionToolButton->setText( EWA_LOCAL_STRING( tr( "Options" ) ) );
    translateOptionsActionsMenu();

    m_pConnectionActionToolButton->setText( EWA_LOCAL_STRING( tr( "Internet" ) ) );
    translateConnectionActionsMenu();

    m_pEventsActionToolButton->setText( EWA_LOCAL_STRING( tr( "Information" ) ) );

    updateActionsTooltips();
}

void EWAMainWindow::normalizeListviewsWidths()
{
    int iMaxWidth = EWAApplication::calculateListWidgetsWidth( ui.pSitesListWidget );
    iMaxWidth = qMax( iMaxWidth, EWAApplication::calculateListWidgetsWidth( ui.pOptionsListWidget ) );
    iMaxWidth = qMax( iMaxWidth, EWAApplication::calculateListWidgetsWidth( ui.pConnectionListWidget ) );

    int a,b,c,d;
    ui.pOptionsListWidget->getContentsMargins( &a, &b, &c, &d );
    iMaxWidth += a + c;

    ui.pSitesListWidget->resize( iMaxWidth, ui.pSitesListWidget->height() );
    ui.pSitesListWidget->setFixedWidth( iMaxWidth );
    ui.pOptionsListWidget->resize( iMaxWidth, ui.pOptionsListWidget->height() );
    ui.pOptionsListWidget->setFixedWidth( iMaxWidth );
    ui.pConnectionListWidget->resize( iMaxWidth, ui.pConnectionListWidget->height() );
    ui.pConnectionListWidget->setFixedWidth( iMaxWidth );
}



QString EWAMainWindow::actTooltip( const QString& icon, const QString& text ) const
{
    QString strTemplate = "<br>&nbsp;<img src=\"%1\" width=\"16\" height=\"16\"/>&nbsp;&nbsp;%2";
    return strTemplate.arg( icon ).arg( text );
}

void EWAMainWindow::updateActionsTooltips()
{
    QString strSitesListActionTooltip = tr( "<b>%1</b>:%2%3" )
        .arg( m_pSitesListToolButton->text() )
        .arg( actTooltip( ":/images/documents.png", ui.pSitesListWidget->item(0)->text() ) )
        .arg( actTooltip( ":/images/dldtype.png", ui.pSitesListWidget->item(1)->text() ) );
    m_pSitesListToolButton->setToolTip( strSitesListActionTooltip );

    QString strOptionsActionTooltip = m_qstrTopToolbarActionsTooltipTemplate
        .arg( m_pOptionsActionToolButton->text() )
        .arg( actTooltip( ":/images/preferences.png", ui.pOptionsListWidget->item(0)->text() ) )
        .arg( actTooltip( ":/images/systray.png", ui.pOptionsListWidget->item(1)->text() ) )
        .arg( "" ); //-- disable "%4"
    m_pOptionsActionToolButton->setToolTip( strOptionsActionTooltip );
    
    
    QString strConnectionActionTooltip = m_qstrTopToolbarActionsTooltipTemplate
        .arg( m_pConnectionActionToolButton->text() )
        .arg( actTooltip( ":/images/webkit.png", ui.pConnectionListWidget->item(0)->text() ) )
        .arg( actTooltip( ":/images/proxy.png", ui.pConnectionListWidget->item(1)->text() ) )
        .arg( actTooltip( ":/images/cache.png", ui.pConnectionListWidget->item(2)->text() ) );
    m_pConnectionActionToolButton->setToolTip( strConnectionActionTooltip );

    m_pEventsActionToolButton->setToolTip( tr( "<b>%1</b>" ).arg( m_pEventsActionToolButton->text() ) );
}

void EWAMainWindow::createSitesListActionsMenu()
{
    m_pSitesListActionMenu = new QMenu( this );    
    m_pSitesListActionMenuList = m_pSitesListActionMenu->addAction( 
        ui.pSitesListWidget->item(0)->icon(), 
        tr( "Sites List" ) );
    m_pSitesListActionMenuList->setData( 4 );
    m_pSitesListActionMenuDownloadType = m_pSitesListActionMenu->addAction( 
        ui.pSitesListWidget->item(1)->icon(), 
        tr( "Download type" ) );
    m_pSitesListActionMenuDownloadType->setData( 5 );
    
    connect( m_pSitesListActionMenu, SIGNAL( triggered(QAction*) ),
        this, SLOT( slotSitesListSubmenuTriggered(QAction*) ) );

    m_pSitesListToolButton = new QToolButton(this);
    m_pSitesListToolButton->setIcon( ui.pSitesListWidget->item(0)->icon() );
    m_pSitesListToolButton->setCheckable( true );
    m_pSitesListToolButton->setPopupMode( QToolButton::DelayedPopup );
    m_pSitesListToolButton->setMenu( m_pSitesListActionMenu );
    m_pTopToolBar->addWidget( m_pSitesListToolButton );

    m_pTopToolBarButtons->addButton( m_pSitesListToolButton );

    connect( m_pSitesListToolButton, SIGNAL( toggled(bool) ),
        this, SLOT( slotSitesListToolButtonToggled(bool) ) );
}

void EWAMainWindow::createSitesListFileMenuButton()
{
    m_pSitesListFileMenu = new QMenu( this );
    m_pSitesListFileMenuOpenAction = m_pSitesListFileMenu->addAction( 
        QIcon( ":/images/open.png" ), tr( "Open File" ), this, SLOT( on_openTargetsListButton_clicked() ), QKeySequence( "CTRL+O" ) );
    m_pSitesListFileMenuCreateAction = m_pSitesListFileMenu->addAction(
        QIcon( ":/images/new.png" ), tr( "Create File" ), this, SLOT( slotCreateSitesListFile() ), QKeySequence( "CTRL+N" ) );
    
    ui.openTargetsListButton->setMenu( m_pSitesListFileMenu );
}

void EWAMainWindow::translateSitesListActionsMenu()
{
    m_pSitesListActionMenuList->setText( ui.pSitesListWidget->item(0)->text() );
    m_pSitesListActionMenuDownloadType->setText( ui.pSitesListWidget->item(1)->text() );
}

void EWAMainWindow::createOptionsActionsMenu()
{
    m_pOptionsActionMenu = new QMenu( this );    
    m_pOptionsActionMenuPreferences = m_pOptionsActionMenu->addAction( 
        ui.pOptionsListWidget->item(0)->icon(), 
        tr( "System tray" ) );
    m_pOptionsActionMenuPreferences->setData( 4 );
    
    m_pOptionsActionMenuTray = m_pOptionsActionMenu->addAction( 
        ui.pOptionsListWidget->item(1)->icon(), 
        tr( "Preferences" ) );
    m_pOptionsActionMenuTray->setData( 5 );

    connect( m_pOptionsActionMenu, SIGNAL( triggered(QAction*) ),
        this, SLOT( slotOptionsSubmenuTriggered(QAction*) ) );

    m_pOptionsActionToolButton = new QToolButton(this);
    m_pOptionsActionToolButton->setIcon( QIcon( ":/images/applications.png" ) );//ui.pOptionsListWidget->item(0)->icon() );
    m_pOptionsActionToolButton->setCheckable( true );
    m_pOptionsActionToolButton->setPopupMode( QToolButton::DelayedPopup );
    m_pOptionsActionToolButton->setMenu( m_pOptionsActionMenu );
    m_pTopToolBar->addWidget( m_pOptionsActionToolButton );

    m_pTopToolBarButtons->addButton( m_pOptionsActionToolButton );

    connect( m_pOptionsActionToolButton, SIGNAL( toggled(bool) ),
        this, SLOT( slotOptionsActionToolButtonToggled(bool) ) );
}

void EWAMainWindow::prepareSysTrayUsage()
{
    bool bTrayOK = EWAApplication::getTrayIconManager()->isAvailable();
    
    if( !bTrayOK )
    {
        ui.closeMsgCheckBox->setChecked( false );
        ui.closeMsgCheckBox->hide();
        
        m_pOptionsActionMenuTray->setVisible( false );
        ui.pSysTrayPage->setEnabled( false );
        ui.pSysTrayPage->hide();
        
        ui.pOptionsListWidget->item( 1 )->setHidden( true );
    }
}


void EWAMainWindow::translateOptionsActionsMenu()
{
    m_pOptionsActionMenuPreferences->setText( ui.pOptionsListWidget->item(0)->text() );
    m_pOptionsActionMenuTray->setText( ui.pOptionsListWidget->item(1)->text() );
}

void EWAMainWindow::slotSitesListSubmenuTriggered( QAction *pAction )
{
    if( pAction )
    {
        int iNum = pAction->data().toInt()-4;

        showSitesListSubTab( iNum );
    }
}

void EWAMainWindow::slotOptionsSubmenuTriggered( QAction *pAction )
{
    if( pAction )
    {
        int iNum = pAction->data().toInt()-4;

        showOptionsSubTab( iNum );
    }
}

void EWAMainWindow::showSitesListSubTab( int iNum )
{
    if( iNum >= 0 && iNum < ui.pSitesListWidget->count() )
    {
        m_pSitesListToolButton->setChecked( true );
        ui.pSitesListWidget->setCurrentRow( iNum );
    }
}

void EWAMainWindow::showOptionsSubTab( int iNum )
{
    if( iNum >= 0 && iNum < ui.pOptionsListWidget->count() )
    {
        m_pOptionsActionToolButton->setChecked( true );
        ui.pOptionsListWidget->setCurrentRow( iNum );
    }
}

void EWAMainWindow::createConnectionActionsMenu()
{
    m_pConnectionActionMenu = new QMenu( this );    
    m_pConnectionActionMenuWebKit = m_pConnectionActionMenu->addAction( 
        ui.pConnectionListWidget->item(0)->icon(), 
        tr( "System tray" ) );
    m_pConnectionActionMenuWebKit->setData( 4 );
    m_pConnectionActionMenuProxy = m_pConnectionActionMenu->addAction( 
        ui.pConnectionListWidget->item(1)->icon(), 
        tr( "Startup" ) );
    m_pConnectionActionMenuProxy->setData( 5 );
    m_pConnectionActionMenuCache = m_pConnectionActionMenu->addAction( 
        ui.pConnectionListWidget->item(2)->icon(), 
        tr( "Preferences" ) );
    m_pConnectionActionMenuCache->setData( 6 );

    connect( m_pConnectionActionMenu, SIGNAL( triggered(QAction*) ),
        this, SLOT( slotConnectionSubmenuTriggered(QAction*) ) );

    m_pConnectionActionToolButton = new QToolButton(this);
    m_pConnectionActionToolButton->setIcon( ui.pConnectionListWidget->item(0)->icon() );
    m_pConnectionActionToolButton->setCheckable( true );
    m_pConnectionActionToolButton->setPopupMode( QToolButton::DelayedPopup );
    m_pConnectionActionToolButton->setMenu( m_pConnectionActionMenu );
    m_pTopToolBar->addWidget( m_pConnectionActionToolButton );

    m_pTopToolBarButtons->addButton( m_pConnectionActionToolButton );

    connect( m_pConnectionActionToolButton, SIGNAL( toggled(bool) ),
        this, SLOT( slotConnectionActionToolButtonToggled(bool) ) );
}

void EWAMainWindow::translateConnectionActionsMenu()
{
    m_pConnectionActionMenuWebKit->setText( ui.pConnectionListWidget->item(0)->text() );
    m_pConnectionActionMenuProxy->setText( ui.pConnectionListWidget->item(1)->text() );
    m_pConnectionActionMenuCache->setText( ui.pConnectionListWidget->item(2)->text() );
}

void EWAMainWindow::slotConnectionSubmenuTriggered( QAction *pAction )
{
    if( pAction )
    {
        int iNum = pAction->data().toInt()-4;

        showConnectionSubTab( iNum );
    }
}

void EWAMainWindow::showConnectionSubTab( int iNum )
{
    if( iNum >= 0 && iNum < ui.pConnectionListWidget->count() )
    {
        m_pConnectionActionToolButton->setChecked( true );
        ui.pConnectionListWidget->setCurrentRow( iNum );
    }
}

void EWAMainWindow::slotSitesListToolButtonToggled( bool on )
{
    if( on )
    {
        ui.pTopStackedWidget->setCurrentIndex( 0 );
    }
}

void EWAMainWindow::slotOptionsActionToolButtonToggled( bool on )
{
    if( on )
    {
        ui.pTopStackedWidget->setCurrentIndex( 1 );
    }
}

void EWAMainWindow::slotConnectionActionToolButtonToggled( bool on )
{
    if( on )
    {
        ui.pTopStackedWidget->setCurrentIndex( 2 );
    }
}
void EWAMainWindow::slotEventsActionToolButtonToggled( bool on )
{
    if( on )
    {
        ui.pTopStackedWidget->setCurrentIndex( 3 );
    }
}

void EWAMainWindow::createEventsActionsMenu()
{
    m_pEventsActionToolButton = new QToolButton(this);
    m_pEventsActionToolButton->setIcon( QIcon( ":/images/external_info.png" ) );
    m_pEventsActionToolButton->setCheckable( true );
    m_pEventsActionToolButton->setPopupMode( QToolButton::DelayedPopup );
    m_pTopToolBar->addWidget( m_pEventsActionToolButton );

    m_pTopToolBarButtons->addButton( m_pEventsActionToolButton );

    connect( m_pEventsActionToolButton, SIGNAL( toggled(bool) ),
        this, SLOT( slotEventsActionToolButtonToggled(bool) ) );
}

QString EWAMainWindow::getCachePath() const 
{ 
    QString res = ui.pCacheDirLineEdit->text();
    if( res.isEmpty() )
    {
        res = EWAApplication::settings()->getCacheFolder();
    }
    return res; 
}

void EWAMainWindow::connectStackWidgets()
{
    connect(ui.pTopStackedWidget, SIGNAL(currentChanged(int)), this, SLOT(slotShowTabSexy(int)));
    connect(ui.pSitesListStackedWidget, SIGNAL(currentChanged(int)), this, SLOT(slotShowTabSexy(int)));
    connect(ui.pOptionsStackedWidget, SIGNAL(currentChanged(int)), this, SLOT(slotShowTabSexy(int)));
    connect(ui.pConnectionStackedWidget, SIGNAL(currentChanged(int)), this, SLOT(slotShowTabSexy(int)));
}

void EWAMainWindow::slotShowTabSexy( int tab )
{
    if( !EWAApplication::splashScreen()->isVisible() )
    {
        QStackedWidget *pStackedWidget = qobject_cast<QStackedWidget*>( sender() );
        if( pStackedWidget )
        {
            if( m_pFaderWidget )
            {
                m_pFaderWidget->close();
            }
            m_pFaderWidget = new EWAFaderWidget( pStackedWidget->widget( tab ) );
            m_pFaderWidget->start();
        }
    }
}

bool EWAMainWindow::slotProcessMinimizationRequest()
{
    //-- EWA can't work without sys tray:
    bool bWantQuit = !EWAApplication::getTrayIconManager()->isAvailable();
    
    if( !bWantQuit )
    {
        if( EWAApplication::settings()->m_pGroupPreferences->needWarnOnCLose() )
        {
            QMessageBox warnOnClose( QMessageBox::Information, tr( "EWA:" ),
                tr( "<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 4.0//EN\" \"http://www.w3.org/TR/REC-html40/strict.dtd\">"
                    "<html><head><meta name=\"qrichtext\" content=\"1\" /><style type=\"text/css\">"
                    "p, li { white-space: pre-wrap; }"
                    "</style></head><body style=\" font-family:'MS Shell Dlg 2'; font-size:8.25pt; font-weight:400; font-style:normal;\">"
                    "<p style=\" margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;\"><span style=\" font-size:12pt;\"><b>EWA will be minimized to the system tray.</b></span></p>"
                    "<p style=\" margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;\"><span style=\" font-size:12pt;\">To terminate application, use the <b>&quot;Quit&quot;</b> action.</span></p></body></html>"
                 )
                ,QMessageBox::NoButton, this );
            
            warnOnClose.addButton( tr( "Quit" ), QMessageBox::RejectRole );
            warnOnClose.addButton( tr( "Minimize" ), QMessageBox::AcceptRole );
            warnOnClose.setInformativeText( tr( "This message can be disabled from \"Options\" tab." ) );
            
            bWantQuit = ( warnOnClose.exec() == QDialog::Rejected );
        }
    }
    
    if( bWantQuit )
    {
        m_quitAction->trigger();
        return false;
    }
    else
    {
        showMinimized();
        return true;
    }
}
