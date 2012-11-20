#include "ewa_htmlselectordlg.h"

#include <QPaintEvent>
#include <QPainter>

EWAHtmlSelectorDlg::EWAHtmlSelectorDlg( QWidget *pParent )
:QWidget( pParent )
{
    ui.setupUi( this );
}

EWAHtmlSelectorDlg::~EWAHtmlSelectorDlg()
{
}

void EWAHtmlSelectorDlg::on_pButtonOk_clicked()
{
    emit signalOk();
}

void EWAHtmlSelectorDlg::on_pButtonCancel_clicked()
{
    emit signalCancel();
}

void EWAHtmlSelectorDlg::paintEvent( QPaintEvent* )
{
    QRect r = rect();
    
    QPainter p( this );
    p.setPen( Qt::NoPen );
    p.setBrush( Qt::blue );
    p.setOpacity(0.6);
    p.drawRoundedRect( r, 14, 14 );
    
}

void EWAHtmlSelectorDlg::mouseMoveEvent( QMouseEvent *event )
{
    QPoint click = event->globalPos();

    if( !m_ptPrevPos.isNull() && event->buttons() == Qt::LeftButton )
    {
        QRect currentGeom = geometry();
        QPoint newPos = mapToGlobal( this->pos() );
        QPoint delta = click - m_ptPrevPos;
        newPos += delta;
        currentGeom.moveTo( mapFromGlobal( newPos ) );

        move( currentGeom.topLeft() );
    }
    
    m_ptPrevPos = click;
    
    QWidget::mouseMoveEvent( event );
}

void EWAHtmlSelectorDlg::mousePressEvent( QMouseEvent *event )
{
    m_ptPrevPos = event->globalPos();
    event->accept();
}

void EWAHtmlSelectorDlg::mouseReleaseEvent( QMouseEvent *event )
{
    m_ptPrevPos = QPoint();
    event->accept();
}
