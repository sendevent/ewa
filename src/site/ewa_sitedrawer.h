/*******************************************************************************
**
** file: ewa_sitedrawer.h
**
** class: EWASiteDrawer
**
** description:
** Routine for drawing site's information:
** progresses, urls, labels, etc
**
** 19.05.2010
**
** sendevent@gmail.com
**
*******************************************************************************/

#ifndef EWA_SITEDRAWER_H
#define EWA_SITEDRAWER_H

#include <QtGlobal>

class EWASiteHandle;
class QRect;
class QColor;
class QLinearGradient;
class QPainter;
class QPixmap;
class EWASiteDrawer 
{    
    public:
        static QRect getSlicedRect( const QRect& optrect, const int& percent = 100, const int iMinWidth = 5 );
        static QLinearGradient generateGradient( const QColor &color, const QRect& rect, const QColor& base );
        
        static void drawSiteStateInfo( EWASiteHandle *pSite, QPainter *pPainter, const QRect& qrRect, 
            bool bShowPercentsAndText = true, bool bSelected = false, bool bUnderMouse = false );
        
        static void makeThumbnailFromImage( QPixmap& img, qreal blurR = 8 );
        
        static void drawShadowedPixmap(QPainter *p, const QPixmap &pix, int dx, int dy, bool onlyshdw=false);
};

#endif //-- EWA_SITEDRAWER_H