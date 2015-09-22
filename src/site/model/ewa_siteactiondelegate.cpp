/*******************************************************************************
**
** file: ewa_siteactiondelegate.cpp
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

#include "ewa_siteactiondelegate.h"
#include "ewa_sitehandle.h"

EWASiteActionDelegate::EWASiteActionDelegate( QObject *parent )
:EWASiteDelegateBase( parent )
{
    m_refreshImgNormal = QImage( QLatin1String( ":/images/refresh_0.png" ) );

	m_refreshImgPressed = QImage( QLatin1String( ":/images/refresh_1.png" ) );
}

EWASiteActionDelegate::~EWASiteActionDelegate()
{
}

QSize EWASiteActionDelegate::sizeHint( const QStyleOptionViewItem &option,
const QModelIndex &index ) const
{
    Q_UNUSED( option );
    if( handleFromIndex( index ) )
    {
        return QSize( 16, 16 );
    }
    return QSize();
}

void EWASiteActionDelegate::paint( QPainter *painter, const QStyleOptionViewItem &option,
                         const QModelIndex &index ) const
{
    EWASiteHandle* sitePtr = handleFromIndex( index );
    if( sitePtr )
    {
		painter->save();

		if ( option.state & QStyle::State_Selected )
		{
			painter->fillRect( option.rect, EWASiteDelegateBase::m_selectedColor );
		}

		if( sitePtr->isDownloadingActive() )
		    painter->drawImage( option.rect, m_refreshImgPressed );
		else
		    painter->drawImage( option.rect, m_refreshImgNormal );

		painter->restore();
    }
}


QWidget* EWASiteActionDelegate::createEditor( QWidget *parent,
const QStyleOptionViewItem& option, const QModelIndex& index ) const
{
    Q_UNUSED( parent );
    Q_UNUSED( option );
    Q_UNUSED( index );
    return 0;
}

bool EWASiteActionDelegate::editorEvent( QEvent *event, QAbstractItemModel *model,
const QStyleOptionViewItem &option, const QModelIndex &index )
{
	if( event->type() == QEvent::MouseButtonPress )
	{
	    QMouseEvent *pMouseEvent = static_cast<QMouseEvent *>( event );
	    if( pMouseEvent && pMouseEvent->buttons() == Qt::LeftButton )
	    {
	        EWASiteHandle* sitePtr = handleFromIndex( index );
            if( sitePtr )
            {
                sitePtr->slotActionDelegatActivated();
            }
        }
        else if( pMouseEvent && pMouseEvent->buttons() == Qt::RightButton )
        {
            EWASiteDelegateBase::requestContextMenuForSite( pMouseEvent->globalPos(), index );
        }
    }

    return QStyledItemDelegate::editorEvent( event, model, option, index );
}

void EWASiteActionDelegate::updateEditorGeometry( QWidget *editor,
const QStyleOptionViewItem &option, const QModelIndex& ) const
{
    editor->setGeometry( option.rect );
}
