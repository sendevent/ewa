/*******************************************************************************
**
** file: ewa_webpage.h
**
** class: EWAWebPage
**
** description:
** QWebPage etension for internalusage
**
** 23.03.2009
**
** sendevent@gmail.com
**
*******************************************************************************/
#ifndef EWA_WEBPAGE_H
#define EWA_WEBPAGE_H

#include <QWebPage>
#include <QNetworkRequest>
#include <QNetworkReply>

class EWANetworkAccessManager;
class EWASiteHandle;
class EWAWebPage : public QWebPage
{
    Q_OBJECT
    
    public:
        EWAWebPage( QObject *parent = 0, const QSize& maxSz = QSize() );
        virtual ~EWAWebPage();

        EWAWebPage& operator=( const EWAWebPage& other );

        void setSite( EWASiteHandle *pSite );

        bool downloadPresaved();
        bool downloadRegular();

        void setSilentlyOnce( bool on ) { m_bSilentlyOnce = on; };

        EWASiteHandle *getSite() const {return m_sitePtr;}
        QSize getContentsSize() const {return m_szContentsSize; }
        
        void setUserAgent( const QString& strUserAgent ) { m_qstrUserAgent = strUserAgent; }
        QString getUserAgent() const {return m_qstrUserAgent;}
        QString getOriginalUserAgent() const;

        void setEWANetworkManagerPtr( EWANetworkAccessManager *pManager );
        const EWANetworkAccessManager *getNetworkManager() const {return m_ENAManagerPtr;}

        void setBlank();
        bool isBlank() const;
        void setErrorMessage( const QString& urlStr, const QString& errStr, QWebFrame *pFrame = 0 );

        QWebFrame* getCurrentFrame() const;
                
    protected:
        EWANetworkAccessManager *m_ENAManagerPtr;
        EWASiteHandle *m_sitePtr;
        
        QSize m_szContentsSize;
        
        QString m_qstrUserAgent;
            
        bool m_bRecordAllowed
            ,m_bIsRecording
            ,m_bSilentlyOnce;
        

        virtual QString userAgentForUrl( const QUrl& url ) const;
        virtual bool acceptNavigationRequest( QWebFrame *frame, const QNetworkRequest& request, NavigationType type );
        
        void showJSOutput( const QString& msg );
        virtual void javaScriptAlert( QWebFrame *frame, const QString& msg );
        virtual bool javaScriptConfirm( QWebFrame *frame, const QString& msg );
        virtual void javaScriptConsoleMessage( const QString& message, int lineNumber, const QString& sourceID );

        void setCanRemember( bool on ){ m_bRecordAllowed = on;}
        bool isCanRemember() const { return m_bRecordAllowed;}

        bool isNeedRemember() const {return m_bIsRecording;}

        bool loadNextRequest();

        //-- init internal connections
        void connectMe();
        void disconnectMe();

    protected slots:
        void handleUnsupportedContent( QNetworkReply *reply );
        void slotLoadStarted();
        void slotLoadFinished( bool ok );
        void slotFrameCreated( QWebFrame *pFrame );
        void slotFrameLoaded( bool bOk );
        
        bool shouldInterruptJavaScript();
        void slostStatusBarVisibilityChangeRequested( bool visible );
        void slotToolBarVisibilityChangeRequested( bool visible );
        void slotDownloadRequested( const QNetworkRequest& request );
        void slotGeometryChangeRequested( const QRect& geom );
        void slotRestoreFrameStateRequested( QWebFrame *pFrame );



    signals:
        void signalTargetLoaded( bool ok );
        void signalNeedDownload( QNetworkReply *reply );
        void contentsSizeChanged( const QSize& size );
};

#endif //-- EWA_WEBPAGE_H
