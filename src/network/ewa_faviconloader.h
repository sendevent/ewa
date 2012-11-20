/*******************************************************************************
**
** file: ewa_faviconloader.h
**
** class: EWAFaviconLoader
**
** description:
** workaround for QtWebKit's problems with handling favicons on windows
**
** 11.12.2009
**
** ewauthor@indatray.com
**
*******************************************************************************/

#ifndef EWA_FAVICONLOADER_H
#define EWA_FAVICONLOADER_H

//#ifdef Q_OS_WIN
//    #define EWA_FAVICONS
//#endif //--Q_OS_WIN

#include <QThread>

class QIcon;
class QNetworkReply;
class QNetworkRequest;
class QNetworkAccessManager;
class EWAFaviconLoader : public QThread
{
    Q_OBJECT
    
    public:
        EWAFaviconLoader( QObject *pParent = 0);
        virtual ~EWAFaviconLoader();
        
        bool setTargetUrl( const QString& strDomain );
        void setUserAgent( const QString& strUA ) {m_strUserAgent = strUA;}
        
        void stop() { if( isRunning() ) { quit(); wait(); } }
    protected:
        QString m_strTargetUrl;
        QString m_strUserAgent;
        
        void run();
    
    protected slots:
        void slotRecived( QNetworkReply *pReply );
    
    signals:
        void signalIconRecived( const QIcon& icon );
};

#endif //-- EWA_FAVICONLOADER_H
