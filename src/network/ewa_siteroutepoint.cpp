/*******************************************************************************
**
** file: ewa_siteroutepoint.cpp
**
** class: EWASiteRoutePoint
**
** description:
** Storable presentation of QNetworkRequest.
**
** 09.02.2009
**
** sendevent@gmail.com
**
*******************************************************************************/

#include "ewa_siteroutepoint.h"
#include "ewa_application.h"

EWASiteRoutePoint::EWASiteRoutePoint( const QNetworkRequest& rec, QNetworkAccessManager::Operation op,
    const QByteArray& data )
{
    initStore( rec, op, data );
}

EWASiteRoutePoint::EWASiteRoutePoint( const EWASiteRoutePoint *pOther )
{
    if( pOther )
    {
        initStore( pOther->getRequest(), pOther->getOperation(), pOther->getData() );
    }
}

EWASiteRoutePoint::~EWASiteRoutePoint()
{
}


void EWASiteRoutePoint::initStore( const QNetworkRequest& rec, QNetworkAccessManager::Operation op,
    const QByteArray& data )
{
    setRequest( rec );
    setData( data );
    setOperation( op );
}

void EWASiteRoutePoint::setRequest( const QNetworkRequest& rec )
{
    m_qnRequest = rec;
}

const QNetworkRequest& EWASiteRoutePoint::getRequest() const
{
    return m_qnRequest;
}

void EWASiteRoutePoint::setData( const QByteArray& data )
{
    m_qbaData = data;
}

const QByteArray& EWASiteRoutePoint::getData() const
{
    return m_qbaData;
}

void EWASiteRoutePoint::setOperation( QNetworkAccessManager::Operation op )
{
    m_qnamOperation = op;
}

QNetworkAccessManager::Operation EWASiteRoutePoint::getOperation() const
{
    return m_qnamOperation;
}

void EWASiteRoutePoint::operator=( const EWASiteRoutePoint& other )
{
    initStore( other.getRequest(), other.getOperation(), other.getData() );
}

QMultiMap< QByteArray, QByteArray > EWASiteRoutePoint::getRawHeadersMap() const
{
    QMultiMap< QByteArray, QByteArray > res;

    QList<QByteArray> raws = m_qnRequest.rawHeaderList();
    foreach( QByteArray raw, raws )
    {
        res.insert( raw, m_qnRequest.rawHeader( raw ) );
    }

    return res;
}

bool EWASiteRoutePoint::operator==( const EWASiteRoutePoint& other ) const
{
    if( m_qnRequest!=other.m_qnRequest )
        return false;
    if( m_qbaData!=other.m_qbaData )
        return false;
    if( m_qnamOperation!=other.m_qnamOperation )
        return false;

    return true;
}
bool EWASiteRoutePoint::operator!=( const EWASiteRoutePoint& other ) const
{
    return !this->operator==( other );
}

void EWASiteRoutePoint::load( QSettings *pSettings )
{
    if( pSettings )
    {
        //clear();
        
        QString urlStr = pSettings->value( QLatin1String( "EWA_REQ_URL" ), "" ).toString();
        m_qnRequest.setUrl( urlStr );

        m_qnamOperation = QNetworkAccessManager::Operation( pSettings->value( QLatin1String( "EWA_REQ_OPERATION" ), 2 ).toInt() );
        m_qbaData = pSettings->value( QLatin1String( "EWA_REQ_DATA" ), QByteArray() ).toByteArray();
        m_qbaData = EWAApplication::getXORedString( m_qbaData, m_qnRequest.url().toString().toUtf8() );

        int hdrCount = pSettings->beginReadArray( QLatin1String( "rawHeaders" ) );
        for( int hdrCounter = 0; hdrCounter < hdrCount; ++hdrCounter )
        {
            pSettings->setArrayIndex( hdrCounter );

            QByteArray name = pSettings->value( QLatin1String( "name" ), QByteArray() ).toByteArray();
            QByteArray value = pSettings->value( QLatin1String( "value" ), QByteArray() ).toByteArray();
            value = EWAApplication::getXORedString( value, name );

            m_qnRequest.setRawHeader( name, value );
        }
        pSettings->endArray();
    }
}

void EWASiteRoutePoint::save( QSettings *pSettings )
{
    if( pSettings )
    {
        pSettings->setValue( QLatin1String( "EWA_REQ_URL" ), m_qnRequest.url().toString() );
        pSettings->setValue( QLatin1String( "EWA_REQ_OPERATION" ), (int)m_qnamOperation );
        pSettings->setValue( QLatin1String( "EWA_REQ_DATA" ), 
            EWAApplication::getXORedString( m_qbaData, m_qnRequest.url().toString().toUtf8() ) );
        
        QList<QByteArray> hdrsList = m_qnRequest.rawHeaderList();
        int hdrCount = hdrsList.size();
        pSettings->beginWriteArray( QLatin1String( "rawHeaders" ) );
        for( int hdrCounter = 0; hdrCounter < hdrCount; ++hdrCounter )
        {
            pSettings->setArrayIndex( hdrCounter );

            QByteArray name = hdrsList.at( hdrCounter );
            QByteArray value = m_qnRequest.rawHeader( name );
            value = EWAApplication::getXORedString( value, name );
            
            pSettings->setValue( QLatin1String( "name" ), name );
            pSettings->setValue( QLatin1String( "value" ), value );
        }
        pSettings->endArray();
    }
}
