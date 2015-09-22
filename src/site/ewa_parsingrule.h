/*******************************************************************************
**
** file: ewa_parsingrule.h
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

#ifndef EWA_SITERULE_H
#define EWA_SITERULE_H

#include "ewa_object.h"

#define EPR_DEFAULT_SRC "(.*)"
#define EPR_DEFAULT_DST ""

class QRegExp;
class EWAParsingRule : public EWAObject
{
    public:

        EWAParsingRule( 
            const QString& STRSrc = QString( EPR_DEFAULT_SRC ), 
            const QString& iDst = QString( EPR_DEFAULT_DST ),
            bool min = false, 
            bool iCS = false 
            );
        EWAParsingRule( const EWAParsingRule* pCopyIt );
        virtual ~EWAParsingRule();

        bool processString( QString& source ) const;

        bool operator==( const EWAParsingRule& other ) const;
        bool operator!=( const EWAParsingRule& other ) const;
        
        void cloneDataFrom( const EWAParsingRule& other );

        inline void setPattern( const QString& pattern ) { m_qstrPattern = pattern; }
        inline const QString& getPattern() const { return m_qstrPattern; }

        inline void setDestination( const QString& dst ) { m_qstrDestination = dst; }
        inline const QString& getDestination() const { return m_qstrDestination; }

        inline void setCaseSensitive( bool on ) { m_bIsCaseSensitive = on; }
        inline bool isCaseSensitive() const { return m_bIsCaseSensitive; }

        inline void setMinimal( bool on ) { m_bIsMinimal = on; }
        inline bool isMinimal() const { return m_bIsMinimal; }
        QRegExp makeRegExp() const;
        
        void load( QSettings *pSettings );
        void save( QSettings *pSettings );

    protected:

        QString m_qstrPattern, m_qstrDestination;
        bool m_bIsCaseSensitive, m_bIsMinimal;
        
        QString showDebug() const;
        bool editCaptured( QString& source ) const;
        bool getCaptured( QString& source ) const;
        
        void construct( 
            const QString& strSrc = QString( EPR_DEFAULT_SRC ), 
            const QString& strDst = QString( EPR_DEFAULT_DST ), 
            bool bMin = false, 
            bool bCS = false );

};
#endif //-- EWA_SITERULE_H
