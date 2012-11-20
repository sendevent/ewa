/*******************************************************************************
**
** file: ewa_showpolicy.cpp
**
** class: EWAShowPolicy
**
** description:
** Class for determine show/don't show message widget if web document (not)changed
**
** 22.10.2009
**
** ewauthor@indatray.com
**
*******************************************************************************/

#include "ewa_showpolicy.h"
#include <QDebug>

EWAShowPolicy::EWAShowPolicy( bool bFinded, const QRegExp& rx, bool bUsed, QObject *pParent )
:QObject( pParent )
{
    setShowIfChanged( bFinded );
    setRegExp( rx );
    setUsed( bUsed );
}

EWAShowPolicy::EWAShowPolicy( const EWAShowPolicy *pOther )
{
    this->operator = ( pOther );
}

EWAShowPolicy::~EWAShowPolicy()
{
}

EWAShowPolicy& EWAShowPolicy::operator=( const EWAShowPolicy *pOther )
{
    setShowIfChanged( pOther->showIfChanged() );
    setRegExp( pOther->getRegExp() );
    setUsed( pOther->isUsed() );
    
    return *this;
}

void EWAShowPolicy::load( QSettings *pSettings )
{
    if( pSettings )
    {
        m_path2element.clear();
        
        m_rx.setPattern( pSettings->value( QLatin1String( "SPRXPattern" ), "(.*)" ).toString() );
        m_rx.setCaseSensitivity( Qt::CaseSensitivity( pSettings->value( "SPRXCaseSensetive", 0 ).toUInt() ) );
        m_rx.setMinimal( pSettings->value( "SPRXMinimal", false ).toBool() );
        m_bUsed = pSettings->value( "SPRXUsed", false ).toBool();
        m_bShowIfChanged = pSettings->value( "SPRXShowChanged", false ).toBool();
        
        int nodesCount = pSettings->beginReadArray( "nodes" );
            for( int nodeCounter = 0; nodeCounter < nodesCount; ++nodeCounter )
            {
                pSettings->setArrayIndex( nodeCounter );
                m_path2element += pSettings->value( QLatin1String( "node" ), -1 ).toInt();
            }
        pSettings->endArray();
        
        int frameNodesCount = pSettings->beginReadArray( "frameNodes" );
            for( int frameNodeCounter = 0; frameNodeCounter < frameNodesCount; ++frameNodeCounter )
            {
                pSettings->setArrayIndex( frameNodeCounter );
                m_path2frame += pSettings->value( QLatin1String( "frameNode" ), 0 ).toInt();
            }
        pSettings->endArray();
    }
}

void EWAShowPolicy::save( QSettings *pSettings )
{
    if( pSettings )
    {
        pSettings->setValue( QLatin1String( "SPRXPattern" ), m_rx.pattern() );
        pSettings->setValue( QLatin1String( "SPRXCaseSensetive" ), (uint)m_rx.caseSensitivity() );
        pSettings->setValue( QLatin1String( "SPRXMinimal" ), m_rx.isMinimal() );
        pSettings->setValue( QLatin1String( "SPRXUsed" ), m_bUsed );
        pSettings->setValue( QLatin1String( "SPRXShowChanged" ), m_bShowIfChanged );
        
        int nodesCount = m_path2element.count();
        pSettings->beginWriteArray( "nodes" );
            for( int nodeCounter = 0; nodeCounter < nodesCount; ++nodeCounter )
            {
                pSettings->setArrayIndex( nodeCounter );
                pSettings->setValue( QLatin1String( "node" ), m_path2element.at( nodeCounter ) );
            }
        pSettings->endArray();
        
        int frameNodesCount = m_path2frame.count();
        pSettings->beginWriteArray( "frameNodes" );
            for( int frameNodeCounter = 0; frameNodeCounter < frameNodesCount; ++frameNodeCounter )
            {
                pSettings->setArrayIndex( frameNodeCounter );
                pSettings->setValue( QLatin1String( "frameNode" ), m_path2frame.at( frameNodeCounter ) );
            }
        pSettings->endArray();
    }
}

QString EWAShowPolicy::lookInHtml( const QString& strHtml ) const
{
    QString currentMatched;
    if( !m_rx.isValid() || strHtml.isEmpty() )
    {
        return currentMatched;
    }
    
    int pos = 0;
    if( !m_rx.pattern().compare( "(.*)" ) )
    {
        currentMatched = strHtml;
    }
    else
    {
        while( ( pos = m_rx.indexIn( strHtml, pos ) ) != -1 )
        {
            int numCaps = m_rx.numCaptures();
            for( int c = 1; c <= numCaps; c++ )
            {
                currentMatched += m_rx.cap( c );
            }
            int capLength = m_rx.matchedLength();
            pos += capLength;
            if( pos == strHtml.length() || !pos || capLength < 0 )
                break;
        }
    }
    
    return currentMatched;
}