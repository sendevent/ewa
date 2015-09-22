/*******************************************************************************
**
** file: ewa_trayiconmanager.h
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
#ifndef EWA_TRAYICONMANAGER_H
#define EWA_TRAYICONMANAGER_H

#include <QObject>
#include <QIcon>
#include <QSystemTrayIcon>
#include <QModelIndex>

class QMenu;
class QAction;
class QWidgetAction;
class QSystemTrayIcon;
class EWATrayIconManager : public QObject
{
    Q_OBJECT
    
    public:
        EWATrayIconManager( QObject *pParent = 0 );
        virtual ~EWATrayIconManager();
        
        bool isAvailable() const;
        bool supportBaloons() const;

        void setBaloonType( int type );
        void setMsgLengthLimitUsed( bool on );
        void setMsgLengthLimit( int amount );
        void setWordWrap( bool on );

        QMenu* getTrayContextMenu() const {return m_pTrayIconMenu; }
        QSystemTrayIcon* getTrayIcon() const {return m_pTrayIcon;}
        void hideTrayIcon() { m_pTrayIcon->hide(); };

        void setMessage( const QString& title, const QString txt, int iLengthLimit,
        bool bNoWordsWrap, bool bUseLimit, int iMsgType, int iMsgTtl );
        void showMessageBalloon( const QString& title, const QString& text );

        void setIcon( const QIcon& icon );
        void setToolTip( const QString& toolTip );

        void translateCloseMenuAction();

    protected:
        QSystemTrayIcon *m_pTrayIcon;
        QMenu *m_pTrayIconMenu;
        QAction *m_pCloseMenuAction;
        QWidgetAction *m_pSitesContextAction;

        QString m_msgText
            ,m_msgTitle;
            
        int m_iMsgType
            ,m_iMsgTtl;

        int m_iLengthLimit;
        bool m_bNoWordsWrap, m_bUseLimit;

        QSystemTrayIcon::MessageIcon m_trayMsgIcon;

        void construct();
        void showNextMsgPart();

    public slots:
        void slotCloseContextMenu();

    protected slots:
        void slotTrayMessageClicked();
        void slotIconActivated( QSystemTrayIcon::ActivationReason );

        void slotSiteInContextClicked( const QModelIndex& );
        void slotSiteInContextDoubleClicked( const QModelIndex& );
};

#endif //-- EWA_TRAYICONMANAGER_H
