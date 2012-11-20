/*******************************************************************************
**
** file: ewa_siteusagedelegate.h
**
** class: EWASiteUsageDelegate
**
** description:
** Delegate for used/unused checkbox
**
** 23.03.2009
**
** ewauthor@indatray.com
**
*******************************************************************************/


#ifndef EWA_SITEUSAGEDELEGATE_H
#define EWA_SITEUSAGEDELEGATE_H

#include "ewa_sitedelegatebase.h"

class EWASiteUsageDelegate : public EWASiteDelegateBase
{
    public:
        EWASiteUsageDelegate( QObject *parent = 0 );
        ~EWASiteUsageDelegate();

        void paint( QPainter *painter, const QStyleOptionViewItem &option,
                   const QModelIndex &index ) const;

        QSize sizeHint( const QStyleOptionViewItem & option, const QModelIndex & index ) const;

        virtual QWidget* createEditor( QWidget *parent, const QStyleOptionViewItem & option, const QModelIndex & index ) const;
        virtual bool editorEvent( QEvent *event, QAbstractItemModel *model, const QStyleOptionViewItem & option, const QModelIndex & index );
        virtual void updateEditorGeometry( QWidget *editor, const QStyleOptionViewItem &option, const QModelIndex & index ) const;

    protected:
        mutable QImage m_pBoxImg, m_pMarkImg;

        void drawSiteIsUsedCheckbox( QPainter *painter, const QStyleOptionViewItem &option, EWASiteHandle* pSite ) const;
        void drawBox( QPainter *painter, const QStyleOptionViewItem &option, EWASiteHandle* pSite ) const;
        void drawMark( QPainter *painter, const QStyleOptionViewItem &option, EWASiteHandle* pSite ) const;
};

#endif //-- EWA_SITEUSAGEDELEGATE_H
