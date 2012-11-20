#include "ewa_colorchooser.h"
#include "ewa_application.h"

#include <QMouseEvent>
#include <QDesktopWidget>

EWAColorChooser::EWAColorChooser( QWidget *parent )
:QWidget( parent )
{
    ui.setupUi( this );

    m_pMyCursor = QCursor( QPixmap( QLatin1String( ":/images/colorselectorcurs.png" ) ), 0, 21 );
    m_bSelectionStarted = m_bCursorHandled = false;
}

EWAColorChooser::~EWAColorChooser()
{
}


 void EWAColorChooser::mousePressEvent( QMouseEvent *event )
{
    if( ui.frame->geometry().contains( event->pos() ) )
    {
        setCursorInternal();
    }
}

 void EWAColorChooser::mouseMoveEvent( QMouseEvent *event )
{
    if( ui.frame->geometry().contains( event->pos() ) && m_bCursorHandled )
    {
        EWAApplication::setOverrideCursor( m_pMyCursor );
        resetCursorInternal();
    }

    if( m_bSelectionStarted )
    {
        choseColor( QCursor::pos() );
    }
}

 void EWAColorChooser::mouseReleaseEvent( QMouseEvent *event )
{
    Q_UNUSED( event );
    EWAApplication::restoreOverrideCursor();
    choseColor( QCursor::pos() );
    resetCursorInternal();
    m_bSelectionStarted = false;
}

void EWAColorChooser::setCursorInternal()
{
    ui.label->setCursor( Qt::ClosedHandCursor );
    m_bCursorHandled = true;
    m_bSelectionStarted = true;
}

void EWAColorChooser::resetCursorInternal()
{
    ui.label->setCursor( Qt::OpenHandCursor );
    m_bCursorHandled = false;
}

void EWAColorChooser::choseColor( const QPoint& globalPos ) const
{
    QDesktopWidget desktop;
    for( int i = 0; i< desktop.numScreens(); i++  )
    {
        if( desktop.screenGeometry( i ).contains( globalPos ) )
        {
            QPixmap pm = QPixmap::grabWindow( desktop.screen( i )->winId(), globalPos.x(), globalPos.y(), 1, 1 );

            emit signalColorChanged( pm.toImage().pixel( 0, 0 ) );

            break;
        }
    }
}