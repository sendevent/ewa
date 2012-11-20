#ifndef EWA_TIMER_H
#define EWA_TIMER_H

#include <QObject>

class EWATimer : public QObject
{
    Q_OBJECT
    
    public:
        EWATimer( QObject *parent = 0 );
        virtual ~EWATimer();

        void setInterval( int period );
        int getInterval() const;

        bool isActive() const {return m_bImActive;}

    protected:
        int m_iStep
            ,m_iPeriod
            ,m_iTickCounter;

        bool m_bImActive;

    public slots:
        void start( int interval = -1 );
        void stop();

    protected slots:
        void slotAppTimerTicked();

    signals:
        void signalTimeOut();
        void signalStarted();
};

#endif //-- EWA_TIMER_H
