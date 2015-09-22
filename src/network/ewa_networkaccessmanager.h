/*******************************************************************************
**
** file: ewa_networkaccessmanager.h
**
** class: EWANetworkAccessManager
**
** description:
** Overloaded QNetworkAccessManager - for "recording" & "replaying" http "routes"
**
** 09.02.2009
**
** sendevent@gmail.com
**
*******************************************************************************/

#ifndef EWA_NETWORKACCESMANAGER_H
#define EWA_NETWORKACCESMANAGER_H

#ifndef QT_NO_OPENSSL
    #include <QSslError>
#endif //-- QT_NO_OPENSSL

#include <QNetworkAccessManager>
#include <QMap>
#include <QNetworkReply>

#include "ewa_object.h"

class EWASiteHandle;
class EWACookiesJar;
class EWAHttpPocketsCollection;
class EWAUserActionsCollection;
class EWAUserActionsRecorder;
class EWAUserActionsPlayer;

class QBuffer;
class QNetworkDiskCache;
class QWebFrame;

class EWANetworkAccessManager : public QNetworkAccessManager, public EWAObject
{
    Q_OBJECT
    
    public:
        EWANetworkAccessManager( QObject *parent = 0 );
        virtual ~EWANetworkAccessManager();

        void resetRequestCompleter();

        bool m_bNeedSaveRequest;
        bool loadNextRequest();
        
        bool hasFakedRequests() const;
        void clearFakedRequests();

        void clear4NewSession();
        virtual QNetworkDiskCache *cache() const;
        
        bool ignoreSSLErrors() const { return m_bIgnoreSSLErrors; }
        void setIgnoreSSLErrors( bool ignore ) { m_bIgnoreSSLErrors = ignore; }
        
        EWAUserActionsRecorder* getGesturesRecorder() const {return m_pGesturesRecorder;}
        EWAUserActionsPlayer* getGesturesPlayer() const {return m_pGesturesPlayer;}
        EWAHttpPocketsCollection* getHttpPackets() const {return m_pHttpPackets;}
        QVector<EWAUserActionsCollection*>* getUserActivitiesPtr() const {return m_pUserActivitysOnPages;}
        
        void setSite( EWASiteHandle *pSite );
        
        void save( QSettings *pSettings );
        void load( QSettings *pSettings );
        
        void cloneSettings( const EWANetworkAccessManager *pOther );
        
        void setCommonDelayForActions( QVector<EWAUserActionsCollection*> *pPages, int iDelay ) const;
        
        QString lastErrorString() const;
        QNetworkReply::NetworkError lastErrorCode() const {return m_lastError;}
    protected:
        bool m_bIgnoreSSLErrors;
        int m_iCurrentRoutePoint;
        EWACookiesJar *m_pCookies;
        QNetworkDiskCache *m_pCache;
        EWAUserActionsRecorder *m_pGesturesRecorder;
        EWAUserActionsPlayer *m_pGesturesPlayer;
        EWAHttpPocketsCollection *m_pHttpPackets;
        QVector<EWAUserActionsCollection*>* m_pUserActivitysOnPages;
        EWASiteHandle *m_sitePtr;
        QMap< QNetworkReply*, QBuffer* > *m_pBuffersMap;
        bool m_bFakeStarted;
        QString m_strLastError;
        QNetworkReply::NetworkError m_lastError;
        
        /*QUrl m_delayedFakedRedirectionUrl;
        QWebFrame *m_pDelayedFakedRedirectionFrame;*/
        
        void initLastError();
        
        virtual QNetworkReply* createRequest( Operation op, const QNetworkRequest& req,
            QIODevice *outgoingData = 0 );

    protected slots:
#ifndef QT_NO_OPENSSL
        void slotProcesSslErrors( QNetworkReply *reply, const QList<QSslError> &errors );
#endif //-- QT_NO_OPENSSL
        void slotFakedRequestFinished();
        void slotRequestFinished( QNetworkReply *pReply );
        //void startDelayedFakedRedirection( QObject *pObject = 0 );
    
    /*signals:
        void signalErrorOccured( const QString& strUrl, const QString& strErr );*/
};

#endif //-- EWA_NETWORKACCESMANAGER_H
