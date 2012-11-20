#ifndef EWA_SITEDELEGATEBASE_H
#define EWA_SITEDELEGATEBASE_H

#include <QtGui>

#include "ewa_siteslistmodel.h"
#include "ewa_siteslistmanager.h"
#include "ewa_webpage.h"

class EWASiteDelegateBase : public QStyledItemDelegate
{
    public:
        EWASiteDelegateBase( QObject *parent = 0 );
        virtual ~EWASiteDelegateBase();
        static QColor m_selectedColor;
    protected:
        EWASiteHandle* handleFromIndex( const QModelIndex& index ) const;

        void requestContextMenuForSite( const QPoint& pnt, const QModelIndex &index );
};

#endif //--EWA_SITEDELEGATEBASE_H
