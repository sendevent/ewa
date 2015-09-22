/*******************************************************************************
**
** file: ewa_parsingrule.cpp
**
** class: EWAParsingRule
**
** description:
** Parsing rule for getting data from target site's html body.
**
** 06.02.2009
**
** sendevent@gmail.com
**
*******************************************************************************/

#include "ewa_parsingrule.h"

#include <QObject>
#include <QRegExp>

EWAParsingRule::EWAParsingRule( const QString& strSrc, const QString& strDst, bool bMin, bool bCS )
{
    construct( strSrc, strDst, bMin, bCS );
}

EWAParsingRule::EWAParsingRule( const EWAParsingRule* pCopyIt )
{
    construct( 
        pCopyIt ? pCopyIt->getPattern() : EPR_DEFAULT_SRC,
        pCopyIt ? pCopyIt->getDestination() : EPR_DEFAULT_DST,
        pCopyIt ? pCopyIt->isMinimal() : false,
        pCopyIt ? pCopyIt->isCaseSensitive() : false
        );
}

EWAParsingRule::~EWAParsingRule()
{
}
bool EWAParsingRule::processString( QString& source ) const
{
    if( getDestination().contains( "%" ) )
        return getCaptured( source );
    else
        return editCaptured( source );
}

bool EWAParsingRule::editCaptured( QString& source ) const
{
    QString tmpBuff = source;
    QRegExp rx = this->makeRegExp();
    if( rx.isValid() )
    {
        if( !getPattern().compare( "(.*)" ) )
        {
             source = getDestination();
             return true;
        }
        int pos = 0;
        while( ( pos = rx.indexIn( tmpBuff, pos ) ) != -1 )
        {
            int count = rx.numCaptures();
            for( int c = 1; c <= count; c++ )
            {
                QString captured = rx.cap( c );
                if( captured.isEmpty() )
                    continue;

                tmpBuff = tmpBuff.replace( captured, getDestination(),
                    ( isCaseSensitive() ? Qt::CaseSensitive : Qt::CaseInsensitive ) );
            }
            pos += rx.matchedLength();
            if( !pos )
                break;
        }

        if( !tmpBuff.isEmpty() )
        {
			if( source.compare( tmpBuff, Qt::CaseInsensitive ) )
			{
				source = tmpBuff;
				return true;
			}
        }
    }
    return false;
}

bool EWAParsingRule::getCaptured( QString& source ) const
{
    QRegExp rx = this->makeRegExp();

    if( rx.isValid() )
    {
        int pos = 0;
        QString res;
        QString diff = getDestination();
        diff = diff.replace( "%", "%1" );
        if( !getPattern().compare( "(.*)" ) )
        {
             source = diff.arg( source );
             return true;
        }
        while( ( pos = rx.indexIn( source, pos ) ) != -1 )
        {
            int numCaps = rx.numCaptures();
            for( int c = 1; c <= numCaps; c++ )
            {
                res += diff.arg( rx.cap( c ) );
            }
            int capLength = rx.matchedLength();
            pos += capLength;
            if( pos == source.length() || !pos )
                break;
        }

        if( !res.isEmpty() )
        {
            source = res;
            return true;
        }
    }

    return false;
}

QRegExp EWAParsingRule::makeRegExp() const
{
    QRegExp rx( getPattern(), ( isCaseSensitive() ? Qt::CaseSensitive : Qt::CaseInsensitive ),
        QRegExp::RegExp2 );
    rx.setMinimal( isMinimal() );

    return rx;
}

QString EWAParsingRule::showDebug() const
{
    return QObject::tr( "EWAParsingRule:\n\tpattern: %1\n\tdst: %2\n\tcaseSensitivity: %3\n\tminimal: %4" )
    .arg( getPattern() ).arg( getDestination() ).arg( isCaseSensitive() ).arg( isMinimal() );
}


bool EWAParsingRule::operator==( const EWAParsingRule& other ) const
{
    if( m_qstrPattern.compare( other.m_qstrPattern ) )
        return false;
    if( m_qstrDestination.compare( other.m_qstrDestination ) )
        return false;
    if( m_bIsCaseSensitive != other.m_bIsCaseSensitive )
        return false;
    if( m_bIsMinimal != other.m_bIsMinimal )
        return false;

    return true;
}

bool EWAParsingRule::operator!=( const EWAParsingRule& other ) const
{
    return !this->operator==( other );
}

void EWAParsingRule::cloneDataFrom( const EWAParsingRule& other )
{
    setPattern( other.getPattern() );
    setDestination( other.getDestination() );
    setMinimal( other.isMinimal() );
    setCaseSensitive( other.isCaseSensitive() );
}

void EWAParsingRule::construct( const QString& strSrc, const QString& strDst, bool bMin, bool bCS )
{
    QString strCorrectPattern = strSrc;
    if( strCorrectPattern.isEmpty() )
    {
        strCorrectPattern = EPR_DEFAULT_SRC;
    }
    QString strCorrectDst = strDst;
    if( strCorrectDst.isEmpty() )
    {
        strCorrectDst = EPR_DEFAULT_DST;
    }
    
    setPattern( strCorrectPattern );
    setDestination( strCorrectDst );
    setCaseSensitive( bCS );
    setMinimal( bMin );
}

void EWAParsingRule::load( QSettings *pSettings )
{
    if( pSettings )
    {
        setPattern( pSettings->value( QLatin1String( "pattern" ) ).toString() );
        setDestination( pSettings->value( QLatin1String( "dst" ) ).toString() );
        setCaseSensitive( pSettings->value( QLatin1String( "caseSensitivity" ) ).toBool() );
        setMinimal( pSettings->value( QLatin1String( "minimal" ) ).toBool() );
    }
}

void EWAParsingRule::save( QSettings *pSettings )
{
    if( pSettings )
    {
        pSettings->setValue( QLatin1String( "pattern" ), getPattern() );
        pSettings->setValue( QLatin1String( "dst" ), getDestination() );
        pSettings->setValue( QLatin1String( "caseSensitivity" ), isCaseSensitive() );
        pSettings->setValue( QLatin1String( "minimal" ), isMinimal() );
    }
}
