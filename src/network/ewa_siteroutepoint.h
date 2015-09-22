/*******************************************************************************
**
** file: ewa_siteroutepoint.h
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

#ifndef EWA_SITEROUTEPOINT_H
#define EWA_SITEROUTEPOINT_H

#include <QNetworkRequest>
#include <QNetworkAccessManager>

#include "ewa_object.h"

class EWASiteRoutePoint : public EWAObject
{
    public:
        EWASiteRoutePoint( const QNetworkRequest& rec = QNetworkRequest(),
            QNetworkAccessManager::Operation op = QNetworkAccessManager::GetOperation,
            const QByteArray& data = QByteArray() );
        EWASiteRoutePoint( const EWASiteRoutePoint *pOther );
        virtual ~EWASiteRoutePoint();

        void initStore( const QNetworkRequest& rec, QNetworkAccessManager::Operation op =
            QNetworkAccessManager::GetOperation, const QByteArray& data = QByteArray() );

        void setRequest( const QNetworkRequest& rec );
        const QNetworkRequest& getRequest() const;

        void setData( const QByteArray& data );
        const QByteArray& getData() const;

        void setOperation( QNetworkAccessManager::Operation op );
        QNetworkAccessManager::Operation getOperation() const;

        void operator=( const EWASiteRoutePoint& other );
        QMultiMap< QByteArray, QByteArray > getRawHeadersMap() const;

        inline QString getUrl() const { return m_qnRequest.url().toString(); }

        bool operator==( const EWASiteRoutePoint& other ) const;
        bool operator!=( const EWASiteRoutePoint& other ) const;
        
        void load( QSettings *pSettings );
        void save( QSettings *pSettings );

    protected:
        QNetworkRequest m_qnRequest;
        QByteArray m_qbaData;
        QNetworkAccessManager::Operation m_qnamOperation;
};

#endif //-- EWA_SITEROUTEPOINT_H
