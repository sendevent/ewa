/*******************************************************************************
**
** file: ewa_sitefavicondelegate.cpp
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

#include "ewa_sitefavicondelegate.h"
#include "ewa_sitehandle.h"

EWASiteFaviconDelegate::EWASiteFaviconDelegate( QObject *parent )
:EWASiteDelegateBase( parent )
{
}

QSize EWASiteFaviconDelegate::sizeHint( const QStyleOptionViewItem & option,
const QModelIndex & index ) const
{
    EWASiteHandle* sitePtr = handleFromIndex( index );
    if( sitePtr )
    {
        return sitePtr->getFavicoPixmap().size();
    }
    return QStyledItemDelegate::sizeHint( option, index );
}

void EWASiteFaviconDelegate::paint( QPainter *painter, const QStyleOptionViewItem &option,
                         const QModelIndex &index ) const
{
    EWASiteHandle* sitePtr = handleFromIndex( index );
    if( sitePtr )
    {
        painter->save();

        drawSiteFavicon( painter, option, sitePtr );

        painter->restore();
    }
}

void EWASiteFaviconDelegate::drawSiteFavicon( QPainter *painter,
const QStyleOptionViewItem &option, EWASiteHandle* pSite ) const
{
    if( !pSite )
        return;

    if ( option.state & QStyle::State_Selected )
    {
        painter->fillRect( option.rect, EWASiteDelegateBase::m_selectedColor );
    }
    QPixmap pic = pSite->getFavicoPixmap();

    painter->save();
    painter->drawPixmap( option.rect, pic );
    
    painter->restore();
}
