/*******************************************************************************
**
** file: ewa_useractionscollection.h
**
** class: EWAUserActionsCollection
**
** description:
** Container for store user actions on the web page.
**
** 16.10.2009
**
** ewauthor@indatray.com
**
*******************************************************************************/

#ifndef EWA_USERACTIONSCOLLECTION_H
#define EWA_USERACTIONSCOLLECTION_H

#include <QObject>
#include <QKeyEvent>
#include <QMouseEvent>

class EWAJSMouseEvent;
class EWAJSKeyboardEvent;
class QSettings;
class EWAUserAction;
class EWAWebView;
class EWAUserActionsCollection : public QObject
{
    public:
        EWAUserActionsCollection( QObject *pParent = 0 );
        EWAUserActionsCollection( const EWAUserActionsCollection* pOther );
        ~EWAUserActionsCollection();
        
        int getActionsCount() const;
        QVector< EWAUserAction* > *getActionsPtr() const;
        
        EWAUserAction* addUserAction( const QEvent *pEvent, EWAWebView *pWebView = 0 );
        EWAUserAction* addUserAction( const EWAUserAction *pSrcAction );
        
        virtual void save( QSettings *pSettings );
        virtual void load( QSettings *pSettings );

        void clear();
        
        int getMinDelay() const;
        int getMaxDelay() const;

        void cloneDataFrom( const EWAUserActionsCollection *pCollection );
        
    protected:
        QVector< EWAUserAction* > *m_pUserActions;

        void saveEvent( QSettings *pSettings, QEvent *pEvent );

        QEvent *loadEvent( QSettings *pSettings );
        QMouseEvent *loadMouseEvent( QSettings *pSettings, QEvent::Type type );
        QKeyEvent *loadKeyEvent( QSettings *pSettings, QEvent::Type type );
};

#endif //-- EWA_USERACTIONSCOLLECTION_H
