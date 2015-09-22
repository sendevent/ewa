/*******************************************************************************
**
** file: ewa_useractionsplayer.h
**
** class: EWAUserActionsPlayer
**
** description:
** class for handling keyboard/mouse events on webpages through JavaScript
**
** 14.10.2009
**
** sendevent@gmail.com
**
*******************************************************************************/

#ifndef EWA_USERaCTIONSPLAYER_H
#define EWA_USERaCTIONSPLAYER_H

#include "ewa_useractionsprocessor.h"

class EWAUserActionsPlayer : public EWAUserActionsProcessor
{
    Q_OBJECT
    
    public:    
        EWAUserActionsPlayer( QObject *pParent = 0 );
        ~EWAUserActionsPlayer();

        void startPlay();
        virtual void nextPageReady();
        virtual void stop();
        virtual void reset();
        
        bool isReplayed() const;

    public slots:
        virtual void slotTimerTicked();    

    protected:
        EWAUserAction *m_pCurrUsrAct;
        
        int m_iPageCounter
            ,m_iEventCounter
            ,m_iExecutedActionsCounter
            ,m_iOverallEventCounter;
            
        void executeCurrentEvent();
        
        EWAUserAction* getAction( int iEventNum ) const;
        
        int currentPageNumber() const;
        
    signals:
        void signalUserActivitiesReplayProgress( int percents );
        void signalUserActivitiesReplayStarted();
        void signalUserActivitiesReplayFinished();
};
#endif //-- EWA_USERaCTIONSPLAYER_H
