/*******************************************************************************
**
** file: ewa_trayiconmanager.cpp
**
** class: EWATrayIconManager
**
** description:
** This class is handle for system tray entry. It manage icon image ( set in main window )
** and user interaction - mouse activyties.
**
** 09.02.2009
**
** sendevent@gmail.com
**
*******************************************************************************/

#include "ewa_trayiconmanager.h"

#include "ewa_application.h"
#include "ewa_siteslistmanager.h"
#include "ewa_mainwindow.h"

#include <QMenu>
#include <QAction>
#include <QWidgetAction>

EWATrayIconManager::EWATrayIconManager( QObject *pParent )
:QObject( pParent )
{
    construct();
}

EWATrayIconManager::~EWATrayIconManager()
{
    delete m_pTrayIconMenu;
    m_pTrayIconMenu = 0;

    m_pTrayIcon->hide();
    delete m_pTrayIcon;
    m_pTrayIcon = 0;

}

void EWATrayIconManager::construct()
{
    m_msgText = m_msgTitle = QString();
    m_iLengthLimit = 256;
    m_bNoWordsWrap = true;
    m_bUseLimit = true;
    m_iMsgType = 0;
    m_iMsgTtl = 15;

    m_trayMsgIcon = ( QSystemTrayIcon::MessageIcon ) 1;

    m_pTrayIconMenu = new QMenu();

    connect( ( QObject* )EWAApplication::getSitesManager(), SIGNAL( signalContextMenuItemClicked(const QModelIndex&) ),
        this, SLOT( slotSiteInContextClicked(const QModelIndex&) ) );
    connect( ( QObject* )EWAApplication::getSitesManager(), SIGNAL( signalContextMenuItemDoubleClicked(const QModelIndex&) ),
        this, SLOT( slotSiteInContextDoubleClicked(const QModelIndex&) ) );

    m_pSitesContextAction = new QWidgetAction( m_pTrayIconMenu );
    m_pSitesContextAction->setDefaultWidget( EWAApplication::getSitesManager()->getMenuTableView() );

    m_pCloseMenuAction = m_pTrayIconMenu->addAction( QIcon( ":/images/close.png" ),
    tr( "Close this menu" ), this, SLOT( slotCloseContextMenu() ) );
    m_pTrayIconMenu->addSeparator();
    m_pTrayIconMenu->addAction( m_pSitesContextAction );

    m_pTrayIcon = new QSystemTrayIcon( this );
    m_pTrayIcon->setContextMenu( m_pTrayIconMenu );

    connect( m_pTrayIcon, SIGNAL( messageClicked() ),
        this, SLOT( slotTrayMessageClicked() ) );

    connect( m_pTrayIcon, SIGNAL( activated(QSystemTrayIcon::ActivationReason) ),
        this, SLOT( slotIconActivated(QSystemTrayIcon::ActivationReason) ) );

    setIcon( QIcon( QLatin1String( ":/images/ewa.png" ) ) );
    m_pTrayIcon->setVisible( true );
}

void EWATrayIconManager::translateCloseMenuAction()
{
    if( m_pCloseMenuAction )
    {
        m_pCloseMenuAction->setText( tr( "Close this menu" ) );
    }
}

void EWATrayIconManager::slotSiteInContextClicked( const QModelIndex& index )
{
    if( index.column() != 3 )
    {
        slotCloseContextMenu();
    }
}

void EWATrayIconManager::slotSiteInContextDoubleClicked( const QModelIndex& )
{
    slotCloseContextMenu();
}

void EWATrayIconManager::slotTrayMessageClicked()
{
    showNextMsgPart();
}
void EWATrayIconManager::slotIconActivated( QSystemTrayIcon::ActivationReason reason )
{
    switch( reason )
    {
        case QSystemTrayIcon::DoubleClick:
        case QSystemTrayIcon::Trigger:
        {
           if( EWAApplication::getMainWindow()->isHidden() )
            {
                EWAApplication::getMainWindow()->setFocus();
                EWAApplication::getMainWindow()->showNormal();
                EWAApplication::getMainWindow()->activateWindow();
            }
            else
                EWAApplication::getMainWindow()->slotProcessMinimizationRequest();
            break;
        }
        case QSystemTrayIcon::MiddleClick:
        {
            EWAApplication::instance()->quit();
            break;
        }
        case QSystemTrayIcon::Unknown:
        {
            qWarning( "ffuck, what have you done?!1" );
            break;
        }
        case QSystemTrayIcon::Context:
        {
			break;
        }
    }
}

void EWATrayIconManager::showMessageBalloon( const QString& title, const QString& text )
{
    m_pTrayIcon->showMessage( title, text, m_trayMsgIcon, m_iMsgTtl*1000 ) ;
}

void EWATrayIconManager::setIcon( const QIcon& icon )
{
    getTrayIcon()->setIcon( icon );
}

void EWATrayIconManager::setToolTip( const QString& toolTip )
{
    getTrayIcon()->setToolTip( toolTip );
}

void EWATrayIconManager::setMessage( const QString& title, const QString txt, int iLengthLimit,
        bool bNoWordsWrap, bool bUseLimit, int iMsgType, int iMsgTtl )
{
    m_msgTitle = title;
    m_msgText = txt;

    m_iLengthLimit = iLengthLimit;
    m_bNoWordsWrap = bNoWordsWrap;
    m_bUseLimit = bUseLimit;

    m_iMsgType = iMsgType;
    m_trayMsgIcon = ( QSystemTrayIcon::MessageIcon ) m_iMsgType;
    m_iMsgTtl = iMsgTtl;

    showNextMsgPart();
}

void EWATrayIconManager::showNextMsgPart()
{
    if( m_msgText.isEmpty() )
    {
        return;
    }

    QString msgPart;
	if( m_bUseLimit && m_msgText.length() > m_iLengthLimit )
	{
        msgPart = m_msgText.left( m_iLengthLimit );
        long int spaceId = msgPart.lastIndexOf( " " );
		if( m_bNoWordsWrap
		    && spaceId > 0
		    && spaceId < m_iLengthLimit )
		{
			msgPart = msgPart.left( spaceId+1 );
		}

		m_msgText = m_msgText.right( m_msgText.length() - msgPart.length() );
	}
	else
	{//-- isn't need to be limited or current msg body is less then limit length:
        msgPart = m_msgText;

		m_msgText.clear();
	}

	if( !msgPart.isEmpty() )
    {
        showMessageBalloon( m_msgTitle, msgPart );
    }
}

void EWATrayIconManager::slotCloseContextMenu()
{
    m_pTrayIconMenu->close();
}

bool EWATrayIconManager::isAvailable() const
{
    return QSystemTrayIcon::isSystemTrayAvailable();
}

bool EWATrayIconManager::supportBaloons() const
{
    return QSystemTrayIcon::supportsMessages();
}
