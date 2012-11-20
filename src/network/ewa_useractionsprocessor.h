/*******************************************************************************
**
** file: ewa_useractionsprocessor.h
**
** class: EWAUserActionsProcessor
**
** description:
** base for classes of handlers keyboard/mouse events on webpages through JavaScript
**
** 14.10.2009
**
** ewauthor@indatray.com
**
*******************************************************************************/
#ifndef EWA_USERACTIONSPROCESSOR_H
#define EWA_USERACTIONSPROCESSOR_H

#include <QObject>

#include "ewa_useractionscollection.h"
#include "ewa_webview.h"
#include "ewa_object.h"

class EWASiteHandle;
class EWAUserAction;
class EWATimer;
class EWAUserActionsProcessor : public QObject, public EWAObject
{
    Q_OBJECT
    
    public:    
        EWAUserActionsProcessor( QObject *pParent = 0 );
        ~EWAUserActionsProcessor();

        void setSite( EWASiteHandle *pSite );
        void setWebView( EWAWebView *pWebView ) { m_pWebView = pWebView; }
        
        QVector<EWAUserActionsCollection*>* getPagesPtr() const {return m_pagesActionsCollectionPtr;}
        void setPagesPtr( QVector<EWAUserActionsCollection*>* pCollection );
        int getPagesCount() const {return getPagesPtr()->count(); }

        virtual void stop();
        virtual void reset();
        
        virtual void nextPageReady() {};
        
        EWAUserAction* getAction( int iPageNum, int iEventNum ) const;

        void startEventTimer();
        void stopEventTimer();
        
        virtual void cloneSettings( const EWAUserActionsProcessor *pOther );
        
        void load( QSettings *pSettings );
        void save( QSettings *pSettings );

        EWAUserActionsCollection *addPage();
        
        void sleep() {m_bPaused = true;}
        void wakeUp() {m_bPaused = false;nextPageReady();}
        
        static int getMinActDelay() { return 15;}
    protected:
        EWASiteHandle *m_sitePtr;
        EWAWebView *m_pWebView;
        QVector<EWAUserActionsCollection*>* m_pagesActionsCollectionPtr;
        EWATimer *m_pUsrActivityTimer;
        int m_iUserActionDelay;
        bool m_bPaused;
        
        void clear();
        
    public slots:
        virtual void slotTimerTicked() = 0;
};

#endif //-- EWA_USERACTIONSPROCESSOR_H
