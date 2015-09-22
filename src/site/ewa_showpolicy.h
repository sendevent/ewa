/*******************************************************************************
**
** file: ewa_showpolicy.h
**
** class: EWAShowPolicy
**
** description:
** Class for determine show/don't show message widget if web document (not)changed
**
** 22.10.2009
**
** sendevent@gmail.com
**
*******************************************************************************/

#ifndef EWA_SHOWPOLICY_H
#define EWA_SHOWPOLICY_H

#include "ewa_object.h"

#include <QList>
#include <QRegExp>

class EWAShowPolicy : public QObject, public EWAObject
{
    Q_OBJECT
    
    public:
        EWAShowPolicy( bool bFinded = true, const QRegExp& rx = QRegExp( "(.*)" ), bool used = false, QObject *pParent = 0 );
        EWAShowPolicy( const EWAShowPolicy *pOther );
        virtual ~EWAShowPolicy();
        
        EWAShowPolicy& operator=( const EWAShowPolicy *pOther );
        
        void load( QSettings *pSettings );
        void save( QSettings *pSettings );
        
        bool showIfChanged() const { return m_bShowIfChanged; }
        void setShowIfChanged( bool bOn ) { m_bShowIfChanged = bOn; }
        
        QRegExp getRegExp() const {return m_rx;}
        void setRegExp( const QRegExp& rx ) {m_rx = rx;}

        bool isUsed() const {return m_bUsed;}
        void setUsed( bool bUsed ) { m_bUsed = bUsed; }
        
        const QList<int> getPath2Element() const {return m_path2element;}
        void setPath2Element( const QList<int>& list ) {m_path2element = list;}
        
        const QList<int> getPath2ElementsFrame() const {return m_path2frame;}
        void setPath2ElementsFrame( const QList<int>& list ) {m_path2frame = list; }
        
        
        QString lookInHtml( const QString& strHtml ) const;
        
    protected:
        bool m_bShowIfChanged;
        QRegExp m_rx;
        bool m_bUsed;
        QList<int> m_path2element, m_path2frame;
};
#endif //-- EWA_SHOWPOLICY_H
