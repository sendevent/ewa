/*******************************************************************************
**
** file: ewa_sitewidgetlabel.cpp
**
** class: EWATextLabel
**
** description:
** Info control for EWASiteWidget. Showing in message title area time/download
** progress and text description, like as in EWASitesModelView.
**
** 29.05.2009
**
** ewauthor@indatray.com
**
*******************************************************************************/

#include "ewa_textlabel.h"
#include "ewa_application.h"

#include <QPainter>

#define BGR_OPACITY 0.5
#define HSV_MAX_V 180


EWATextLabel::EWATextLabel( QWidget *pParent )
:QLabel( pParent )
{
    m_ggColor = Qt::white;
    setColor( Qt::blue ); 

}

EWATextLabel::~EWATextLabel()
{
}

 void EWATextLabel::paintEvent( QPaintEvent *e )
{   
    drawBackground();
    QLabel::paintEvent( e );
}

void EWATextLabel::drawBackground()
{
    QRect fullRect = rect();
    QRect roundedRect( QPoint( fullRect.topLeft().x()+1, fullRect.topLeft().y() ), QPoint( fullRect.width()-1, fullRect.height() ) );
    
    
    QPainter painter( this );
    
    painter.fillRect( fullRect, Qt::transparent );
    
    painter.setPen( Qt::NoPen );
    painter.setBrush( m_color );
    
    painter.setRenderHints( EWAApplication::getRenderHints() );
    painter.setOpacity(0.5);
    painter.drawRoundedRect( roundedRect, 5, 5, Qt::AbsoluteSize );
}

void EWATextLabel::setColor( const QColor& color )
{
    if( color.isValid() )
    {
        m_color = color;
        updateBackgroundGradient();
        QPalette pal = palette();
        pal.setColor( QPalette::WindowText, QColor( 255 - m_color.red(), 255 - m_color.green(), 255 - m_color.blue() ) );
        setPalette( pal );
    }
}

QRect EWATextLabel::getRectWithText( const QString& str ) const
{
    QString txt = str;
    if( txt.isEmpty() )
    {
        txt = text();
    }
    
    QRect r = fontMetrics().boundingRect( this->rect(), alignment(), txt );
    r.setWidth( r.width()+2 );
    r.setHeight( r.height()+2 );
    
    return r;
}

 void EWATextLabel::resizeEvent( QResizeEvent *e )
{
    QLabel::resizeEvent( e );
    updateBackgroundGradient();
}

bool EWATextLabel::isDarckColor( const QColor& color ) const
{
    return color.value() < HSV_MAX_V;
}

void EWATextLabel::updateBackgroundGradient()
{
    m_bckgrndGradient.setColorAt( 0.00, m_ggColor );
    m_bckgrndGradient.setColorAt( 0.05, m_color );
    m_bckgrndGradient.setColorAt( 0.95, m_color );
    m_bckgrndGradient.setColorAt( 1.00, m_ggColor );
    
    m_bckgrndGradient.setStart( 0, 0 );
    m_bckgrndGradient.setFinalStop( getRectWithText().width(), 0 );
}

void EWATextLabel::setText( const QString& text )
{
    QLabel::setText( fontMetrics().elidedText( text, Qt::ElideMiddle, maximumWidth() ).prepend( " " ) ); //-- hehe =)
    resize( getRectWithText().size() );
    updateBackgroundGradient();
}


void EWATextLabel::clear()
{
    QLabel::clear();
    //-- hide();
}
