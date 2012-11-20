#include "ewa_timer.h"
#include "ewa_application.h"

EWATimer::EWATimer( QObject *parent )
:QObject( parent )
{
    m_iStep = EWAApplication::getOneTickPeriod();
    m_iPeriod = m_iStep;
    m_bImActive = false;
    m_iTickCounter = 0;
}

EWATimer::~EWATimer()
{
    stop();//-- to disconnect with AppTimer
}


void EWATimer::setInterval( int iPeriod )
{
    if( iPeriod == -1 )
    {
        return;
    }
    double dPeriod( iPeriod );
    if( dPeriod <= 51. )
        dPeriod = 50.;
    m_iPeriod = dPeriod/m_iStep;
}
int EWATimer::getInterval() const
{
    return m_iPeriod;
}

void EWATimer::start( int interval )
{
    stop();//-- to disconnect with AppTimer

    setInterval( interval );

    m_iTickCounter = 0;

    connect( EWAApplication::instance(), SIGNAL( signalOneTick() ),
        this, SLOT( slotAppTimerTicked() ) );

    m_bImActive = true;

    EWAApplication::instance()->registerTimer( this );
    emit signalStarted();
}

void EWATimer::stop()
{
    disconnect( EWAApplication::instance(), SIGNAL( signalOneTick() ),
        this, SLOT( slotAppTimerTicked() ) );

    m_bImActive = false;

    EWAApplication::instance()->unregisterTimer( this );
}

void EWATimer::slotAppTimerTicked()
{
    m_iTickCounter++;
    if( m_iTickCounter >= m_iPeriod )
    {
        m_iTickCounter = 0;
        emit signalTimeOut();
    }
}
