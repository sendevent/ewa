/*******************************************************************************
**
** file: ewa_networkaccessmanager.cpp
**
** class: EWANetworkAccessManager
**
** description:
** Overloaded QNetworkAccessManager - for "recording" & "replaying" http "routes"
**
** 09.02.2009
**
** ewauthor@indatray.com
**
*******************************************************************************/

#include "ewa_networkaccessmanager.h"

#include "ewa_sitehandle.h"
#include "ewa_useractionsrecorder.h"
#include "ewa_useractionsplayer.h"
#include "ewa_cookiesjar.h"
#include "ewa_httppocketscollection.h"

#include <QBuffer>
#include <QMultiMap>
#include <QNetworkDiskCache>
#include <QDesktopServices>

EWANetworkAccessManager::EWANetworkAccessManager( QObject *parent )
:QNetworkAccessManager( parent ), m_pCache(0)
{
    m_pBuffersMap = new QMap< QNetworkReply*, QBuffer* >();
    m_bNeedSaveRequest = false;
    m_pCookies = new EWACookiesJar( this );
    setCookieJar( m_pCookies );
    
    //-- QNetworkAccessManager takes ownership of the object cookieJar
    //-- don't delete it
    resetRequestCompleter();

#ifndef QT_NO_OPENSSL    
    connect( this, SIGNAL( sslErrors(QNetworkReply*,const QList<QSslError>&) ),
        this, SLOT( slotProcesSslErrors(QNetworkReply*,const QList<QSslError>&) ) );
#endif //- QT_NO_OPENSSL
    m_pCache = new QNetworkDiskCache( this );
    QString location = EWAApplication::getAppCachePath();
    m_pCache->setCacheDirectory( location );
    m_pCache->setMaximumCacheSize( EWAApplication::settings()->m_pGroupCache->getLimit() );
    setCache( m_pCache );
    
    m_pGesturesRecorder = new EWAUserActionsRecorder( this );
    m_pGesturesPlayer = new EWAUserActionsPlayer( this );
    m_pHttpPackets = new EWAHttpPocketsCollection( this );

    m_pUserActivitysOnPages = new QVector<EWAUserActionsCollection*>();
    m_pGesturesRecorder->setPagesPtr( m_pUserActivitysOnPages );
    m_pGesturesPlayer->setPagesPtr( m_pUserActivitysOnPages );
    
    initLastError();

    connect( this, SIGNAL(finished(QNetworkReply*)),
        this, SLOT(slotRequestFinished(QNetworkReply*)) );
}

EWANetworkAccessManager::~EWANetworkAccessManager()
{
    delete m_pBuffersMap;
    if( m_pUserActivitysOnPages )
    {
        for( int i = m_pUserActivitysOnPages->count()-1; i>= 0 ; i-- )
        {
            EWAUserActionsCollection *pCollection = m_pUserActivitysOnPages->at( i );
            m_pUserActivitysOnPages->remove( i );
            delete pCollection;
        }
        delete m_pUserActivitysOnPages;
    }
    
    delete m_pHttpPackets;
    m_pHttpPackets = 0;
    
    delete m_pGesturesPlayer;
    m_pGesturesPlayer = 0;
    
    delete m_pGesturesRecorder;
    m_pGesturesRecorder = 0;
    
    delete m_pCache;
    m_pCache = 0;
    
    delete m_pCookies;
    m_pCookies = 0;
}

QNetworkReply* EWANetworkAccessManager::createRequest( Operation op,
    const QNetworkRequest& req, QIODevice *outgoingData )
{
    QNetworkRequest request( req );
    request.setAttribute( QNetworkRequest::CacheLoadControlAttribute, QNetworkRequest::PreferNetwork );
    
    QNetworkReply *pReply = 0;
    if( !m_bNeedSaveRequest || !m_sitePtr->replayTypeIsHttp() )
    {
        pReply = QNetworkAccessManager::createRequest( op, request, outgoingData );
    }
    else
    {
        m_bNeedSaveRequest = false;

        QByteArray storedPostData;
        QBuffer *buffer = new QBuffer;

        EWASiteRoutePoint *storeReq = m_pHttpPackets->addPacket();

        if( !outgoingData )
        {
            delete buffer;
            storeReq->initStore( request, op );
            pReply = QNetworkAccessManager::createRequest( op, request );
        }
        else
        {
            storedPostData = outgoingData->readAll();
            storeReq->initStore( request, op, storedPostData );

            buffer->setData( storeReq->getData() );
            buffer->open( QIODevice::ReadWrite );

            pReply = QNetworkAccessManager::createRequest( op, request, buffer );
            buffer->setParent( pReply );
        }
    }
    
    return pReply;
}


void EWANetworkAccessManager::resetRequestCompleter()
{
    m_sitePtr = 0;
    clear4NewSession();
}

bool EWANetworkAccessManager::loadNextRequest()
{
    if( m_sitePtr->replayTypeIsHttp()
    && m_pHttpPackets
    && m_pHttpPackets->packetsCollectionCount()
    && m_iCurrentRoutePoint <= m_pHttpPackets->packetsCollectionCount()-1 )
    {
        EWASiteRoutePoint *request = m_pHttpPackets->getPacketAt( m_iCurrentRoutePoint++ );
        if( request )
        {
            Operation op = request->getOperation();
            QNetworkRequest qnReq = request->getRequest();

            //-- buffer will be deleted in the slot 
            //-- when reply is received:

            m_bFakeStarted = true;
            
            QBuffer* pBuff = new QBuffer( (QByteArray*)&( request->getData() ) );
            m_pBuffersMap->insert( createRequest( op, qnReq, pBuff ), pBuff );
            //qDebug() << m_sitePtr->getWidget()->getTitle() << "fake to:" << m_pBuffersMap->key( pBuff )->url().toString();
            connect(m_pBuffersMap->key( pBuff ), SIGNAL(finished()),
                     this, SLOT(slotFakedRequestFinished()));
            
            /*QBuffer* pBuff = new QBuffer( (QByteArray*)&( request->getData() ) );
            if( pBuff->data().isEmpty() )
            {
                delete pBuff;
                pBuff = 0;
            }
            
            m_sitePtr->getWidget()->getWebView()->page()->getCurrentFrame()->load( qnReq, op, pBuff ? pBuff->data() : 0 );*/
            
            return true;
        }
    }
    
    return false;
}

//void EWANetworkAccessManager::startDelayedFakedRedirection( QObject *pObject )
//{
//    qDebug() << m_sitePtr->getMsgTitle() << "startDelayedFakedRedirection:" << (m_pDelayedFakedRedirectionFrame != 0 ) << m_delayedFakedRedirectionUrl.isValid();
//    if( m_pDelayedFakedRedirectionFrame && m_delayedFakedRedirectionUrl.isValid() )
//    {
//        m_pDelayedFakedRedirectionFrame->load( m_delayedFakedRedirectionUrl );
//        qApp->processEvents();
//        m_pDelayedFakedRedirectionFrame = 0;
//        m_delayedFakedRedirectionUrl.clear();
//    }
//}
        
void mimeTypeFromReply( QNetworkReply *pReply, const QByteArray& baReceivedData, QString& strType )
{
    const QByteArray content_type_title( "content-type" );
    const QByteArray content_length_title( "content-length" );
    if( !pReply )
        return;
        
    quint64 quiContentLength = 0;
    if( pReply->hasRawHeader( content_length_title ) )
    {
        quiContentLength = pReply->rawHeader( content_length_title ).toLongLong();
    }
    else
    {
        quiContentLength = baReceivedData.length();
    }
    
    QByteArray baMimeType;
    if( pReply->hasRawHeader( content_type_title ) )
    {
        baMimeType = pReply->rawHeader( content_type_title );
    }
    else if( quiContentLength )
    {
        baMimeType = "text/plain";
    }
    
    strType = QString( baMimeType );
    
    qDebug() << "mymeTipe is:" << strType;
}
void EWANetworkAccessManager::slotFakedRequestFinished()
{
    qDebug() << "slotFakedRequestFinished" << m_sitePtr->getMsgTitle();
    QNetworkReply *pRepl = qobject_cast<QNetworkReply *>( sender() );
    if( !hasFakedRequests()
        || !pRepl
        || !m_pBuffersMap->value( pRepl )
        || !m_sitePtr )
    {
        qWarning() << "EWANetworkAccessManager::slotFakedRequestFinished - don't proceed";
        return;
    }
    
    qApp->processEvents();
    
    m_bFakeStarted = false;

    QBuffer *pBuff = m_pBuffersMap->value( pRepl );

    m_pBuffersMap->remove( pRepl );
    delete pBuff;

    int iStatusCode = pRepl->attribute( QNetworkRequest::HttpStatusCodeAttribute ).toInt();
    if( 200 != iStatusCode )
        qWarning( "EWANetworkAccessManager::slotFakedRequestFinished(): response code - %d", iStatusCode );

    QUrl redirectUrl = pRepl->attribute( QNetworkRequest::RedirectionTargetAttribute ).toUrl();
    
    QByteArray baRecivedData = pRepl->readAll();
    QString strMimeType;
    mimeTypeFromReply( pRepl, baRecivedData, strMimeType );
    
    //-- page's content can contain only instructions
    //-- for redirect - forceing it here:
    if( redirectUrl.isValid() )
    {
        if( redirectUrl.isRelative() )
        {
            redirectUrl = pRepl->url().resolved( redirectUrl );
        }
        
        qWarning( "EWANetworkAccessManager::slotFakedRequestFinished(): rederecting to \"%s\"", qPrintable( redirectUrl.toString() ) );
        
        m_sitePtr->getWebView()->page()->getCurrentFrame()->load( redirectUrl );
    }
    else if( baRecivedData.isEmpty() )
    {
        qWarning( "EWANetworkAccessManager::slotFakedRequestFinished(): received content and redirect url are empty. Set blank page." );
    }
    else
    {
        QWebFrame *pFrame = m_sitePtr->getWebView()->page()->getCurrentFrame();
        QUrl baseUrl = pFrame->url();
        if( baseUrl.isRelative() )
        {
            baseUrl = m_sitePtr->getWebView()->page()->mainFrame()->url().resolved( baseUrl );

        }
        
        if( strMimeType.contains( "text/html", Qt::CaseInsensitive ) )
            m_sitePtr->getWebView()->setHtml( baRecivedData, baseUrl, false, pFrame );
        else
            pFrame->setContent( baRecivedData, strMimeType, baseUrl );
    }
    
    pRepl->deleteLater();
    qApp->processEvents();
}

bool EWANetworkAccessManager::hasFakedRequests() const
{
    return !m_pBuffersMap->isEmpty();
}

void EWANetworkAccessManager::clearFakedRequests()
{
    if( hasFakedRequests() )
    {
        QMapIterator< QNetworkReply*, QBuffer* > it( *m_pBuffersMap );
        while( it.hasNext() )
        {
            it.next();
            QNetworkReply* pReply = it.key();
            if( pReply )
            {
                disconnect( pReply, SIGNAL(finished()),
                     this, SLOT(slotFakedRequestFinished()) );
                pReply->abort();
                pReply->deleteLater();
            }
            
            QBuffer* pBuff = it.value();
            if( pBuff )
            {
                delete pBuff;
            }
        }
        
        m_pBuffersMap->clear();
    }
}

void EWANetworkAccessManager::clear4NewSession()
{
    m_pCookies->clear();
    m_iCurrentRoutePoint = 0;
    clearFakedRequests();
    /*m_pDelayedFakedRedirectionFrame = 0;
    m_delayedFakedRedirectionUrl.clear();*/
}

#ifndef QT_NO_OPENSSL
void EWANetworkAccessManager::slotProcesSslErrors( QNetworkReply *reply,
    const QList<QSslError> &errors )
{
    foreach( QSslError err, errors )
        qWarning( "SSL Error: %s", qPrintable( err.errorString() ) );
    if( ignoreSSLErrors() )
    {
        reply->ignoreSslErrors();
    }
}
#endif //-- QT_NO_OPENSSL
 QNetworkDiskCache *EWANetworkAccessManager::cache() const
{
    return m_pCache;
}

void EWANetworkAccessManager::setSite( EWASiteHandle *pSite )
{
    if( pSite )
    {
        m_sitePtr = pSite;
        m_pGesturesRecorder->setSite( m_sitePtr );
        m_pGesturesPlayer->setSite( m_sitePtr );
        m_pHttpPackets->setSite( m_sitePtr );
    }
}

void EWANetworkAccessManager::save( QSettings *pSettings )
{
    if( pSettings )
    {
        getGesturesRecorder()->save( pSettings );
        getHttpPackets()->save( pSettings );
    }
}

void EWANetworkAccessManager::load( QSettings *pSettings )
{
    if( pSettings )
    {
        getGesturesPlayer()->load( pSettings );
        getHttpPackets()->load( pSettings );
    }
}

void EWANetworkAccessManager::cloneSettings( const EWANetworkAccessManager *pOther )
{
    if( pOther )
    {
        m_pGesturesPlayer->cloneSettings( pOther->getGesturesPlayer() );
        getHttpPackets()->cloneSettings( pOther->getHttpPackets() );
    }
}

void EWANetworkAccessManager::setCommonDelayForActions( QVector<EWAUserActionsCollection*> *pPages, int iDelay ) const
{
    if( pPages )
    {
        iDelay = qMax( iDelay, EWAUserActionsProcessor::getMinActDelay() );
        
        for( int i = 0; i<pPages->count(); i++ )
        {
            EWAUserActionsCollection *pActions = pPages->at( i );
            for( int j = 0; j < pActions->getActionsCount(); j ++ )
            {
                pActions->getActionsPtr()->at( j )->setTime( iDelay );
            }
        }
    }
}

void EWANetworkAccessManager::initLastError()
{
    m_lastError = QNetworkReply::NoError;
    m_strLastError = tr( "Uncknown error (Error code 0)" );
}

/*void showRequest( QNetworkRequest *pReq )
{
    qDebug() << "showRequest";
    qDebug() << pReq->url();
    foreach( QByteArray rawHeader, pReq->rawHeaderList() )
    {
        qDebug() << rawHeader << ":" << pReq->rawHeader( rawHeader );
    }
}

void showReply( QNetworkReply *pReq )
{
    qDebug() << "showReply";
    qDebug() << pReq->url();
    foreach( QByteArray rawHeader, pReq->rawHeaderList() )
    {
        qDebug() << rawHeader << ":" << pReq->rawHeader( rawHeader );
    }
    
    qDebug() << "QNetworkRequest::HttpStatusCodeAttribute" << pReq->attribute( QNetworkRequest::HttpStatusCodeAttribute );
    qDebug() << "QNetworkRequest::HttpReasonPhraseAttribute" << pReq->attribute( QNetworkRequest::HttpReasonPhraseAttribute );
    qDebug() << "QNetworkRequest::RedirectionTargetAttribute" << pReq->attribute( QNetworkRequest::RedirectionTargetAttribute );
    qDebug() << "QNetworkRequest::ConnectionEncryptedAttribute" << pReq->attribute( QNetworkRequest::ConnectionEncryptedAttribute );
    qDebug() << "QNetworkRequest::CacheLoadControlAttribute" << pReq->attribute( QNetworkRequest::CacheLoadControlAttribute );
    qDebug() << "QNetworkRequest::CacheSaveControlAttribute" << pReq->attribute( QNetworkRequest::CacheSaveControlAttribute );
    qDebug() << "QNetworkRequest::SourceIsFromCacheAttribute" << pReq->attribute( QNetworkRequest::SourceIsFromCacheAttribute );
    qDebug() << "QNetworkRequest::DoNotBufferUploadDataAttribute" << pReq->attribute( QNetworkRequest::DoNotBufferUploadDataAttribute );
    qDebug() << "QNetworkRequest::HttpPipeliningAllowedAttribute" << pReq->attribute( QNetworkRequest::HttpPipeliningAllowedAttribute );
    qDebug() << "QNetworkRequest::HttpPipeliningWasUsedAttribute" << pReq->attribute( QNetworkRequest::HttpPipeliningWasUsedAttribute );
}*/

void EWANetworkAccessManager::slotRequestFinished( QNetworkReply *pReply )
{
    if( !pReply )
    {
        initLastError();
    }
    else
    {
        m_lastError = pReply->error();
        
        m_strLastError = tr( "%1 (Error code %2)" )
            .arg( EWAApplication::translate( this, pReply->errorString().toUtf8().data() ) )
            .arg( pReply->error() );
        
        if( QNetworkReply::NoError != m_lastError 
            && 
                ( !pReply->attribute( QNetworkRequest::HttpStatusCodeAttribute ).isNull()
                    && pReply->attribute( QNetworkRequest::HttpStatusCodeAttribute ).toInt() != 200
                ) )
        {
            //emit signalErrorOccured( pReply->url().toString(), m_strLastError );
            qWarning() << "EWANetworkAccessManager::slotRequestFinished" << m_strLastError << m_sitePtr->getMsgTitle();
            /*qDebug() << "request was:";
            showRequest( &pReply->request() );
            qDebug() << "reply is:";
            showReply( pReply );*/
        }
    }
}

QString EWANetworkAccessManager::lastErrorString() const
{
    return m_strLastError;
}