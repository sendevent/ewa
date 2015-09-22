/*******************************************************************************
**
** file: ewa_sitehandle.h
**
** class: EWASiteHandle
**
** description:
** Class to handle web sites.
**
** 09.02.2009
**
** sendevent@gmail.com
**
*******************************************************************************/

#ifndef EWA_SITEHANDLE_H
#define EWA_SITEHANDLE_H

#include <QObject>
#include <QList>
#include <QBuffer>

#include "ewa_parsingrulecollection.h"
#include "ewa_siteroutepoint.h"
#include "ewa_application.h"
#include "ewa_sitewidget.h"
#include "ewa_showpolicy.h"
#include "ewa_webview.h"

#include "ewa_faviconloader.h"

        
class EWASitePtr
{
    public:
    EWASitePtr( QObject *pSite = 0 )
    {
        m_pSite = qobject_cast<EWASiteHandle*>( pSite );
    }
    EWASitePtr( const EWASitePtr& other )
    {
        m_pSite = other.m_pSite;
    }
    EWASiteHandle *m_pSite;
};

Q_DECLARE_METATYPE( EWASitePtr )


class EWASiteWidget;
class EWAWebView;
class EWAWebPage;
class EWANetworkAccessManager;
class QSettings;
class EWATimer;
class EWASiteThumbnailFileSystem;

class EWASiteHandle :public QObject
{
    Q_OBJECT
    
    public:
        enum DownloadsPeriodPolicy
        {
            DP_Fixed = 0,
            DP_Random,
            DP_Parallel
        };

        EWASiteHandle( QObject *parent = 0 );
        EWASiteHandle( const EWASiteHandle& other );
        EWASiteHandle( const EWASiteHandle *pOther );
        virtual ~EWASiteHandle();

        EWASiteHandle& operator=( const EWASiteHandle& other );
        EWASiteHandle& operator=( const EWASiteHandle *pOther );

        void setUrl( const QString& url );
        const QString getUrl() const;
        void setUserAgent( const QString& strUA );
        QString getHost() const {return m_url.host();}
        const QString getUrlStrLimit32() const;

        void markupMonitoredElement();

        QString getBodyAsHtml() const;
        QString getBodyAsPlainText() const;
        QString getBodyAsHtmlSaved() const { return m_qstrUtf8BodyHTML;};
        QString getBodyAsPlainTextSaved() const {return m_qstrUtf8BodyText;};

        bool isStarted() const { return m_bStarted; }
        
        void setRulesCollection( const EWAParsingRuleCollection *pOther )
        {
            m_pRulesCollection->cloneData( pOther );
        }
        EWAParsingRuleCollection *getRulesCollection() const { return m_pRulesCollection; }
        
        int rulesCount() const { return getRulesCollection()->getRulesCount(); }
        
        bool isUsed() const { return m_bUsed; }
        void setUsed( bool on );
        
        bool isModifyed() const {return m_bChanged;}

        bool isDownloadingActive() const;

        EWAWebView* getWebView() const;
        EWAWebPage* getWebPage() const { return getWidget()->getWebView()->getWebPage(); }
        EWASiteWidget *getWidget() const;

        void setEWANetworkManagerPtr( EWANetworkAccessManager *pAccessManager );
        EWANetworkAccessManager* getEWANetworkManagerPtr() const {return m_networkAccessManagerPtr; }

        QPixmap getFavicoPixmap() const;

        bool setPeriodAndUnits( const int& period, const unsigned int& unit );
        int getPeriodAndUnits( int& period ) const;
        int getPeriodInSeconds() const {return m_lDownloadPeriodSec/1000; }
        int getPeriodInMSeconds() const {return m_lDownloadPeriodSec; }

        QPoint getPageScroll() const;

        void setShowPolicy( const EWAShowPolicy *pPolicy ) { m_pShowPolicy->operator=( pPolicy ); emit signalSiteModified();};
        EWAShowPolicy* getShowPolicy() const { return m_pShowPolicy; }

        void start();
        void stop();
        void stopAndClear();
        void stopEx();

        long getCountdownOriginal() const {return m_lCountDownSecondsOriginal; };
        long getCountdownCurrent() const {return m_lCountDownSecondsCurrent; };
        void resetCountdown();
        QString getCountdownCurrentHMS() const { return secondsToHMS( getCountdownCurrent() ); };

        QString secondsToHMS( long sec ) const;

        EWASitePtr getPointer() const{ return m_pointer; }

        int getDownloadProgress() const { return m_iLoadingProgress; }

        void setRecording( bool on ) { m_bIsRecording = on; }
        bool isRecording() const { return m_bIsRecording; }

        void setWebPageBaseSize( const QSize& sz );
        const QSize getWebPageBaseSize() const;
        QSize getWebPageCurrentSize() const;

        void setHtmlWithoutDownload( const QString &html, const QString& baseUrl = QString() );

        QSize getNativeScrSize() const { return m_qszNativeScrSize; }

        bool extendedMessages() const {return m_bExtendedMsg;}
        void setExtendedMessages( bool on );

        void setMsgTitle( const QString& title );
        QString getMsgTitle() const {return m_qstrMsgTitle; }
        QString getVisibleMsgTitle() const;

        void setMsgTTL( const int ttl )
        {
            if( getWidget()->getTTL() != ttl )
            {
                getWidget()->setTTL( ttl ); 
            }
        }
        int getMsgTTL() const {return getWidget()->getTTL(); }

        int getMsgWidth() const { return getWidget()->width(); }
        void setMsgWidth( const int w ) {getWidget()->setNewSize( w, -1 ); }

        int getMsgHeight() const { return getWidget()->height(); }
        void setMsgHeight( const int h ) {getWidget()->setNewSize( -1, h );}

        QRect getWidgetBaseRect() const {return QRect( getMsgLocation().x(), getMsgLocation().y(), getMsgWidth(), getMsgHeight());}

        QPoint getMsgLocation() const {return getWidget()->getLocation(); }
        void setMsgLocation( const QPoint& pnt ) {getWidget()->setLocation( pnt );}

        QColor getMsgColor() const {return getWidget()->getColor();}
        void setMsgColor( const QColor& color ) {getWidget()->setColor( color );}

        int getSysTrayMsgType() const {return m_iSysTrayMsgType;}
        void setSysTrayMsgType( const int& type ) { m_iSysTrayMsgType = type ;emit signalSiteModified();}

        bool useMsgLengthLimit() const {return m_bMsgLengthLimit;}
        void setUseMsgLengthLimit( bool use )
        {
            if( m_bMsgLengthLimit != use )
            {
                m_bMsgLengthLimit = use;
                emit signalSiteModified();
            }
        }

        int getMsgLengthLimit() const {return m_iMsgLengthLimit;}
        void setMsgLengthLimit( const int limit )
        {
            if( m_iMsgLengthLimit != limit )
            {
                m_iMsgLengthLimit = limit;
                emit signalSiteModified();
            }
        }

        bool noMsgWordsWrap() const {return m_bMsgNoWordsWrap;}
        void setNoMsgWordsWrap( const bool on ) 
        {
            if( m_bMsgNoWordsWrap != on )
            {
                m_bMsgNoWordsWrap = on;
                emit signalSiteModified();
            }
        }

        int getMsgBaseTransparency() const;
        void setMsgBaseTransparency( const int to );
        
        bool replayTypeIsHttp() const{ return m_bReplayTypeIsHttp;}
        void setReplayTypeIsHttp( bool bIsHttp )
        {
            if( m_bReplayTypeIsHttp != bIsHttp )
            {
                m_bReplayTypeIsHttp = bIsHttp;
                
                emit signalSiteModified();
            }
        }
        
        void closeWidgetByHideViewMode( int mode );
        
        bool isReplayFinished() const;
        
        void load( QSettings *pSettings );
        void save( QSettings *pSettings, bool bForce = false );
        void load( const QString& fileName );
        void save( const QString& fileName = QString(), bool bForce = false );

        void setTypingProgress( int progress );
        int getTypingProgress()const {return m_iTypingProgress;}
        
        

        int getDownloadCounter() const {return m_iDownloadCounter;}

        void oneSecondTicked();

        void setPrevMatched( const QString& strPrevMatched ) { m_qstrPrevMatched = strPrevMatched; }
        QString getPrevMatched() const { return m_qstrPrevMatched; }
        
        QString getWatchedElementSource() const;
        QWebElement getWatchedElement() const;
        
        QColor getChangesMarkerColor() const {return m_changesMarkupColor;}
        QString getThumbnailFileName() const {return m_thumbnailFileName;}
        
        QString getFileName();
    protected:
        QString m_strFileName;
        EWAParsingRuleCollection *m_pRulesCollection;
        EWAShowPolicy *m_pShowPolicy;

//#ifdef EWA_FAVICONS
        EWAFaviconLoader *m_pFaviconLoader;
//#endif
        EWANetworkAccessManager *m_networkAccessManagerPtr;
        mutable EWASiteWidget *m_pSiteWidget;
        EWASitePtr m_pointer;
        EWATimer *m_pReqTimeoutTimer;
        
        QByteArray m_siteTumbnailBuffer;
        QString m_thumbnailFileName;
        EWASiteThumbnailFileSystem *m_sitesThumbnailsFSPtr;
        
        int timerPeriod
            ,m_iDownloadCounter
            ,m_iMsgLengthLimit
            ,m_iSysTrayMsgType
            ,m_iLoadingProgress
            ,m_iTypingProgress
            ,m_iLastPtogress;
        
        unsigned int m_iPeriodUnits
            ,m_eTimeUnits;
            
        unsigned short m_usSeconds
            ,m_usMinutes
            ,m_usHours;
        
        int m_lDownloadPeriodSec
            ,m_lCountDownSecondsCurrent
            ,m_lCountDownSecondsOriginal;
        
        bool m_bUsed
            ,m_bStarted
            ,m_bIsRecording
            ,m_bDownloading
            ,m_bForceDownload
            ,m_bLookForFavico
            ,m_bMsgNoWordsWrap
            ,m_bMsgLengthLimit
            ,m_bReplayTypeIsHttp
            ,m_bChanged;
        
        QString m_qstrUtf8BodyText
            ,m_qstrUtf8BodyHTML
            ,m_qstrPrevMatched
            ,m_qstrMsgTitle
            ,m_qstrDescr;
        
        QUrl m_url;
        
        QSize m_webPageSize
            ,m_qszNativeScrSize;
            
        QIcon m_favicon;
        
        QPoint m_pntViewScroll;
            
        QColor m_changesMarkupColor;
        
        void construct();

        void processServerAnswer( const QString& html );
        void runExtRoutine();
        /**
        ** Check const QString& html for changes from prev download.
        ** Return true if:
        ** ShowPolicy is unused;
        ** ShowPolicy is set to "Show if Changed" and text changed;
        ** ShowPolicy is set to "Show if NOT Changed" and text NOT changed.
        */
        bool trackChanges( const QString& html );

        void setPeriod( int period );
        void setPeriodInSeconds( int count );
        void setPeriodInMinutes( int count );
        void setPeriodInHours( int count );

        QString prepareDescription( const QString& descr ) const;

        void updateTumbnail();

        bool m_bExtendedMsg;

        void connectMe();
        void disconnectMe();
        
        void lookForFavicon();
        
        void setFileName( const QString& fn );
        
        void clearIniFile( QSettings *pSetts = 0 ) const;
        
        bool saveFavicon2data( QByteArray& ba ) const;
        void loadFaviconFromData( const QByteArray& data );
        
    protected slots:
        void slotFaviconChanged( const QIcon& icon = QIcon() );
        void slotUserActivitiesReplayProgress( int percent );
        void slotUserActivitiesReplayed();
        
        void slotStopDownloadByTimeout();

    public slots:
        void slotDownload();
        void slotDownloaded( bool ok = true );
        void slotTrackDownloadProgress( int );
        void setPageScroll( const QPoint& pt );
        void slotActionDelegatActivated();
        void slotWantDownload( QNetworkReply *pReply );
        void setChangesMarkerColor( const QColor& color );
        void slotModifyed();

    signals:
        void signalDownloaded( const EWASitePtr *pSite );
        void signalSiteLoadProgress( int );
        void signalSiteTypingProgress( int progress );
        void signalOneSecond();
        void signalDescriptionChanged( const QString& url );
        void signalUsageChanged();
        void signalFaviconChanged();
        
        void signalUserTypingStarted();
        void signalUserTypingFinished();
        
        void signalSiteModified();

    friend class EWAWebView;
    friend class EWASitesListManager;
};

#endif //-- EWA_SITEHANDLE_H
