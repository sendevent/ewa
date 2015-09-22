/*******************************************************************************
**
** file: ewa_cookiesjar.cpp
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

#include "ewa_cookiesjar.h"

EWACookiesJar::EWACookiesJar( QObject *parent )
:QNetworkCookieJar( parent )
{
}

EWACookiesJar::~EWACookiesJar( void )
{
}

void EWACookiesJar::clear()
{
    QList<QNetworkCookie> cookies = allCookies();
    cookies.clear();
    setAllCookies( cookies );
}

EWACookiesJar& EWACookiesJar::operator=( const EWACookiesJar& other )
{
    clear();

    setAllCookies( other.allCookies() );

    return *this;
}
