/*******************************************************************************
**
** file: ewa_siteslistmodeldelegate.cpp
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

#include "ewa_siteslistmodeldelegate.h"
#include "ewa_sitehandle.h"
#include "ewa_sitedrawer.h"

EWASitesListModelDelegate::EWASitesListModelDelegate( QObject *parent )
:EWASiteDelegateBase( parent )
{
    m_defaultBaseColor = Qt::white;
    m_loadingColor = QColor( 245, 158, 0 );
}
void EWASitesListModelDelegate::paint( QPainter *painter, const QStyleOptionViewItem &option,
                         const QModelIndex &index ) const
{
    EWASiteHandle* sitePtr = handleFromIndex( index );
    if( !sitePtr )
        return;

    painter->save();
    bool bSelected = (option.state & QStyle::State_Selected );
    bool bUnderMouse = false;
    if( index.isValid() )
    {
        const EWASitesListModel *pModel = qobject_cast<const EWASitesListModel*>( index.model() );
        if( pModel )
        {
            bUnderMouse = index.row() == pModel->getRowUnderMouse();
        }
    }
    
    EWASiteDrawer::drawSiteStateInfo( sitePtr, painter, option.rect, true, bSelected, bUnderMouse );
    
    painter->restore();
}

void EWASitesListModelDelegate::updateEditorGeometry( QWidget *editor,
const QStyleOptionViewItem &option, const QModelIndex &index ) const
{
    EWASiteHandle* sitePtr = handleFromIndex( index );
    if( sitePtr )
    {
        QRect r = option.rect;
        r.setWidth( option.fontMetrics.width( sitePtr->getVisibleMsgTitle() ) );
        editor->setGeometry( r );
    }
}

QWidget* EWASitesListModelDelegate::createEditor( QWidget *parent,
const QStyleOptionViewItem & option, const QModelIndex & index ) const
{
    Q_UNUSED( option );
	EWASiteHandle* sitePtr = handleFromIndex( index );
    if( sitePtr )
    {
        QLabel *label = new QLabel( sitePtr->getVisibleMsgTitle(), parent );
        return label;
    }

	return 0;
}

bool EWASitesListModelDelegate::editorEvent( QEvent *event,
    QAbstractItemModel *model, const QStyleOptionViewItem &option, const QModelIndex &index )
{
    if( event->type() == QEvent::MouseButtonPress )
	{
        QMouseEvent *pMouseEvent = static_cast<QMouseEvent *>( event );
        if( pMouseEvent && pMouseEvent->buttons() == Qt::RightButton )
        {
            EWASiteDelegateBase::requestContextMenuForSite( pMouseEvent->globalPos(), index );
        }
    }

    return QStyledItemDelegate::editorEvent( event, model, option, index );
}
