/*******************************************************************************
**
** file: ewa_settings.h
**
** class: EWASettings
**
** description:
** App's settings manager/storage
**
** 23.09.2010
**
** sendevent@gmail.com
**
*******************************************************************************/

#ifndef EWA_SETTINGS_H
#define EWA_SETTINGS_H

#include <QObject>
#include <QSize>
#include <QPoint>

class QSettings;
class QSize;
class QPoint;
class EWASettingsGroup: public QObject
{
    Q_OBJECT
    public:
        EWASettingsGroup( QObject *pParent = 0, const QString& strName = QString() ) : QObject( pParent )
        ,m_strGroupName(strName)
        {
        }
        
        virtual void load( QSettings *pSettings, const QString& name = QString() );
        virtual void save( QSettings *pSettings, const QString& name = QString() ) const;
        
        QString getName() const {return m_strGroupName;}
    protected:
        QString m_strGroupName;
};
class EWASitesTabSettingsGroup: public EWASettingsGroup
{
    Q_OBJECT
    
    Q_PROPERTY( QString fileName READ getFileName WRITE setFileName DESIGNABLE false SCRIPTABLE false STORED true USER true );
    Q_PROPERTY( bool downloadSimultaneously READ getDownloadSimultaneously WRITE setDownloadSimultaneously DESIGNABLE false SCRIPTABLE false STORED true USER true );
    
    public:
        EWASitesTabSettingsGroup( QObject *pParent = 0, const QString& strName = QString() ): EWASettingsGroup( pParent, strName ) 
        ,m_strFileName( "" )
        ,m_bDownloadSimultaneously( true )
        {
        }
        
        QString getFileName() const
        {
            return m_strFileName;
        }
        bool getDownloadSimultaneously() const
        {
            return m_bDownloadSimultaneously;
        }
        
    
    private:
        QString m_strFileName;
        bool m_bDownloadSimultaneously;
    
    public slots:
        void setFileName( const QString& strFileName )
        {
            m_strFileName = strFileName;
        }
        void setDownloadSimultaneously( bool on )
        {
            m_bDownloadSimultaneously = on;
        }
};

class EWASettingsPreferencesGroup: public EWASettingsGroup
{
    Q_OBJECT
    
    Q_PROPERTY( bool autostartDownload READ getAutostartDownload WRITE setAutostartDownload );
    Q_PROPERTY( bool checkForUpdates READ getCheckForUpdates WRITE setCheckForUpdates );
    Q_PROPERTY( bool copyLastMessage READ needCopyLastMessage WRITE setCopyLastMessage );
    Q_PROPERTY( bool saveMessageToLog READ needSaveMessageToLog WRITE setSaveMessageToLog );
    Q_PROPERTY( bool warnOnCLose READ needWarnOnCLose WRITE setWarnOnCLose );
    Q_PROPERTY( int langId READ getLangId WRITE setLangId );
    
    public:
        EWASettingsPreferencesGroup( QObject *pParent = 0, const QString& strName = QString() ): EWASettingsGroup( pParent, strName ) 
        ,m_bAutostartDownload( false )
        ,m_bCheckForUpdates( true )
        ,m_bCopyLastMessage( false )
        ,m_bSaveMessageToLog( false )
        ,m_bWarnOnCLose( true )
        ,m_langId( 0 )
        {}
        
        bool getAutostartDownload() const
        {
            return m_bAutostartDownload;
        }
        
        bool getCheckForUpdates() const
        {
            return m_bCheckForUpdates;
        }
        
        bool needCopyLastMessage() const
        {
            return m_bCopyLastMessage;
        }
        
        bool needSaveMessageToLog() const
        {
            return m_bSaveMessageToLog;
        }
        
        bool needWarnOnCLose() const
        {
            return m_bWarnOnCLose; 
        }
        
        int getLangId() const
        {
            return m_langId;
        }
        
    public slots:
        
        void setAutostartDownload( bool on )
        {
            m_bAutostartDownload = on;
        }
        void setCheckForUpdates( bool on )
        {
            m_bCheckForUpdates = on;
        }
        void setCopyLastMessage( bool on )
        {
            m_bCopyLastMessage = on;
        }
        void setSaveMessageToLog( bool on )
        {
            m_bSaveMessageToLog = on;
        }
        void setWarnOnCLose( bool on )
        {
            m_bWarnOnCLose = on;
        }
        void setLangId( int on )
        {
            m_langId = on;
        }
        
    private:
        bool m_bAutostartDownload
            ,m_bCheckForUpdates
            ,m_bCopyLastMessage
            ,m_bSaveMessageToLog
            ,m_bWarnOnCLose; 
        int m_langId;
};

class EWATrayIconSettingsGroup: public EWASettingsGroup
{
    Q_OBJECT
    
    Q_PROPERTY( QString fileName READ getFileName WRITE setFileName );
    Q_PROPERTY( bool useToolTip READ getUseToolTip WRITE setUseToolTip );
    Q_PROPERTY( QString toolTip READ getToolTip WRITE setToolTip );
    
    public:
        EWATrayIconSettingsGroup( QObject *pParent = 0, const QString& strName = QString() ): EWASettingsGroup( pParent, strName ) 
        ,m_fileName( "" )
        ,toolTip( "%EWA%" )
        ,m_bUseToolTip( true )
        {}
        
        QString getFileName() const
        {
            return m_fileName;
        }
        
        bool getUseToolTip() const
        {
            return m_bUseToolTip;
        }
        
        QString getToolTip() const
        {
            return toolTip;
        }
        
    public slots:
        
        void setFileName( const QString& on )
        {
            m_fileName = on;
        }
        void setUseToolTip( bool on )
        {
            m_bUseToolTip = on;
        }
        void setToolTip( const QString& on )
        {
            toolTip = on;
        }
        
    private:
        QString m_fileName
            ,toolTip;
        bool m_bUseToolTip;
};

class EWAWebKitSettingsGroup: public EWASettingsGroup
{
    Q_OBJECT
    
    Q_PROPERTY( bool useJava READ getUseJava WRITE setUseJava );
    Q_PROPERTY( bool useNPAPI READ getUseNPAPI WRITE setUseNPAPI );
    Q_PROPERTY( bool usePrivateBrowsing READ getUsePrivateBrowsing WRITE setUsePrivateBrowsing );
    Q_PROPERTY( bool useAutogetImages READ getUseAutogetImages WRITE setUseAutogetImages );
    Q_PROPERTY( bool useJavascript READ getUseJavascript WRITE setUseJavascript );
    Q_PROPERTY( bool useJSShowAlerts READ getUseJSShowAlerts WRITE setUseJSShowAlerts );
    Q_PROPERTY( bool useJSAccessClipboard READ getUseJSAccessClipboard WRITE setUseJSAccessClipboard );
    
    public:
        EWAWebKitSettingsGroup( QObject *pParent = 0, const QString& strName = QString() ): EWASettingsGroup( pParent, strName ) 
        ,m_bUseJava( false )
        ,m_bUseNPAPI( true )
        ,m_bUsePrivateBrowsing( false )
        ,m_bUseAutogetImages( true )
        ,m_bUseJavascript( true )
        ,m_bUseJSShowAlerts( false )
        ,m_bUseJSAccessClipboard( false )
        {}
        
        bool getUseJava() const
        {
            return m_bUseJava;
        }
        
        bool getUseNPAPI() const
        {
            return m_bUseNPAPI;
        }
        
        bool getUsePrivateBrowsing() const
        {
            return m_bUsePrivateBrowsing;
        }
        
        bool getUseAutogetImages() const
        {
            return m_bUseAutogetImages;
        }
        
        bool getUseJavascript() const
        {
            return m_bUseJavascript;
        }
        
        bool getUseJSShowAlerts() const
        {
            return m_bUseJSShowAlerts;
        }
        
        bool getUseJSAccessClipboard() const
        {
            return m_bUseJSAccessClipboard;
        }
        
    public slots:
    
        void setUseJava( bool on )
        {
            m_bUseJava = on;
        }
        void setUseNPAPI( bool on )
        {
            m_bUseNPAPI = on;
        }
        void setUsePrivateBrowsing( bool on )
        {
            m_bUsePrivateBrowsing = on;
        }
        void setUseAutogetImages( bool on )
        {
            m_bUseAutogetImages = on;
            
        }
        void setUseJavascript( bool on )
        {
            m_bUseJavascript = on;
        }
        void setUseJSShowAlerts( bool on )
        {
            m_bUseJSShowAlerts = on;
        }
        void setUseJSAccessClipboard( bool on )
        {
            m_bUseJSAccessClipboard = on;
        }
        
    private:
        bool m_bUseJava
            ,m_bUseNPAPI
            ,m_bUsePrivateBrowsing
            ,m_bUseAutogetImages
            ,m_bUseJavascript
            ,m_bUseJSShowAlerts
            ,m_bUseJSAccessClipboard;
    
};

class EWAProxySettingsGroup: public EWASettingsGroup
{
    Q_OBJECT
    
    Q_PROPERTY( bool enable READ isEnabled WRITE setEnabled );
    Q_PROPERTY( int typeID READ getTypeID WRITE setTypeID );
    Q_PROPERTY( QString host READ getHost WRITE setHost );
    Q_PROPERTY( int port READ getPort WRITE setPort );
    Q_PROPERTY( QString login READ getLogin WRITE setLogin );
    Q_PROPERTY( QString password READ getPassword WRITE setPassword );
    
    public:
        EWAProxySettingsGroup( QObject *pParent = 0, const QString& strName = QString() ): EWASettingsGroup( pParent, strName ) 
        ,m_bEnable( false )
        ,m_iTypeID( 0 )
        ,m_strHost( "" )
        ,m_iPort( 0 )
        ,m_strLogin( "" )
        ,m_strPassword( "" )
        {}
        
        bool isEnabled() const
        {
            return m_bEnable;
        }
        
        int getTypeID() const
        {
            return m_iTypeID;
        }
        
        QString getHost() const
        {
            return m_strHost;
        }
        
        int getPort() const
        {
            return m_iPort;
        }
        
        QString getLogin() const
        {
            return m_strLogin;
        }
        
        QString getPassword() const
        {
            return m_strPassword;
        }
    
    public slots:
            
        void setEnabled( bool on )
        {
            m_bEnable = on;
            emit signalSettingsChanged();
        }
        void setTypeID( int i )
        {
            m_iTypeID = i;
            emit signalSettingsChanged();
        }
        void setHost( const QString& strHost )
        {
            m_strHost = strHost;
            emit signalSettingsChanged();
        }
        void setPort( int i )
        {
            m_iPort = i;
            emit signalSettingsChanged();
        }
        void setLogin( const QString& strLogin )
        {
            m_strLogin = strLogin;
            emit signalSettingsChanged();
        }
        void setPassword( const QString& strPwd )
        {
            m_strPassword = strPwd;
            emit signalSettingsChanged();
        }
        
    private:
        bool m_bEnable;
        int m_iTypeID;
        QString m_strHost;
        int m_iPort;
        QString m_strLogin;
        QString m_strPassword;
    
    signals:
        void signalSettingsChanged();
};

class EWACacheSettingsGroup: public EWASettingsGroup
{
    Q_OBJECT
    
    Q_PROPERTY( QString storage READ getStorage WRITE setStorage );
    Q_PROPERTY( int limit READ getLimit WRITE setLimit );
    
    public:
        EWACacheSettingsGroup( QObject *pParent = 0, const QString& strName = QString() ): EWASettingsGroup( pParent, strName )  
        ,m_strStorage( "" )
        ,m_iLimit( 10 )
        {}
        
        QString getStorage() const
        {
            return m_strStorage;
        }
        
        int getLimit() const
        {
            return m_iLimit;
        }
        
    public slots:        
    
        void setStorage( const QString& strOn )
        {
            m_strStorage = strOn;
        }
        void setLimit( int on )
        {   
            m_iLimit = on;
        }
        
    private:
        QString m_strStorage;
        int m_iLimit;
};

class EWAMainWindowSettingsGroup: public EWASettingsGroup
{
    Q_OBJECT
    
    Q_PROPERTY( QSize lastSize READ getLastSize WRITE setLastSize );
    Q_PROPERTY( QPoint lastPos READ getLastPos WRITE setLastPos );
    Q_PROPERTY( bool hidden READ isHidden WRITE setHidden );
    Q_PROPERTY( int xmsgCornersRadX READ getXMsgCornersRadiusX WRITE setXMsgCornersRadiusX );
    Q_PROPERTY( int xmsgCornersRadY READ getXMsgCornersRadiusY WRITE setXMsgCornersRadiusY );
    Q_PROPERTY( int xMsgBorderWidth READ getXMsgBorderWidth WRITE setXMsgBorderWidth );
    Q_PROPERTY( bool compositing READ useRichWM WRITE setUseRichWM );
    Q_PROPERTY( bool editedXMessageWarning READ getEditedXMessageWarning WRITE setEditedXMessageWarning );
    Q_PROPERTY( int tab READ getTab WRITE setTab );
    Q_PROPERTY( int subTab READ getSubTab WRITE setSubTab );
    
    public:
        EWAMainWindowSettingsGroup( QObject *pParent = 0, const QString& strName = QString() ): EWASettingsGroup( pParent, strName ) 
        ,m_lastSize( QSize( -1, -1 ) )
        ,m_lastPos( QPoint( 0, 0 ) )
        ,m_bHidden( false )
        ,m_iXmsgCornersRadX( 10 )
        ,xmsgCornersRadY( 10 )
        ,xMsgBorderWidth( 4 )
        ,m_bCompositing( true )
        ,m_bEditedXMessageWarning( true )
        ,m_iTab( 0 )
        ,m_iSubTab( 0 )
        {
        }
        
        QSize getLastSize() const
        {
            return m_lastSize;
        }
        QPoint getLastPos() const
        {
            return m_lastPos;
        }
        bool isHidden() const
        {
            return m_bHidden;
        }
        
        int getXMsgCornersRadiusX() const
        {
            return m_iXmsgCornersRadX;
        }
        
        int getXMsgCornersRadiusY() const
        {
            return xmsgCornersRadY;
        }
        
        int getXMsgBorderWidth() const
        {
            return xMsgBorderWidth;
        }
        
        bool useRichWM() const
        {
            return m_bCompositing;
        }
        
        bool getEditedXMessageWarning() const
        {
            return m_bEditedXMessageWarning;
        }
        
        int getTab() const
        {
            return m_iTab;
        }
        int getSubTab() const 
        {
            return m_iSubTab;
        }
        
    public slots:
    
        void setLastSize( const QSize& on )
        {
            m_lastSize = on;
        }
        void setLastPos( const QPoint& on )
        {
            m_lastPos = on;
        }
        void setHidden( bool on )
        {
            m_bHidden = on;
        }
        void setXMsgCornersRadiusX( int on )
        {
            m_iXmsgCornersRadX = on;
            
        }void setXMsgCornersRadiusY( int on )
        {
            xmsgCornersRadY = on;
        }
        void setXMsgBorderWidth( int on )
        {
            xMsgBorderWidth = on;
        }
        void setUseRichWM( bool on )
        {
            m_bCompositing = on;
        }
        void setEditedXMessageWarning( bool on )
        {
            m_bEditedXMessageWarning = on;
        }
        void setTab( int i )
        {
            m_iTab = i;
        }
        void setSubTab( int i )
        {
            m_iSubTab = i;
        }
        
    private:
        QSize m_lastSize;
        QPoint m_lastPos;
        bool m_bHidden;
        int m_iXmsgCornersRadX
            ,xmsgCornersRadY
            ,xMsgBorderWidth;
        
        bool m_bCompositing
            ,m_bEditedXMessageWarning;
        
        int m_iTab
            ,m_iSubTab;
};

class EWASettings: public QObject
{
    Q_OBJECT
    
    public:
        EWASettings( QObject *pParent = 0 );
        virtual ~EWASettings();
        
        EWASitesTabSettingsGroup *m_pGroupSites;
        EWASettingsPreferencesGroup *m_pGroupPreferences;
        EWATrayIconSettingsGroup *m_pGroupTrayIcon;
        EWAWebKitSettingsGroup *m_pGroupWebkit;
        EWAProxySettingsGroup *m_pGroupProxy;
        EWACacheSettingsGroup *m_pGroupCache;
        EWAMainWindowSettingsGroup *m_pGroupApp;
        
        QString getUserDataFolder();
        QString getSitesListFolder();
        QString getIconsCollectionFolder();
        QString getTranslationsFolder();
        QString getFaviconsFolder();
        QString getCacheFolder();
        QString getDefaultESLFile();
        QString getLogFile();
        QString getDefaultIconFile();
        QString getIniFile();
        
        int getESLVersionMajor() const {return 1;}
        int getESLVersionMinor() const {return 2;}
    
        void save();
        void load();
        
    protected:
        
        QString m_sSitesListFolder;
        QString m_sIconsCollectionFolder;
        QString m_sTranslationsFolder;
        QString m_sFaviconsFolder;
        QString m_sCacheFolder;
        QString m_sDefaultIconFile;
        QString m_sDefaultESLFile;
        QString m_sLogFile;
        QString m_sIniFile;
        QSettings *m_pIniFile;
        QString m_sUserDataFolder;
        
        void normolizeSlashesInPath( QString& path ) const;
        
        void processRealAppPath();
        
    signals:
        void signalSplashScreenMessage( const QString& msg );
        
};

#endif //-- EWA_SETTINGS_H