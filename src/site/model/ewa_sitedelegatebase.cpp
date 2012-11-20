#include "ewa_sitedelegatebase.h"
#include "ewa_sitehandle.h"

/*static*/ QColor EWASiteDelegateBase::m_selectedColor = QColor( 0, 92, 162, 206 );
EWASiteDelegateBase::EWASiteDelegateBase( QObject *parent )
:QStyledItemDelegate( parent )
{
}

EWASiteDelegateBase::~EWASiteDelegateBase()
{
}

EWASiteHandle* EWASiteDelegateBase::handleFromIndex( const QModelIndex& index ) const
{
    if( index.isValid()
        && qVariantCanConvert<EWASitePtr>( index.data() ) )
    {
        return qVariantValue<EWASitePtr>( index.data() ).m_pSite;
    }

    return 0;
}

void EWASiteDelegateBase::requestContextMenuForSite( const QPoint& pnt, const QModelIndex &index )
{
    EWASiteHandle *pSite = handleFromIndex( index );
    if( pSite )
    {
        EWAApplication::getSitesManager()->showContextMenuForSite( pnt, pSite );
    }
}

