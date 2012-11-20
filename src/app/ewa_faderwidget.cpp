#include <QtGui>
#include <QTimer>
#include "ewa_faderwidget.h"

EWAFaderWidget::EWAFaderWidget(QWidget *parent)
    : QWidget(parent)
{
    if ( parent )
        m_startColor = parent->palette().window().color();
    else
        m_startColor = Qt::white;

    m_iCurrentAlpha = 0;
    m_iDuration = 333;

    m_pTimer = new QTimer( this );
    connect( m_pTimer, SIGNAL(timeout()), this, SLOT(update()) );

    setAttribute( Qt::WA_DeleteOnClose );
    resize( parent->size() );
}

void EWAFaderWidget::start()
{
    m_iCurrentAlpha = 255;
    m_pTimer->start( 33 );
    show();
}

void EWAFaderWidget::paintEvent(QPaintEvent *)
{
    QPainter painter( this );
    QColor semiTransparentColor = m_startColor;
    semiTransparentColor.setAlpha( m_iCurrentAlpha );
    painter.fillRect( rect(), semiTransparentColor );

    m_iCurrentAlpha -= 255 * m_pTimer->interval() / m_iDuration;
    if( m_iCurrentAlpha <= 0 ) 
    {
        m_pTimer->stop();
        close();
    }
}
