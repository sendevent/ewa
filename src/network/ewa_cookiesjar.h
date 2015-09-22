/*******************************************************************************
**
** file: ewa_cookiesjar.h
**
** class: EWACookiesJar
**
** description:
** Internal cookies implementation.
**
** 09.02.2009
**
** sendevent@gmail.com
**
*******************************************************************************/

#ifndef EWA_COOKIESJAR_H
#define EWA_COOKIESJAR_H

#include <QNetworkCookieJar>

class EWACookiesJar: public QNetworkCookieJar
{
    Q_OBJECT
    public:
        EWACookiesJar( QObject *parent = 0 );
        virtual ~EWACookiesJar();

        EWACookiesJar& operator=( const EWACookiesJar& other );

        void clear();
};

#endif //-- EWA_COOKIESJAR_H
