/*******************************************************************************
**
** file: ewa_mainwindow.h
**
** class: EWAMainWindow
**
** description:
** Frontend for settings, sessions and event processing.
**
** 09.02.2009
**
** ewauthor@indatray.com
**
*******************************************************************************/

#ifndef EWA_MAINWINDOW_H
#define EWA_MAINWINDOW_H

#include <QtGui>
#include <QSystemTrayIcon>
#include <QSettings>
#include <QTextBrowser>

#include <QColor>
#include <QIcon>
#include <QPointer>

#include "ui_ewa_mainwindow.h"
#include "ewa_sitewidgetsettingsdlg.h"
#include "ewa_sysmsgsettings.h"
#include "ewa_sexytabshower.h"

class EWAViewManager;
class EWASitesListManager;
class QAction;
class EWASiteWidget;
class EWAAboutDialog;
class EWAUpdatesChecker;
class EWAFaderWidget;

class EWAMainWindow : public QMainWindow
{
    Q_OBJECT

    public:
        EWAMainWindow( QWidget *parent = 0 );
        virtual ~EWAMainWindow();

        void showStartup();

        bool isStarted() const { return m_pStartAction->isChecked(); }
        bool needSaveMessage() const {return ui.needLogLastMsgCheckBox->isChecked();}
        const QString& getLogFileName() const {return m_qstrLogFileName; }

        QTextBrowser *getJSTextDiasplay() const {return ui.pJSTextBrowser; }
        
        QString getCachePath() const;
        qint64 getCacheMaxSize() const { qint64 res = ui.pCacheStorageSizeSpinbox->value()*1024*1024; return res;}
        
        void saveSettings();
        void loadSettings();
        
        void checkUpdatesDelayed( int delay );
        void updateUiWhileLoadingSitesList( bool bLoadFinished );
        
    private:
        Ui::EWAMainWindowUi ui;
        QMenu *m_pTrayIconMenu;
        QMenu *m_pCloseWidgetsSubMenu;
        QAction *m_quitAction;
        QAction *m_pStartAction;
        QAction *m_pStopAction;
        QAction *m_pPauseAction;
        QAction *m_helpAction;
        QAction *m_aboutAction;
        QAction *m_pMinimizeAction;
        QAction *m_pRestoreAction;
        
        QAction *m_pCloseAllWidgetsAction
                ,*m_pCloseDesktopWidgetsAction
                ,*m_pCloseTopWidgetsAction
                ,*m_pCloseAutohiddenWidgetsAction;

        QButtonGroup *m_pTopToolBarButtons;

        QMenu *m_pSitesListFileMenu;
        QAction *m_pSitesListFileMenuOpenAction
                ,*m_pSitesListFileMenuCreateAction;

        QMenu *m_pSitesListActionMenu;
        QToolButton *m_pSitesListToolButton;
        QAction *m_pSitesListActionMenuDownloadType
            ,*m_pSitesListActionMenuList;

        QMenu *m_pOptionsActionMenu;
        QToolButton *m_pOptionsActionToolButton;
        QAction *m_pOptionsActionMenuTray
            ,*m_pOptionsActionMenuPreferences;
        

        QMenu *m_pConnectionActionMenu;
        QToolButton *m_pConnectionActionToolButton;
        QAction *m_pConnectionActionMenuWebKit
            ,*m_pConnectionActionMenuProxy
            ,*m_pConnectionActionMenuCache;

        QToolButton *m_pEventsActionToolButton;

        QToolBar *m_pBottomToolBar, *m_pTopToolBar;
        QSize m_topToolbarsIconsSize, m_botToolbarsIconsSize;
        QIcon m_icon;
        QString m_qstrLogFileName;

        QString m_qstrTrayToolTip;
        QString m_qstrMsgTitle;
        
        QPointer<EWAFaderWidget> m_pFaderWidget;
        
        bool m_bUseProxy;
        int m_iProxyType;
        QString m_qstrProxyHost;
        int m_iProxyPort;
        QString m_qstrProxyLogin;
        QString m_qstrProxyPassword;
        
        QString m_qstrTopToolbarActionsTooltipTemplate;
        
        EWAUpdatesChecker *m_pUpdatesChecker;
        
        EWASitesListManager *m_sitesManagerPtr;
        
        void initGui();
        void initTrayIcon();
        void initToolBars();
        void initTopToolBar();
        void createSitesListActionsMenu();
        void createSitesListFileMenuButton();
        void translateSitesListActionsMenu();
        void createOptionsActionsMenu();
        void translateOptionsActionsMenu();
        void createConnectionActionsMenu();
        void translateConnectionActionsMenu();
        void createEventsActionsMenu();
        void initBottomToolBar();

        void connectStackWidgets();

        void initInternalConnections();

	    void setIcon( const QString& fileName );

	    void setTrayToolTip( const QString& toolTip );
	    void updateTrayIconToolTip();

	    
	    void greetUser() const;
	    
	    void updateActionsTooltips();
        QString actTooltip( const QString& icon, const QString& text ) const;
	    
	    void translateLanguagesComboBox();
        void translateActionsText();
        void normalizeListviewsWidths();

        void showSitesListSubTab( int iNum );
        void showOptionsSubTab( int iNum );
        void showConnectionSubTab( int iNum );

        void processUpdatesNeeded( QString& prev ) const;
        int versionToInt( const QString& strVer ) const;

        virtual void closeEvent( QCloseEvent *e );
        virtual void changeEvent( QEvent * event );
        
        void prepareSysTrayUsage();
        
        void getCurrentTubAndSubtab( int& iTab, int& iSubTab ) const;
        void setCurrentTubAndSubtab( const int iTab, const int iSubTab );
        
        bool needAutoconnect() const {return ui.autoconnectCheckBox->isChecked();}
        bool needWarnOnClose() const {return ui.closeMsgCheckBox->isChecked();}
        bool needUseClipboard() const {return ui.insertInClipboardCheckBox->isChecked();}
        int getCurrentLanguage() const {return ui.langComboBox->currentIndex();}
        QString getSitesFileName() const {return ui.listFileNameLineEdit->text();}
        bool isDownloadTypeParallel() const {return ui.parallelRadioButton->isChecked();}
        
        void initSettingsCache();
        void initSettingsProxy();
        void initSettingsWebKit();
        
        void initSettingsPrefs();
        void initSettingsSystray();
        
        void initSettingsApp();
        
        void initSitesList();

    protected slots:
        void on_openTargetsListButton_clicked();
        void slotCreateSitesListFile();

        void slotCloseSitesWidgets();

        void slotStartSitesDownloading();
        void slotStopSitesDownloading();
        void slotPauseSitesDownloading( bool on );
        
        void slot_helpAction_triggered( bool checked = false );
        void slot_aboutAction_triggered( bool checked = false );

        void slotHideToolbarContext( const QPoint& );

        //-- "View" tab:
        void on_iconPushButton_clicked();
        void on_clearIconButton_clicked();
        void on_toolTipGroupBox_toggled( bool on );
        void on_tooltipLineEdit_textChanged( const QString& toolTip );

        
        void on_pUpdatesCheckBox_toggled( bool on );

        void slotUpdateSites( const QString& fileName );

        void on_pCacheDirLineEdit_textChanged( const QString& path );
        void on_pChangeCacheDirButton_clicked();
        void on_pCacheStorageSizeSpinbox_valueChanged( int val );

        void slotChangeMainTab( QAction *pAction );
        
        void on_parallelRadioButton_toggled( bool );
        void on_seriallyRadioButton_toggled( bool );
        
        int on_langComboBox_activated( int iRow );

        void slotSitesListSubmenuTriggered( QAction *pAction );
        void slotOptionsSubmenuTriggered( QAction *pAction );
        void slotConnectionSubmenuTriggered( QAction *pAction );

        void slotSitesListToolButtonToggled( bool on );
        void slotOptionsActionToolButtonToggled( bool on );
        void slotConnectionActionToolButtonToggled( bool on );
        void slotEventsActionToolButtonToggled( bool on );
        
        void slotCheckForUpdates();
        
        void slotShowTabSexy( int );
        
        void updateProxy();
        
        void on_useJavaCheckBox_toggled( bool on );
        void on_useNPAPICheckBox_toggled( bool on );
        void on_usePrivateBrowsingCheckBox_toggled( bool on );
        void on_downloadImagesCheckBox_toggled( bool on );
        void on_enableJavascript_toggled( bool on );
        void on_pShowAlertsCheckBox_toggled( bool on );
        void on_jsAccessClipboardCheckBox_toggled( bool on );
        
    public slots:
        void showMinimized();
        void showNormal();
        void show();
        
        void slotSetSkin( const QString& fn = QString() );
        
        /**
        ** Returns true if minimized,
        ** otherwise returns false;
        ** (usually - on quit)
        */
        bool slotProcessMinimizationRequest();
        
    signals:
        void signalShowSplashscreenMessage( const QString& msg );
        
    friend class EWASettings;
    friend class EWAApplication;
};

#endif //-- EWA_MAINWINDOW_H
