/*******************************************************************************
**
** file: ewa_terminal.cpp
**
** class: EWATerminal
**
** description:
** Unix-like "console" with autotyping ( for "about" dialog )
**
** 23.03.2009
**
** ewauthor@indatray.com
**
*******************************************************************************/

#include "ewa_terminal.h"

#include <QKeyEvent>
#include <QPainter>
#include <QPixmap>

#include <math.h>

EWATerminal::EWATerminal( QWidget *parent )
:QTextEdit( parent )
{
    m_pPixmap = new QPixmap( ":/images/splash.png" );
    m_bForcedOutput = false;
}

EWATerminal::~EWATerminal()
{
    delete m_pPixmap;
}

 void EWATerminal::keyReleaseEvent( QKeyEvent * event )
{
    if( event->key() == Qt::Key_Down )
    {
        m_bForcedOutput = false;
    }
    
    event->accept();
}

 void EWATerminal::keyPressEvent ( QKeyEvent * event )
{
    if( event->key() == Qt::Key_Enter )
    {
        textCursor().insertText( tr( "\n???? ??? ?? ??????? UTF-8 ;-)\n" ) );
        ensureCursorVisible();
    }
    else if( event->key() == Qt::Key_Escape )
    {
        emit signalAltF1Pressed();
    }
    else if( event->key() == Qt::Key_Down )
    {
        m_bForcedOutput = true;
    }
    else
    {
        this->textCursor().insertText( event->text() );
    }
    
    event->accept();
    
}

 void EWATerminal::mousePressEvent( QMouseEvent *event )
{
    if( event->button() == Qt::RightButton )
        QTextEdit::mousePressEvent( event );
}

 void EWATerminal::mouseReleaseEvent( QMouseEvent *event )
{
    if( event->button() == Qt::RightButton )
        QTextEdit::mouseReleaseEvent( event );
}

 void EWATerminal::mouseDoubleClickEvent( QMouseEvent *event )
{
    Q_UNUSED( event );
}

 void EWATerminal::paintEvent( QPaintEvent *event )
{
    QRect r = this->viewport()->rect();
    QPainter p( this->viewport() );
    p.fillRect( r, Qt::black );
    p.setOpacity( 0.35 );
    p.drawPixmap( r, m_pPixmap->scaled( r.size() ) );
    
    QTextEdit::paintEvent( event );
}

double EWATerminal::getDelayPeriod( double /*min*/, double /*max*/ ) const
{
    double dRes = .0;
    /*if( !m_bForcedOutput )
    {
        double init = qrand();
        dRes = floor( min + ( init/RAND_MAX ) *10* ( max - min + 1 ) );
    }*/
    
    return dRes;
}