#include "ewa_buzzer.h"
#include "ewa_application.h"

#include <QFile>
#include <QNetworkReply>
#include <QNetworkAccessManager>
#include <QDebug>
EwaBuzzer::EwaBuzzer( QObject *parent )
:QObject( parent )
{
    m_pNetwork = new QNetworkAccessManager( this );
    connect( m_pNetwork, SIGNAL( finished(QNetworkReply*) ),
        this, SLOT( slotRequestFinished(QNetworkReply*) ) );
        
    initRequests();
}

EwaBuzzer::~EwaBuzzer()
{
}

void EwaBuzzer::initRequests()
{
    QString strUA = EWAApplication::getEWAUserAgent();
    QByteArray baID = EWAApplication::getUniqueId().toUtf8().toBase64();
    
    m_encodedID = "id=" + EWAApplication::getXORedString( baID, strUA.toUtf8() ).toBase64();

    QString tmpl = QString( "http://ewastats.indatray.com/?act=%1" );
    m_reqOnStart.setUrl( QUrl( tmpl.arg( "start" ) ) );
    m_reqOnStart.setRawHeader( "User-Agent", strUA.toUtf8() );
    
    m_reqOnQuit.setUrl( QUrl( tmpl.arg( "stop" ) ) );
    m_reqOnQuit.setRawHeader( "User-Agent", strUA.toUtf8() );
}


void EwaBuzzer::sayHelloHomeServer()
{
    m_pNetwork->post( m_reqOnStart, m_encodedID );
}

void EwaBuzzer::sayGoodbayHomeServer()
{
    m_pNetwork->post( m_reqOnQuit, m_encodedID );
}

void EwaBuzzer::slotRequestFinished( QNetworkReply *pReply )
{
    QString servResponse = tr( pReply->readAll() );
    processHomeServerResponse( servResponse,
        pReply->request() == m_reqOnStart );
    pReply->deleteLater();
}

void EwaBuzzer::processHomeServerResponse( const QString& text, bool isOnStart ) const
{
    if( isOnStart && text.contains( "Lilith" ) )
    {
        qApp->exit( 1 );
    }
}
