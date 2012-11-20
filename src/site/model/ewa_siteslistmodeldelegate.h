/*******************************************************************************
**
** file: ewa_siteslistmodeldelegate.h
**
** class: EWASitesListModelDelegate
**
** description:
** Delegate for displaying site's progresses in table view.
**
** 23.03.2009
**
** ewauthor@indatray.com
**
*******************************************************************************/

#ifndef EWA_SITESLISTMODELDELEGATE_H
#define EWA_SITESLISTMODELDELEGATE_H

#include "ewa_sitedelegatebase.h"

class EWASitesListModelDelegate : public EWASiteDelegateBase
{
    public:
        EWASitesListModelDelegate( QObject *parent = 0 );
        void paint( QPainter *painter, const QStyleOptionViewItem &option,
                   const QModelIndex &index ) const;
		virtual QWidget* createEditor( QWidget *parent, const QStyleOptionViewItem & option, const QModelIndex & index ) const;

    protected:
        QColor m_defaultBaseColor;
        QColor m_loadingColor;

        virtual void updateEditorGeometry( QWidget *editor, const QStyleOptionViewItem &option,
        const QModelIndex &index ) const;
        virtual bool editorEvent( QEvent *event, QAbstractItemModel *model, const QStyleOptionViewItem & option, const QModelIndex & index );
};

#endif //-- EWA_SITESLISTMODELDELEGATE_H
