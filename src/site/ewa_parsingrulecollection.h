/*******************************************************************************
**
** file: ewa_parsingrulecollection.h
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

#ifndef EWA_PARSINGRULECOLLECTION_H
#define EWA_PARSINGRULECOLLECTION_H

#include <QObject>
#include <QVector>

#include "ewa_object.h"

class EWAParsingRule;
class EWAParsingRuleCollection : public QObject, public EWAObject
{
    Q_OBJECT

    public:
        EWAParsingRuleCollection( QObject *pParent = 0 );
        virtual ~EWAParsingRuleCollection();

        void cloneData( const EWAParsingRuleCollection *pOther );
        
        EWAParsingRule* addRule( EWAParsingRule* pCopyIt = 0 );
        EWAParsingRule* addRule( const QString& STRSrc, const QString& iDst, bool min, bool iCS );
        
        int getRulesCount() const {return m_pParsingRules->count(); }
        
        QVector< EWAParsingRule* >* getPtr2Rules() const {return m_pParsingRules;}
        
        bool parseSourceString( QString& res ) const;
        
        void load( QSettings *pSettings );
        void save( QSettings *pSettings );

        void clear();
        
    protected:
        QVector< EWAParsingRule* > *m_pParsingRules;
        
        
};

#endif //-- EWA_PARSINGRULECOLLECTION_H
