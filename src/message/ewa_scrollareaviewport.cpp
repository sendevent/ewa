#include <QPainter>
#include "ewa_scrollareaviewport.h"
#include "ewa_application.h"

EWAScrollAreaViewport::EWAScrollAreaViewport( QWidget *pParent )
:QWidget( pParent )
{
}

/*virtual*/ EWAScrollAreaViewport::~EWAScrollAreaViewport()
{
}

/*virtual*/ /*void EWAScrollAreaViewport::resizeEvent( QResizeEvent *event )
{
    QWidget::resizeEvent( event );
}*/

///*virtual*/ void EWAScrollAreaViewport::paintEvent( QPaintEvent *event )
//{
//    //QWidget::paintEvent( event );   
//    Q_UNUSED( event );
//    
//    if( !isVisible() )
//    {
//        return;
//    }
//    const QRect &r = rect();
//    
//    QPainter p(this);
//    
//    //p.end();
//    
//    p.save();
//    int radius = 14;
//    int radius2 = radius*2;
//    QPainterPath clipPath;
//    clipPath.moveTo( radius, 0 );
//    clipPath.arcTo( r.right() - radius2, 0, radius2, radius2, 90, -90 );
//    clipPath.arcTo( r.right() - radius2, r.bottom() - radius2, radius2, radius2, 0, -90 );
//    clipPath.arcTo( r.left(), r.bottom() - radius2, radius2, radius2, 270, -90 );
//    clipPath.arcTo( r.left(), r.top(), radius2, radius2, 180, -90 );
//    p.setClipPath( clipPath );
//    //QPixmap titleStretch = EWAApplication::cached( tr( "%1/images/title_stretch.png" ).arg( EWAApplication::getUserAppsDirPath() ) );
//    QPixmap topLeft = EWAApplication::cached( tr( "%1/images/groupframe_topleft.png" ).arg( EWAApplication::getUserAppsDirPath() ) );
//    QPixmap topRight = EWAApplication::cached( tr( "%1/images/groupframe_topright.png" ).arg( EWAApplication::getUserAppsDirPath() ) );
//    QPixmap bottomLeft = EWAApplication::cached( tr( "%1/images/groupframe_bottom_left.png" ).arg( EWAApplication::getUserAppsDirPath() ) );
//    QPixmap bottomRight = EWAApplication::cached( tr( "%1/images/groupframe_bottom_right.png" ).arg( EWAApplication::getUserAppsDirPath() ) );
//    QPixmap leftStretch = EWAApplication::cached( tr( "%1/images/groupframe_left_stretch.png" ).arg( EWAApplication::getUserAppsDirPath() ) );
//    QPixmap topStretch = EWAApplication::cached( tr( "%1/images/groupframe_top_stretch.png" ).arg( EWAApplication::getUserAppsDirPath() ) );
//    QPixmap rightStretch = EWAApplication::cached( tr( "%1/images/groupframe_right_stretch.png" ).arg( EWAApplication::getUserAppsDirPath() ) );
//    QPixmap bottomStretch = EWAApplication::cached( tr( "%1/images/groupframe_bottom_stretch.png" ).arg( EWAApplication::getUserAppsDirPath() ) );
//    QLinearGradient lg( 0, 0, 0, r.height() );
//    QColor m_color3( 224,224,224 );
//    QColor m_color4( 255,255,255 );
//    lg.setColorAt( 0, m_color3 );
//    lg.setColorAt( 1, m_color4  );
//    p.setPen( Qt::NoPen );
//    p.setBrush( lg );
//    //p.drawRect( r.adjusted( 0, pTitleWidget->height()/2, 0, 0 ) );
//    p.setClipping( false );
//
//    int topFrameOffset = 0;//pTitleWidget->height();// - 2;
//    QPoint pnt1 = r.topLeft() + QPoint( 0, topFrameOffset );
//    QPoint pnt2 = r.topRight() - QPoint( topRight.width()-1, 0 ) + QPoint( 0, topFrameOffset );
//    QPoint pnt3 = r.bottomLeft() - QPoint( 0, bottomLeft.height()-1 );
//    QPoint pnt4 = r.bottomRight() - QPoint( bottomRight.width()-1, bottomRight.height()-1 );
//    p.drawPixmap( r.topLeft() + QPoint( 0, topFrameOffset ), topLeft );
//    p.drawPixmap( r.topRight() - QPoint( topRight.width()-1, 0 )
//                        + QPoint( 0, topFrameOffset ), topRight );
//    p.drawPixmap( r.bottomLeft() - QPoint( 0, bottomLeft.height()-1 ), bottomLeft );
//    p.drawPixmap( r.bottomRight() - QPoint( bottomRight.width()-1,
//                        bottomRight.height()-1 ), bottomRight );
//    
//    QRect left = r;
//    left.setY( r.y() + topLeft.height() + topFrameOffset );
//    left.setWidth( leftStretch.width() );
//    left.setHeight( r.height() - topLeft.height() - bottomLeft.height() - topFrameOffset );
//    p.drawTiledPixmap( left, leftStretch );
//
//    QRect top = r;
//    top.setX( r.x() + topLeft.width() );
//    top.setY( r.y() + topFrameOffset );
//    top.setWidth( r.width() - topLeft.width() - topRight.width() );
//    top.setHeight( topLeft.height() );
//    p.drawTiledPixmap( top, topStretch );
//
//    QRect right = r;
//    right.setX( r.right() - rightStretch.width()+1 );
//    right.setY( r.y() + topRight.height() + topFrameOffset );
//    right.setWidth( rightStretch.width() );
//    right.setHeight( r.height() - topRight.height()
//                    - bottomRight.height() - topFrameOffset );
//    p.drawTiledPixmap( right, rightStretch );
//
//    QRect bottom = r;
//    bottom.setX( r.x() + bottomLeft.width() );
//    bottom.setY( r.bottom() - bottomStretch.height()+1 );
//    bottom.setWidth( r.width() - bottomLeft.width() - bottomRight.width() );
//    bottom.setHeight( bottomLeft.height() );
//    p.drawTiledPixmap( bottom, bottomStretch );
//    p.restore();
//}
