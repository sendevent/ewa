#include "ewa_splashscreen.h"
#include "ewa_application.h"

#include <QPainter>
#include <QBitmap>
#include <QTime>
#include <QDebug>

EWASplashScreen::EWASplashScreen()
:QSplashScreen(  )
,m_pixmap( ":/images/splash.png" )
,m_lastMessage("")
,m_msgColor( Qt::black )
,m_iAlignment( Qt::AlignLeft )
{
    setWindowFlags( windowFlags() | Qt::WindowStaysOnTopHint );
    
    setPixmap( m_pixmap );
    setupMask();
}

//-- corrects image's borders
void EWASplashScreen::setupMask()
{
    QPixmap maskPixmap( m_pixmap );
    
    QPainter painter( &maskPixmap );
    painter.setRenderHints( EWAApplication::getRenderHints() );
    painter.setBrush( Qt::black );
    
    painter.drawRoundedRect( QRect( 3, 3, 250, 250 ), 65.3, 65.3 );
    
    setMask( maskPixmap.createMaskFromColor( Qt::black, Qt::MaskOutColor ) );
}

EWASplashScreen::~EWASplashScreen()
{
}

 void EWASplashScreen::paintEvent( QPaintEvent *event )
{
    if( m_lastMessage.isEmpty() )
    {
        return;
    }
    
    QRect textRect = fontMetrics().boundingRect( m_lastMessage );
    QRect r = rect();
    QPoint txtPnt( r.left()+30, r.bottom()-30 );
    textRect.moveCenter( txtPnt );
    
    QPainter p( this );
    if( EWAApplication::settings()->m_pGroupApp->useRichWM() )
    {
        p.fillRect( r, Qt::transparent );
    }
    p.setPen( m_msgColor );
    p.drawText( txtPnt, m_lastMessage );
    
    QSplashScreen::paintEvent( event );
}

void EWASplashScreen::showMessage(const QString &message, int alignment, const QColor &color )
{
    m_lastMessage = message;
    m_msgColor = color;
    m_iAlignment = alignment;
    
    if( isVisible() )
        repaint();
}

void EWASplashScreen::show()
{
    showMessage( tr( "Starting..." ), Qt::AlignBottom|Qt::AlignHCenter, Qt::white );
    
    EWAApplication::makeWidgetCentered( this );
    
    if( EWAApplication::settings()->m_pGroupApp->useRichWM() ) 
    {
        setWindowOpacity( 0.0 );
        
        QSplashScreen::show();
        
        QTime time;
        time.start();
        for( double i = 0.0; i <= 0.9999; i += 0.005 )
        {
            setWindowOpacity( i );
            update();
            qApp->processEvents();
            
            if( time.elapsed() > 2000 || windowOpacity() > 0.9999 )
            {
                setWindowOpacity( 1.0 );
                break;
            }
        }
    }
    else
    {
        QSplashScreen::show();
    }
}

void EWASplashScreen::closeSexy( QWidget *mainWin )
{
    if( EWAApplication::settings()->m_pGroupApp->useRichWM() ) 
    {
        QTime time;
        time.start();
        for( double i = 0.9995; i >= 0.0005; i -= 0.005 )
        {
            setWindowOpacity( i );
            update();
            qApp->processEvents();
            
            if( time.elapsed() > 2000 || windowOpacity() < 0.0005 )
            {
                setWindowOpacity( 0.0 );
                break;
            }
        }
    }
    
    QSplashScreen::finish( mainWin );
}