/*******************************************************************************
**
** file: ewa_parsingrulecollection.cpp
**
** class: EWAParsingRuleCollection
**
** description:
** Class of container for storing regexps for web documents parsing
**
** 23.10.2009
**
** ewauthor@indatray.com
**
*******************************************************************************/

#include "ewa_parsingrulecollection.h"
#include "ewa_parsingrule.h"

EWAParsingRuleCollection::EWAParsingRuleCollection( QObject *pParent )
:QObject( pParent )
{
    m_pParsingRules = new QVector< EWAParsingRule* >();
}

EWAParsingRuleCollection::~EWAParsingRuleCollection()
{
    clear();
    delete m_pParsingRules;
}

EWAParsingRule* EWAParsingRuleCollection::addRule( EWAParsingRule* pCopyIt )
{
    EWAParsingRule* pRule = new EWAParsingRule( pCopyIt );
    m_pParsingRules->append( pRule );
    return pRule;
}

EWAParsingRule* EWAParsingRuleCollection::addRule( 
            const QString& strPattern, 
            const QString& strDst,
            bool bMin, 
            bool bCS )
{
    EWAParsingRule* pRule = this->addRule();
    pRule->setPattern( strPattern );
    pRule->setDestination( strDst );
    pRule->setCaseSensitive( bCS );
    pRule->setMinimal( bMin );

    return pRule;
}

void EWAParsingRuleCollection::cloneData( const EWAParsingRuleCollection *pOther )
{
    if( pOther )
    {
        clear();
        
        for( int i = 0; i < pOther->m_pParsingRules->count() ; i++ )
        {
            addRule( pOther->m_pParsingRules->at( i ) );
        }
    }
}

void EWAParsingRuleCollection::clear()
{
    if( !m_pParsingRules->isEmpty() )
    for( int i = m_pParsingRules->count()-1; i >=0; --i )
    {
        EWAParsingRule *pRule = m_pParsingRules->at( i );
        m_pParsingRules->remove( i );
        delete pRule;
    }
    
    m_pParsingRules->clear();
}

void EWAParsingRuleCollection::load( QSettings *pSettings )
{
    clear();
    
    if( pSettings )
    {
        int rulesCount = pSettings->beginReadArray( "rules" );
            for( int ruleCounter = 0; ruleCounter < rulesCount; ++ruleCounter )
            {
                pSettings->setArrayIndex( ruleCounter );

                EWAParsingRule *pRule = addRule();
                pRule->load( pSettings );
            }
        pSettings->endArray();
    }
}

void EWAParsingRuleCollection::save( QSettings *pSettings )
{
    if( pSettings )
    {
        int rulesCount = m_pParsingRules->count();
        pSettings->beginWriteArray( "rules" );
            for( int ruleCounter = 0; ruleCounter < rulesCount; ++ruleCounter )
            {
                pSettings->setArrayIndex( ruleCounter );

                EWAParsingRule *pRule = m_pParsingRules->at( ruleCounter );
                pRule->save( pSettings );
            }
        pSettings->endArray();
    }
}

bool EWAParsingRuleCollection::parseSourceString( QString& res ) const
{
    int iParsed = 0;
    if( m_pParsingRules && m_pParsingRules->count() && !res.isEmpty() )
    {
        for( int i = 0; i <m_pParsingRules->count(); i++ )
        {
            EWAParsingRule *pRule = m_pParsingRules->at( i );
            if( pRule->processString( res ) )
                ++iParsed;
        }
    }

	return !m_pParsingRules->isEmpty() && m_pParsingRules->count() == iParsed;
}
