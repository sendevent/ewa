#include "ewa_sitedrawer.h"
#include "ewa_sitehandle.h"
#include "ewa_sitedelegatebase.h"
#include <QToolTip>
#include <QLinearGradient>
#include <QRect>
#include <QColor>

/*static*/ void EWASiteDrawer::drawSiteStateInfo( EWASiteHandle *pSite, 
    QPainter *pPainter, const QRect& qrRect, bool bShowPercentsAndText, 
    bool bSelected, bool bUnderMouse )
{
    if( !pSite || !pPainter )
        return;

    pPainter->save();
    
    pPainter->setRenderHints( EWAApplication::getRenderHints() );
    QColor textColor = QColor( 0, 0, 127, 255 );
    QColor baseColor = Qt::white;
    if ( bSelected )
    {
        textColor = Qt::white;
        //-- <draw visible selection>
        baseColor = EWASiteDelegateBase::m_selectedColor;
        pPainter->fillRect( qrRect, baseColor );
        //-- </draw visible selection>
    }

    //-- <draw download progress>
    int dwnProgress = pSite->getDownloadProgress();
    if( dwnProgress && !pSite->getTypingProgress() )
    {
        QColor loadingColor( 227, 214, 28 );
        QRect dwnProgressRect = getSlicedRect( qrRect, dwnProgress );
        
        pPainter->setBrush( generateGradient( loadingColor, dwnProgressRect, baseColor ) );
        pPainter->setPen( Qt::transparent );
        pPainter->drawRoundedRect( dwnProgressRect, 4,4 );
        
        if ( bSelected )
        {
            textColor = QColor( 0, 0, 127, 255 );
        }
    }
    //-- </draw download progress>
    else
    {//-- <draw time progress>
        dwnProgress = pSite->getCountdownCurrent();
        double fullPeriod = pSite->getCountdownOriginal();
        if( !dwnProgress || dwnProgress == (int)fullPeriod || pSite->getTypingProgress() )
        {
            dwnProgress = pSite->getTypingProgress();
            fullPeriod = 100;
        }
        
        if( dwnProgress && fullPeriod )
        {
            double fullPeriodPercent = fullPeriod/100;
            int progress = dwnProgress;
            progress = ( int )( dwnProgress/fullPeriodPercent );

            QColor loadingColor( 200, 200, 200 );
            QRect dwnProgressRect = getSlicedRect( qrRect, progress );
            
            pPainter->setBrush( generateGradient( loadingColor, dwnProgressRect, baseColor ) );
            pPainter->setPen( Qt::transparent );
            pPainter->drawRoundedRect( dwnProgressRect, 4,4 );

            if ( bSelected )
            {
                textColor = QColor( 0, 0, 127, 255 );
            }
        }
        dwnProgress ^= dwnProgress;
    }//-- </draw time progress>

    if( bShowPercentsAndText )
    {
        //-- <draw url>
        QRect txtRect = qrRect;
        txtRect.adjust( 2, 1, -2, -1 );
    
        pPainter->setPen( textColor );
        if( bUnderMouse )
        {
            QFont font = pPainter->font();
            font.setBold( true );
            pPainter->setFont( font );
        }
        pPainter->drawText( txtRect, Qt::AlignVCenter|Qt::AlignLeft, pSite->getVisibleMsgTitle() );
        
        //-- </draw url>
    }
    
    if( dwnProgress && bShowPercentsAndText )
    {
        QFont paintersFont = pPainter->font();
        paintersFont.setBold( true );
        pPainter->setFont( paintersFont );
        
        QString strProgress = QString( "  %1%  " ).arg( dwnProgress );
        
        QRect percentsBackgroundRect = pPainter->fontMetrics().boundingRect( strProgress );
        percentsBackgroundRect.moveCenter( qrRect.center() );
        
        pPainter->setBrush( Qt::black );
        pPainter->setOpacity( 0.5 );
        pPainter->drawRoundedRect( percentsBackgroundRect, 4,4 );
        
        pPainter->setOpacity( 1.0 );
        pPainter->setPen( Qt::white );
        QRectF fRect( qrRect );

        fRect.setLeft( fRect.left() - 25 ); //-- white space on left
        fRect.setRight( fRect.right() + 25 ); //-- and right of the text

        pPainter->drawText( fRect, Qt::AlignCenter, strProgress, &fRect );
    }
    
    pPainter->restore();
}

/*static*/ QRect EWASiteDrawer::getSlicedRect( const QRect& optrect, const int& percent, 
const int iMinWidth )
{
    float mid = ( ( ( float )optrect.width() ) / 100 ) * ( float )percent;
    QRect res = QRect( optrect.x(), optrect.y(), ( int )mid, optrect.height() );
    
    if( res.width() < iMinWidth )
    {
        res.setWidth( iMinWidth );
    }

    return res;
}

/*static*/ QLinearGradient EWASiteDrawer::generateGradient( const QColor &color,
    const QRect& rect, const QColor& base )
{
    QPoint p1 = rect.topLeft();
    QPoint p2 = rect.bottomLeft();

    QLinearGradient gradient( p1, p2 );
    gradient.setColorAt( 0, base );
    gradient.setColorAt( 0.10, color.lighter( 120 ) );
    gradient.setColorAt( 0.5, color );
    gradient.setColorAt( 0.90, color.lighter( 120 ) );
    gradient.setColorAt( 1, base );
    gradient.setSpread( QGradient::ReflectSpread );
    return gradient;
}

void EWASiteDrawer::makeThumbnailFromImage( QPixmap& srcImg, qreal blurR )
{
    int iW = srcImg.width();
    int iH = srcImg.height();
    int iDstDemension = EWAApplication::getScreenSize().width()/5;
    
    if( qMax( iW, iH ) > iDstDemension )
    {
        srcImg = iW >= iH 
            ? srcImg.scaledToWidth( iDstDemension, Qt::SmoothTransformation )
            : srcImg.scaledToHeight( iDstDemension, Qt::SmoothTransformation );
    }
    
    QPixmap res( srcImg.width() + blurR + blurR, srcImg.height() + blurR + blurR);
    res.fill( QToolTip::palette().color( QPalette::Inactive, QPalette::AlternateBase ) );
    QPainter p( &res );
    p.setRenderHints( EWAApplication::getRenderHints() );
    
    drawShadowedPixmap( &p, srcImg, (res.width()-srcImg.width())/2-blurR/2, (res.height()-srcImg.height())/2-blurR/2, false );
    srcImg = res.copy();
}
const int fsize = 4;
static double filter[9][9] = 
{ 
    {1, 1, 1, 1, 1, 1, 1, 1, 1}
    ,{1, 1, 1, 1, 1, 1, 1, 1, 1}
    ,{1, 1, 1, 1, 1, 1, 1, 1, 1}
    ,{1, 1, 1, 1, 1, 1, 1, 1, 1}
    ,{1, 1, 1, 1, 1, 1, 1, 1, 1}
    ,{1, 1, 1, 1, 1, 1, 1, 1, 1}
    ,{1, 1, 1, 1, 1, 1, 1, 1, 1}
    ,{1, 1, 1, 1, 1, 1, 1, 1, 1}
    ,{1, 1, 1, 1, 1, 1, 1, 1, 1}
};                               



void EWASiteDrawer::drawShadowedPixmap(QPainter *p, const QPixmap &pix, int dx, int dy, bool onlyshdw )
{
  QImage msk = pix.toImage();
  QImage shadow(msk.width()+2*fsize, msk.height()+2*fsize, QImage::Format_ARGB32);
  double divisor=0.0;
  for(int i=0;i<2*fsize+1;i++)
    for(int j=0;j<2*fsize+1;j++)
      divisor+=filter[i][j];
  
  for(int y=0;y<shadow.height(); y++)
    for(int x=0;x<shadow.width(); x++){
      int l=0;
      for(int sy=-fsize;sy<=fsize;sy++)
        for(int sx=-fsize;sx<=fsize;sx++){
          int tx = x+sx-fsize;
          int ty = y+sy-fsize;
          if(tx<0 || ty<0 || tx>=msk.width() || ty>=msk.height())
            l+=(int)(filter[sx+fsize][sy+fsize]*255);
          else
            l+=(int)(filter[sx+fsize][sy+fsize]*(255-qAlpha(msk.pixel(tx, ty))));
        }
      l=(int)(l/divisor);
      l = 255 - l;
      l=qBound(0, l, 225);

      shadow.setPixel(x, y,qRgba(0, 0, 0, l));
    }
  
  p->drawImage(dx, dy, shadow);
  if(!onlyshdw)  p->drawPixmap(dx, dy, pix);
}