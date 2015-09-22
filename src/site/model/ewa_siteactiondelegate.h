/*******************************************************************************
**
** file: ewa_siteactiondelegate.h
**
** class: EWASiteActionDelegate
**
** description:
** Delegate for "download" site's button.
**
** 09.02.2009
**
** sendevent@gmail.com
**
*******************************************************************************/

#ifndef EWA_SITEACTIONDELEGATE_H
#define EWA_SITEACTIONDELEGATE_H

#include "ewa_sitedelegatebase.h"

class EWASiteActionDelegate : public EWASiteDelegateBase
{
    Q_OBJECT

    public:
        EWASiteActionDelegate( QObject *parent = 0 );
        ~EWASiteActionDelegate();

        void paint( QPainter *painter, const QStyleOptionViewItem &option,
                   const QModelIndex &index ) const;

        QSize sizeHint( const QStyleOptionViewItem & option, const QModelIndex & index ) const;

        virtual QWidget* createEditor( QWidget *parent, const QStyleOptionViewItem & option, const QModelIndex & index ) const;
        virtual bool editorEvent( QEvent *event, QAbstractItemModel *model, const QStyleOptionViewItem & option, const QModelIndex & index );
        virtual void updateEditorGeometry( QWidget *editor, const QStyleOptionViewItem &option, const QModelIndex & index ) const;
        
    protected:
        QImage m_refreshImgNormal
            ,m_refreshImgPressed;
};

#endif //-- EWA_SITEACTIONDELEGATE_H
