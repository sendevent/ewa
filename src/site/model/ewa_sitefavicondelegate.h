/*******************************************************************************
**
** file: ewa_sitefavicondelegate.h
**
** class: EWASiteFaviconDelegate
**
** description:
** Delegate for displaying site's favicon in sites table ( list ) view.
**
** 09.02.2009
**
** ewauthor@indatray.com
**
*******************************************************************************/

#ifndef EWA_SITEFAVICONDELDELEGATE_H
#define EWA_SITEFAVICONDELDELEGATE_H

#include "ewa_sitedelegatebase.h"

class EWASiteFaviconDelegate : public EWASiteDelegateBase
{
    public:
        EWASiteFaviconDelegate( QObject *parent = 0 );
        void paint( QPainter *painter, const QStyleOptionViewItem &option,
                   const QModelIndex &index ) const;
        virtual QSize sizeHint( const QStyleOptionViewItem & option, const QModelIndex & index ) const;

    protected:
        void drawSiteFavicon( QPainter *painter, const QStyleOptionViewItem &option, EWASiteHandle* pSite ) const;
};

#endif //-- EWA_SITEFAVICONDELDELEGATE_H
