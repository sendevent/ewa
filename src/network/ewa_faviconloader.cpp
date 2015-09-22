/*******************************************************************************
**
** file: ewa_faviconloader.cpp
**
** class: EWAFaviconLoader
**
** description:
** workaround for QtWebKit's problems with handeling favicons on windows
**
** 11.12.2009
**
** sendevent@gmail.com
**
*******************************************************************************/

#include "ewa_faviconloader.h"

#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QPixmap>
#include <QIcon>
#include <QtDebug>

EWAFaviconLoader::EWAFaviconLoader( QObject *pParent )
:QThread( pParent )
{
}

EWAFaviconLoader::~EWAFaviconLoader()
{
    stop();
}

void EWAFaviconLoader::slotRecived( QNetworkReply *pReply )
{
    QUrl anotherLocation = pReply->attribute( QNetworkRequest::RedirectionTargetAttribute ).toUrl();
    
    QByteArray binaryData = pReply->readAll();
    
    bool bNoError = QNetworkReply::NoError == pReply->error();
    QString strError = pReply->errorString();
    
    QString strReqUrl = pReply->url().toString();
    int iLastDotId = strReqUrl.lastIndexOf( "." );
    QString strFormat( "ICO" );
    if( iLastDotId > 0 
        && !strReqUrl.endsWith( strFormat, Qt::CaseInsensitive ) )
    {
        strReqUrl.right( strReqUrl.length() - 1 - iLastDotId ).toUpper();
    }
    
    pReply->deleteLater();
    stop();
        
    if( anotherLocation.isValid() )
    {
        if( anotherLocation.isRelative() )
        {
            anotherLocation = QUrl( strReqUrl ).resolved( anotherLocation );
        }
        qDebug() << "EWAFaviconLoader::slotRecived: redirecting from [" 
            << strReqUrl << "] to [" << anotherLocation.toString() << "]";
        
        setTargetUrl( anotherLocation.toString() );
        start();
    }
    else
    {
        if( bNoError )
        {
            QPixmap pixmap = QPixmap::fromImage( QImage::fromData( binaryData, qPrintable( strFormat ) ) );
            
            if( pixmap.isNull() )
                qWarning() << "EWAFaviconLoader::slotRecived: received icon is null";
            else
                emit signalIconRecived( QIcon( pixmap ) );
        }
        else
        {
            qWarning() << "EWAFaviconLoader::slotRecived:" << strError;
        }
    }
}

void EWAFaviconLoader::run()
{
    QUrl url( m_strTargetUrl );
    QNetworkRequest myRequest( url );
    myRequest.setRawHeader( "User-Agent", m_strUserAgent.toUtf8() );

    QNetworkAccessManager networkMgr;

    connect( &networkMgr, SIGNAL( finished(QNetworkReply*) ),
        this, SLOT( slotRecived(QNetworkReply*) ) );
    
    networkMgr.get( myRequest );

    exec();
}

bool EWAFaviconLoader::setTargetUrl( const QString& strDomain )
{
    if( !strDomain.isEmpty() 
        && QUrl( strDomain ).isValid() )
    {
        m_strTargetUrl = strDomain;
        return true;
    }

    return false;
}
