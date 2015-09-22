/*******************************************************************************
**
** file: ewa_siteusagedelegate.cpp
**
** class: EWASiteUsageDelegate
**
** description:
** Delegate for used/unused checkbox
**
** 23.03.2009
**
** sendevent@gmail.com
**
*******************************************************************************/

#include "ewa_siteusagedelegate.h"
#include "ewa_sitehandle.h"

EWASiteUsageDelegate::EWASiteUsageDelegate( QObject *parent )
:EWASiteDelegateBase( parent )
{
    m_pBoxImg = QImage( QLatin1String( ":/images/checkbox_rect.png" ) );
    m_pMarkImg = QImage( QLatin1String( ":/images/checkbox_mark.png" ) );
}

EWASiteUsageDelegate::~EWASiteUsageDelegate()
{
}

QSize EWASiteUsageDelegate::sizeHint( const QStyleOptionViewItem &option,
const QModelIndex &index ) const
{
    Q_UNUSED( option );
    if( handleFromIndex( index ) )
    {
        return QSize( 16, 16 );
    }
    return QSize();
}

void EWASiteUsageDelegate::paint( QPainter *painter, const QStyleOptionViewItem &option,
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
        drawSiteIsUsedCheckbox( painter, option, sitePtr );

        painter->restore();
    }
}

void EWASiteUsageDelegate::drawSiteIsUsedCheckbox( QPainter *painter,
const QStyleOptionViewItem &option, EWASiteHandle* pSite ) const
{
    if( !pSite )
        return;

    drawBox( painter, option, pSite );

    if( pSite->isUsed() )
        drawMark( painter, option, pSite );
}

void EWASiteUsageDelegate::drawBox( QPainter *painter,
const QStyleOptionViewItem &option, EWASiteHandle* pSite ) const
{
    if( !pSite )
        return;

    QRect r = option.rect;
    painter->drawImage( r, m_pBoxImg );
}

void EWASiteUsageDelegate::drawMark( QPainter *painter,
const QStyleOptionViewItem &option, EWASiteHandle* pSite ) const
{
    if( !pSite )
        return;

    if( pSite->isUsed() )
    {
        QRect r = option.rect;
        painter->drawImage( r, m_pMarkImg );
    }
}

 bool EWASiteUsageDelegate::editorEvent( QEvent *event, QAbstractItemModel *model, const QStyleOptionViewItem & option, const QModelIndex & index )
{
	if( event->type() == QEvent::MouseButtonPress )
	{
	    QMouseEvent *pMouseEvent = static_cast<QMouseEvent *>( event );
        if( pMouseEvent && pMouseEvent->buttons() == Qt::LeftButton )
	    {
		    EWASiteHandle* pSite = handleFromIndex( index );
            if( pSite )
            {
			    bool used = pSite->isUsed();
			    pSite->setUsed( !used );
            }
	    }
        else if( pMouseEvent && pMouseEvent->buttons() == Qt::RightButton )
        {
            EWASiteDelegateBase::requestContextMenuForSite( pMouseEvent->globalPos(), index );
        }
	}

    return EWASiteDelegateBase::editorEvent( event, model, option, index );
}

QWidget* EWASiteUsageDelegate::createEditor( QWidget *parent,
const QStyleOptionViewItem& option, const QModelIndex& index ) const
{
    Q_UNUSED( parent );
    Q_UNUSED( option );
    Q_UNUSED( index );
    return 0;
}

void EWASiteUsageDelegate::updateEditorGeometry( QWidget *editor,
const QStyleOptionViewItem &option, const QModelIndex& ) const
{
    editor->setGeometry( option.rect );
}
