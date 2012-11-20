/*******************************************************************************
**
** file: ewa_application.h
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

#ifndef EWA_APPLICATION_H
#define EWA_APPLICATION_H

#include <QtGui/QApplication>
#include <QMap>
#include <QTranslator>
#include <QPainter>

#include "ewa_settings.h"

#ifdef Q_OS_WIN
    #include <windows.h>
#endif //-- Q_OS_WIN

//class QSplashScreen;
class QNetworkReply;
class QWebFrame;
class QListWidget;

class EWASitesListManager;
class EWAMainWindow;
class EWATrayIconManager;
class EWATimer;
class EwaBuzzer;
class EWASplashScreen;
class EWAUserActionsPlayer;
class EWASiteThumbnailFileSystem;

class EWAApplication : public QApplication
{
    Q_OBJECT

    public:
        enum TimeUnit
        {
            TU_Seconds = 0
            ,TU_Minutes
            ,TU_Hours
        };

        enum SysTrayMessagePosition
        {
            TopLeft = 0
            ,TopRight
            ,BottomLeft
            ,BottomRight
        };
            
        EWAApplication( int &argc, char **argv );
        ~EWAApplication();
        
        virtual int exec();
        
        static void setLocalization( const QString& qstrQMFileName );
        QString setDefaultLocalization();
        static QString getLocalizationName();
        
        
        
        void displayInfoMessage( const QString& nsg );
        
        QString getBlankPageSource() const;
        QString getBlankPageSourceTemplate() const;
        
        void registerTimer( const EWATimer *pListener );
        void unregisterTimer( const EWATimer *pListener );
        
        void prepareSysTrayUsage();
        
        static const QString& getUserName();
        static const QString& getMachineName();
        static const QString& getUniqueId();
        static const QString getCommitHash();
        static const QString& getCurrentVersion();
        static const QString getCurrentVersionWithHash();
        static const QString getCurrentVersionBuildTime();
        
        static EWAApplication *instance();
        static EWAMainWindow* getMainWindow();
        static EWASitesListManager* getSitesManager();
        static EWATrayIconManager* getTrayIconManager();
        void firstRun();
        
        static QPainter::RenderHints getRenderHints();
        
        static QStringList getImgFormats();
        
        static void initUserAgentsList( QMap<QString, QString>& map );
        
        static QString getEWAUserAgent();
        
        static QRegExp createRegExp( const QString& pattern );
        
        static const QRect getScreenSize( const QPoint& = QPoint() );
        
        static QByteArray getXORedString( const QByteArray& src, const QByteArray& key );
        
        static QPoint getScreensRange();
        
        static QString getAppCachePath();
        static qint64 getAppCacheSizeMax();
        

        static void makeWidgetCentered( QWidget *pWidget, QWidget *pBaseWidget = 0 );
        
        static bool moveStuffFoldersToUserDir();
        
        static EWASplashScreen* splashScreen();

        static QString getKeyName( int key );
        static QString getCurrentLocaleName();
        static QTimer *getTimer();
        static QString translate( const QObject* pObject, const char* pWord );
        
        static void add2Clipboard( const QString& str );
        
        static int getOneTickPeriod();
        
        static bool isValidFileName( const QString& fn );
        
        
        
        static EWASiteThumbnailFileSystem* siteThumbnailsFS();
        
        static QUrl ensureAbsUrl( const QUrl& url, const QUrl& baseUrl );
        
        static QString generateSiteFileName();
        
        virtual void commitData( QSessionManager& manager );
        
        static QString getSitesFolder();
        
        static EWASettings *settings();

        static int calculateListWidgetsWidth( QListWidget *pListWidget );

        static void regsterUserActionsReplayer( EWAUserActionsPlayer *pPlayer );
        static int getUserActionsReplayerIndex( EWAUserActionsPlayer *pPlayer );
        static void ungetUserActionsReplayerIndex( EWAUserActionsPlayer *pPlayer );
        
    private:
        EwaBuzzer *m_pHomeBuzzer;
        static QTranslator *m_pTranslator;
        static QList<EWAUserActionsPlayer*> *s_pGesturePlayers;
        
        QString m_qstrDefaultContent;
        static int m_iOneTickPeriod;

        static EWAMainWindow *s_pMainWindow;
        static EWATrayIconManager *m_pTrayIconManager;
        static EWASitesListManager *s_pSitesManager;
        static QTranslator *s_translator;
        static QString m_qstrEWAVersion;
        static QString m_qstrUniqueId;
        static QString m_qstrMachineName;
        static QString m_qstrOSUserName;
        static QStringList m_strlstImgFormats;
        static QString m_qstrUserAgent;
        static QString m_qstrLocalization;
        
	    static QTimer *m_pAppTimer;
	    static int s_iTimerUsers;
	    static QList<const EWATimer*> *s_pTimerListeners;
	    static QPoint m_pntScreensRange;
        static EWASplashScreen *m_pSplashScreen;
	    static QMap< QString, int > classes;
	    
        
        static QMap<QString, int> m_KeyNames;
        static EWASiteThumbnailFileSystem* m_pSiteThumbnailsFS;
        
        static void initKeyNames();
        static QList<const EWATimer*> *getTimerListeners();
        static void iniUniqueId();
        static void addIdItem( const QString& strItem );
        static void initAllowedImgFormats();
        
        bool setQtLocalization( const QString& name );
        bool setEWALocalization( const QString& name );
        void installTranslator( const QString &name );
        
        static EWASettings *m_pSettings;
        
#ifdef Q_OS_WIN
        HWINEVENTHOOK m_hWinHookId;
#endif //-- Q_OS_WIN        
        
        

    protected slots:
        void realQuit();
        void slotAppTicked();
        
    public slots:
        virtual void quit();
        void slotShowSplashMessage( const QString& msg );
        
    signals:
        void signalOneTick();
};

#endif //-- EWA_APPLICATION_H

