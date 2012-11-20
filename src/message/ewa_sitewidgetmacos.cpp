#include <QtGlobal>

#if defined(Q_OS_MAC)

#include "ewa_sitewidgetmacos.h"

EWASiteWidgetMacos::EWASiteWidgetMacos( QWidget *pParent )
:EWASiteWidget( pParent )
{
    m_iFlagsMax = windowFlags() | Qt::FramelessWindowHint;
    m_iFlagsNorm = windowFlags() | Qt::FramelessWindowHint;
    setWindowFlags( m_iFlagsNorm );
}
EWASiteWidgetMacos::~EWASiteWidgetMacos()
{
}

#endif //-- Q_OS_MAC
