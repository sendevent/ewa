/*******************************************************************************
**
** file: ewa_useractionsrecorder.h
**
** class: EWAUserActionsRecorder
**
** description:
** class for handling keyboard/mouse events on webpages through JavaScript
**
** 14.10.2009
**
** sendevent@gmail.com
**
*******************************************************************************/

#ifndef EWA_USERACTIONSRECORDER_H
#define EWA_USERACTIONSRECORDER_H

#include "ewa_useractionsprocessor.h"

class EWAUserActionsRecorder : public EWAUserActionsProcessor
{
    Q_OBJECT
    
    public:    
        EWAUserActionsRecorder( QObject *pParent = 0 );
        ~EWAUserActionsRecorder();

        virtual void nextPageReady();
        virtual void reset();
        void rememberEvent( QEvent *pEvent );    

    public slots:
        virtual void slotTimerTicked();
};
#endif //-- EWA_USERACTIONSRECORDER_H
