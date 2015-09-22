/*******************************************************************************
**
** file: ewa_webview.h
**
** class: EWAWebView
**
** description:
** Inherits QWebKit::QWebView, customizeble view for web documents
** with internal API implementations
**
** 10.02.2009
**
** sendevent@gmail.com
**
*******************************************************************************/


#ifndef EWA_WEBVIEW_H
#define EWA_WEBVIEW_H

#include <QWebView>
#include <QWebElement>
#include <QWebHitTestResult>

#include "ewa_webpage.h"
#include "ewa_useraction.h"

class EWASiteHandle;
class EWAUserActionsRecorder;
class EWAUserActionsPlayer;
class EWAUserActionsCollection;
class EWANetworkAccessManager;
class EWADownloadDlg;
class EWASiteWidget;
class QMenu;
class QActionGroup;
class QShortcut;
class EWAWebView : public QWebView
{
    Q_OBJECT
    
    public:
        EWAWebView( QWidget *parent = 0, const QSize& maxSz = QSize() );
        ~EWAWebView();

        int getMaxWidth() const {return m_iMaxWidth;}
        int getMaxHeight() const {return m_iMaxGeight;}

        void setPage( QWebPage *pPage );
        void setUrl( const QString& url );
        void setHtml( const QString & qstrHtml, const QUrl & quBaseUrl = QUrl(),
            bool bSilentlyOnce = false, QWebFrame *pFrame = 0 );
        void setContent(const QByteArray& data, const QString& mimeType = QString(), const QUrl& baseUrl = QUrl());
        void setJSPagesActions( QVector<EWAUserActionsCollection*>* pPages );

        EWAWebPage *page() const { return m_pWebPage; }

        /**
        ** Reset content - stop downloads,
        ** remove content and js.
        ** (Like Web-browser's tab closed)
        */
        void clear();
        
        bool startRecording();
        bool startReplaying();
        
        void triggerPageAction( QWebPage::WebAction action, bool checked = false );
        
        void setSite( EWASiteHandle *pSite );
        
        void setEWANetworkManagerPtr( EWANetworkAccessManager *pManager );
        const EWANetworkAccessManager *getNetworkManager() const {return m_netAccessManagerPtr;}
        
        EWAWebPage *getWebPage() const {return m_pWebPage;}

        void recordStarted();
        void recordFinished();
        
        bool isMenuVisible() const;
        
        void showDownloader( QNetworkReply *pReply );
        
        bool isBlankPage() const;
        
        QList<int> getPathFromWebElement( const QWebElement& element ) const;
        QList<int> getPathFromWebElement( const QPoint& click ) const;
        QWebElement getWebElementFromPath( const QList<int>& path2Frame, const QList<int>& path2Element ) const;
        
        QList<int> getPath2ElementsFrame( const QWebElement& element ) const;
        int getFrameIndex( const QWebFrame *pParent, const QWebFrame *pFrame ) const;
        QWebFrame* getFrameFromPath( const QList<int> list ) const;
        QWebFrame* getChildFrame( const QWebFrame *pParent, int id ) const;
        
    protected:
        EWAWebPage *m_pWebPage;
        QMenu *m_pContextMenu;
        EWADownloadDlg *m_pDownloadDisplay;
        EWASiteHandle *m_sitePtr;
        EWANetworkAccessManager *m_netAccessManagerPtr;
        EWAUserActionsRecorder *m_pUsrActRecorder;
        EWAUserActionsPlayer *m_pUsrActPlayer;
        QWebHitTestResult m_hitTestRes;
        EWASiteWidget *m_siteWidgetPtr;
        
        int m_iMaxWidth, m_iMaxGeight;
        
        bool m_bInResizeEvent;
        
        QShortcut *m_pGoBackShct
                    ,*m_pGoFwdShct
                    ,*m_pReloadShct
                    ,*m_pSelectAllShct
                    ,*m_pCutShct
                    ,*m_pPasteShct
                    ,*m_pCopyShct
                    ,*m_pUndoShct
                    ,*m_pRedoShct;
        
        QActionGroup *m_pNavActionsGroup
            ,*m_pEditActionsGroup
            ,*m_pLinkActionsGroup
            ,*m_pImageActionsGroup;
        QAction *m_pOpenInBrowserAction;
        QPoint m_ptPrevScroll
            ,m_contextMenuPnt;
        
        bool m_bIsRecording
            ,m_bLinkStatMsgShown;
        
        void initContextMenu();
        void initNavigationActions();
        void initEditActions();
        void initLinkActions();
        void initImageActions();
        
        //-- web page's action now can't be triggered by shortcuts
        //-- on whole page, except editable fields.
        //-- correct it:
        void initNavigationActionsShortcuts();
        void initEditActionsShortcuts();
        QShortcut* createShrtcut( const QKeySequence& keySeq, const QWebPage::WebAction& act );
        
        virtual void paintEvent( QPaintEvent *event );
        virtual void resizeEvent( QResizeEvent *event );
        virtual void keyPressEvent( QKeyEvent *event );
        virtual void keyReleaseEvent( QKeyEvent *event );
        virtual void mouseMoveEvent( QMouseEvent *event );
        virtual void mousePressEvent( QMouseEvent *event );
        virtual void mouseReleaseEvent( QMouseEvent *event );
        virtual void contextMenuEvent( QContextMenuEvent *event );
        
        void prepareContextMenu();
        
        void setActionGroupDisabled( QActionGroup *pGroup, bool bDisabled = true ) const;
        QWebElement getByNumberChildOf( const QWebElement& parent, const int& num ) const;

        QString saveImage( const QPoint& pnt );        
        
        QString getHyperLink( const QPoint& pnt, bool& bIsExternal, bool bIsImg = false ) const;
        
        bool canAndNeedResizeTo( const QSize& sz ) const;
        
        QString getFileNameFromUrl( const QString& strFullUrl ) const;
        
        int getNumberUnderParent( const QWebElement& element ) const;
        
        QShortcut* getShorcutForAction( const QWebPage::WebAction& webAct ) const;
        
    protected slots:
        void slotLoadFinished( bool ok );
        void slotLoadStarted();
        
        void slotSaveImage();
        void slotSaveLink();
        
        void slotAdjustSize( const QSize& sz = QSize() );
        bool tryOpenLinkInBrowser( const QPoint& pnt = QPoint() ) const;
        bool tryOpenLinkHere( const QPoint& pnt = QPoint() );
        bool tryGetImageSource() const;
        
        void slotCopyLinkText2Clipboard();
        
        void updateDownloaderDlgGeometry();
        
        void slotWebPageActionShortcutActivatedAmbiguously();
        void slotUpdateWebActShrtctState() const;
        
    signals:
        void signalStatusMessage( const QString& msg, int iPeriod );
};

#endif //-- EWA_WEBVIEW_H
