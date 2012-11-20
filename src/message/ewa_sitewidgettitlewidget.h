/*******************************************************************************
**
** file: ewa_sitewidgettitlewidget.h
**
** class: EWASiteWidgetTitleWidget
**
** description:
** Widget which handle space for EWASiteWidgetLabel and controll buttons.
** 01.07.2009
**
** ewauthor@indatray.com
**
*******************************************************************************/

#ifndef EWA_XMESSAGETITLEWIDGET_H
#define EWA_XMESSAGETITLEWIDGET_H

#include <QWidget>
#include <QLinearGradient>

#include "ui_ewa_sitewidgettitlewidget.h"

class EWATimer;
class EWASiteWidget;
class EWASiteWidgetTitleWidget : public QWidget
{
    Q_OBJECT
    
    public:
        EWASiteWidgetTitleWidget( QWidget *pParent = 0 );
        ~EWASiteWidgetTitleWidget();

        QToolButton *viewModeButton() const {return ui.viewModeButton;}
        QToolButton *stickButton() const {return ui.stickButton;}
        QToolButton *closeButton() const {return ui.closeButton;}
        QToolButton *backButton() const {return ui.backButton;}
        QToolButton *forwardButton() const {return ui.forwardButton;}
        QToolButton *refreshButton() const {return ui.refreshButton;}

        EWASiteWidgetLabel *label() const {return ui.label;}

        void setSite( EWASiteHandle *pSite );

        void setRefreshButtonShown( bool bShown )
        {
            m_bRefreshButtonShown = bShown;
        }
        bool isRefreshButtonShown() const
        {
            return m_bRefreshButtonShown;
        }
        void setBackButtonShown( bool bShown )
        {
            m_bBackButtonShown = bShown;
        }
        bool isBackButtonShown() const
        {
            return m_bBackButtonShown;
        }
        void setForwardButtonShown( bool bShown )
        {
            m_bForwardButtonShown = bShown;
        }
        bool isForwardButtonShown() const
        {
            return m_bForwardButtonShown;
        }
        void setTitleLabelShown( bool bShown )
        {
            m_bTitleLabelShown = bShown;
        }
        bool isTitleLabelShown() const
        {
            return m_bTitleLabelShown;
        }
        void setStickButtonShown( bool bShown )
        {
            m_bStickButtonShown = bShown;
        }
        bool isStickButtonShown() const
        {
            return m_bStickButtonShown;
        }
        void setViewModeButtonShown( bool bShown )
        {
            m_bViewModeButtonShown = bShown;
        }
        bool isViewModeButtonShown() const
        {
            return m_bViewModeButtonShown;
        }
        void setCloseButtonShown( bool bShown )
        {
            m_bCloseButtonShown = bShown;
        }
        bool isCloseButtonShown() const
        {
            return m_bCloseButtonShown;
        }

        void showControlls();
        void hideControlls();
        
        void setIsOnPreview( bool bOnPreview );
        
        bool isMenuVisible() const;
        
    protected:
        Ui::EWASiteWidgetTitleWidgetUi ui;
        
        QWidget *m_pParent;
        EWASiteHandle *m_sitePtr;
        QMenu *m_pWindowFlagsMenu;
        EWASiteWidget *m_pSiteWidget;
        
        QActionGroup *m_pActsGroup;
        QAction *m_pOnTopAction, 
            *m_pOnDesktopAction, 
            *m_pNeedEditAction, 
            *m_pAutoHidebleAction;
        
        bool m_bViewModeButtonShown
            ,m_bRefreshButtonShown
            ,m_bForwardButtonShown
            ,m_bCloseButtonShown
            ,m_bStickButtonShown
            ,m_bBackButtonShown
            ,m_bTitleLabelShown
            ,m_bMsgIsPreview;
        
        virtual void resizeEvent( QResizeEvent *event );
        virtual void changeEvent( QEvent *event );
        void updateGeomtery( const QRect& maxRect );
        
        bool isNotPreview();

        void retranslate();

    protected slots:
        void slotChangeTooltip( const QString& tooltip );
        void slotFaviconSiteChanged();
        void slotStickButtonActionTriggerd( QAction* act );
        
        void on_refreshButton_clicked();
        void on_forwardButton_clicked();
        void on_backButton_clicked();
        void on_viewModeButton_toggled( bool on );
        void on_stickButton_clicked();
        void on_closeButton_clicked();
        
    signals:
        void signalNeedGoBack();
        void signalNeedRefresh();
        void signalNeedGoForward();
        void signalNeedShowMaximized();
        void signalNeedShowNormal();
        void signalNeedAlwaysOnTop();
        void signalNeedAlwaysOnDesktop();
        void signalNeedAutoclose();
        void signalNeedShowEditor();
        void signalNeedClose();
        
    friend class EWASiteWidget;
};

#endif //-- EWA_XMESSAGETITLEWIDGET_H
