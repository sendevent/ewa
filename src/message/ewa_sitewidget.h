/*******************************************************************************
**
** file: ewa_sitewidget.h
**
** class: EWASiteWidget
**
** description:
** Internal popup message - support sliding, customizible tittle, auto hide,
** "always on top/bottom" modes.
** Custom "close" - through nice transparency :-[
** Create/destroy in viewmanager.
** Contains WebView, used in sitehandle and other.
**
** 23.03.2009
**
** sendevent@gmail.com
**
*******************************************************************************/

#ifndef EWA_SYSTRAYMESSAGE_H
#define EWA_SYSTRAYMESSAGE_H

#include <QWidget>

#include <QColor>
#include <QPen>
#include <QRect>
#include <QSize>
#include <QPoint>
#include <QDebug>
#include <QScrollArea>

class QSpinBox;
class QCheckBox;
class QRadioButton;
class QWidget;
class QGroupBox;
class QPushButton;
class QSizeGrip;
class QGridLayout;
class QNetworkReply;
class QScrollBar;

class EWAWebView;
class EWASiteHandle;
class EWANetworkAccessManager;
class EWADownloadDlg;
class EWASitePtr;
class EWATextLabel;
class EWASiteWidgetTitleWidget;
class EWATimer;

/*#ifdef EWA_DBG
    #define PLAY_WITH_RADIUSES
    #define PLAY_WITH_FLAGS
#endif //-- EWA_DBG*/

#ifdef PLAY_WITH_RADIUSES

class RadiusSelectorDlg: public QWidget
{
    Q_OBJECT
    public:
        RadiusSelectorDlg();
        ~RadiusSelectorDlg();
        QSpinBox *m_pXSB, *m_pYSB, *m_pFrameSB;

    signals:
        void signalXChanged( int x );
        void signalYChanged( int y );
        void signalWChanged( int y );
};

#endif //-- PLAY_WITH_RADIUSES

#ifdef PLAY_WITH_FLAGS
class FlagsManagerLialog : public QWidget
{
    Q_OBJECT

public:
    FlagsManagerLialog();
    void setTarget( QWidget *pWindow ) {previewWindow = pWindow;updateCheckboxes();}

public slots:
    void updatePreview();
    void updateCheckboxes( Qt::WindowFlags flags = 0 );

private:
    void createTypeGroupBox();
    void createHintsGroupBox();
    QCheckBox *createCheckBox(const QString &text);
    QRadioButton *createRadioButton(const QString &text);

    QWidget *previewWindow;

    QGroupBox *typeGroupBox;
    QGroupBox *hintsGroupBox;
    QPushButton *quitButton;

    QRadioButton *windowRadioButton;
    QRadioButton *dialogRadioButton;
    QRadioButton *sheetRadioButton;
    QRadioButton *drawerRadioButton;
    QRadioButton *popupRadioButton;
    QRadioButton *toolRadioButton;
    QRadioButton *toolTipRadioButton;
    QRadioButton *splashScreenRadioButton;

    QCheckBox *msWindowsFixedSizeDialogCheckBox;
    QCheckBox *x11BypassWindowManagerCheckBox;
    QCheckBox *framelessWindowCheckBox;
    QCheckBox *windowTitleCheckBox;
    QCheckBox *windowSystemMenuCheckBox;
    QCheckBox *windowMinimizeButtonCheckBox;
    QCheckBox *windowMaximizeButtonCheckBox;
    QCheckBox *windowCloseButtonCheckBox;
    QCheckBox *windowContextHelpButtonCheckBox;
    QCheckBox *windowShadeButtonCheckBox;
    QCheckBox *windowStaysOnTopCheckBox;
    QCheckBox *windowStaysOnBottomCheckBox;
    QCheckBox *customizeWindowHintCheckBox;
};

#endif //-- PLAY_WITH_FLAGS

class EWASiteWidget : public QWidget
{
    Q_OBJECT
    
    public:
        typedef enum EWA_EM_HideViewMode
        {
            EM_HVM_AUTOHIDE = 0
            ,EM_HVM_ALWAYS_ON_TOP = 1
            ,EM_HVM_ALWAYS_ON_DESKTOP = 2
        } EWA_EM_HVM;
        
        EWASiteWidget( QWidget *parent = 0 );
        virtual ~EWASiteWidget();
        
        void showSexy();
        
        bool closeIfAutoShown();
        bool closeIfOnTop();
        bool closeIfOnDesktop();
        
        bool closeByHideViewMode( EWA_EM_HVM mode );
        
        EWA_EM_HVM getCurrentMode() const {return eCurrentHideVewMode;}
        void loadCurrentMode( EWA_EM_HVM mode );

        QPoint getInverteddPos() const;
        
        virtual Qt::WindowFlags getFlagsNormal() const;
        virtual Qt::WindowFlags getFlagsMaximized() const;
        
        void cloneSettings( const EWASiteWidget *pOther );
        
        //-- getters/setters:
        void setColor( const QColor&  );
        QColor getColor() const {return m_color;}
        
        void setTitle( const QString& );
        QString getTitle() const;
        
        bool isAllwaysOnTop() const { return getCurrentMode() == EM_HVM_ALWAYS_ON_TOP; };
        
        void setTTL( const int ttl );
        int getTTL() const { return m_iTTLSecs; }
        
        void setNewSize( int w = -1, int h = -1 );

        void setBaseOpacity( const double transparency );
        double getBaseOpacity() const;
        
        QSize getVisibleWebViewSize() const;
        QRect getVisibleWebViewRect() const;
        
        void setAsPreview( bool preview );
        bool isPreview() const { return m_bImPreview; }

        bool isViewNormal() const;
        
        void setSite( EWASiteHandle *pSite );
        EWASiteHandle *getSite() const { return m_sitePtr; }

        void setLocation( const QPoint& pnt );
        QPoint getLocation() const {return m_locationPnt;}

        void setBaseScrollPoint( const QPoint& pnt );
        QPoint getBaseScrollPoint() const { return m_qpntBaseScoll; }
        QPoint getCurrnetScrollPoint() const {return m_ptCurrScroll;}

        void setBackButtonShown( bool bShown );
        bool isBackButtonShown() const;
        
        void setRefreshButtonShown( bool bShown );
        bool isRefreshButtonShown() const;
        
        void setForwardButtonShown( bool bShown );
        bool isForwardButtonShown() const;
        
        void setTitleLabelShown( bool bShown );
        bool isTitleLabelShown() const;
        
        void setStickButtonShown( bool bShown );
        bool isStickButtonShown() const;
        
        void setViewModeButtonShown( bool bShown );
        bool isViewModeButtonShown() const;
        
        void setCloseButtonShown( bool bShown );
        bool isCloseButtonShown() const;
        
        QIcon getSticButtonIcon() const;
        virtual void realClose();
        
        void setEWANetworkManagerPtr( EWANetworkAccessManager *pAccessManager );
        EWANetworkAccessManager* getEWANetworkManagerPtr() const { return m_accessManagerPtr;}
        
        EWAWebView* getWebView() const {return m_pWebView;}
        
        bool getWebViewTumbnail( QPixmap& image ) const;
        
        void showDownloader( QNetworkReply *pReply );

        virtual void setViewMaximized();
        virtual void setViewNormal();
        
        int getCustomizedVerticalOffset() const;
        
        void scroll2top();
        
        bool wasShown() const { return m_bWasShown; }
        
        void restoreLocation() { setLocation( m_locationPnt ); }
        
        void makeInvisible( bool bInvis = true );
        bool isInvisible() const;
        
        static int statusMessageDurationMs();

        QScrollArea *scrollArea() {return m_pScrollArea;}
        void setScrollArea( QScrollArea *pScrollArea );
        
        virtual void setWindowFlags( Qt::WindowFlags flags );
        
        QScrollBar* vScroll() {return scrollArea()->verticalScrollBar();}
        QScrollBar* hScroll() {return scrollArea()->horizontalScrollBar();}
        
        bool scrollsPolicyIs( Qt::ScrollBarPolicy policy ) const;
        void setScrollsPolicy( Qt::ScrollBarPolicy policy );

    protected: //-- propeties:
#ifdef PLAY_WITH_FLAGS
        FlagsManagerLialog *pFlagsDlg;
#endif //-- PLAY_WITH_FLAGS
        typedef enum EWA_EM_UserInteractionState
        {
            UIS_NONE,
            UIS_MOVE,
        } EWA_EM_UIState;
        EWA_EM_UIState m_eUIState; //-- current state (user|geometry interaction)
        EWASiteHandle *m_sitePtr; //-- site for show
        //-- normal/maximized;
        //-- maximized mode is allowed for non-preview;
        //-- in maximized mode move and resizing are desibled
        bool    m_bViewModeNormal
                ,m_bQuickShowScrolls //-- don't wait full period to view scrolls
                ,m_bImPreview
                ,m_bScrollsShown
                ,m_bWasShown;
                
        qreal m_dOpacityDelta;
        double m_dOriginalOpacity;
        
        EWANetworkAccessManager* m_accessManagerPtr;
        EWAWebView *m_pWebView;
        QColor m_color;
        QLinearGradient m_gradient;
        
        int m_iSecondsBeforClose, //-- current period to close
            m_iTTLSecs; //-- original period to close
        
        EWATimer *m_pCloserTimer, 
            *m_pDecOpacityTimer, 
            *m_pIncOpacityTimer,
            *m_pShowScrollsTimer, 
            *m_pHideScrollsTimer,
            *m_pStatusCleanTimer;
            
        QSizeGrip *m_pSzGrip;
        
        QColor grantsColor;
        QPen grantsPen;
        
        EWATextLabel *m_pStatusBar;
        
        QRect m_prevRect;
        QSize m_prevWebPageSz;
        QPoint m_prevScroll
            ,m_qpntBaseScoll
            ,m_locationPnt
            ,m_ptPrevPos
            ,m_ptCurrScroll;
        
        
        QGridLayout *m_pGridLayout;
        EWASiteWidgetTitleWidget *m_pTitleWidget;
        QScrollArea *m_pScrollArea;
        
        int m_iRX
            ,m_iRY;
        
        double m_dBorderWidth;
        
        EWA_EM_HVM eCurrentHideVewMode;
        
    protected: //-- members:
        //-- internal initializations:
        virtual void construct();
        void connectTitleWidgetButtons();
        
        void connectMe();
        void disconnectMe();
        void connectScrolls();
        void disconnectScrolls();
        void updateSizeGripGeometry();
        
        //-- events handlers overloading:
        virtual void mousePressEvent( QMouseEvent *event );
        virtual void mouseReleaseEvent( QMouseEvent *event );
        virtual void mouseMoveEvent( QMouseEvent *event );
        virtual void mouseDoubleClickEvent( QMouseEvent *event );
        virtual void enterEvent( QEvent *event );
        virtual void leaveEvent( QEvent *event );
        virtual void wheelEvent( QWheelEvent *event );
        virtual void resizeEvent( QResizeEvent *event );
        virtual void moveEvent( QMoveEvent *event );
        virtual void paintEvent( QPaintEvent *event );
        virtual void keyPressEvent( QKeyEvent *event );
        bool eventFilter( QObject *obj, QEvent *event );
        bool isOnAddWizard() const;
        
        
        
        void customProcessWheel( QWheelEvent *event );
        void stopOpacityChanges();
        bool correctWindowAttributes();

        bool isMovablePoint( const QPoint& globalPoint ) const;

        void stickToScreenEdges( const QPoint& ptMoveDelta, QRect &geom, int iPrecision = 16 ) const;
        
        void setTitleBarVisible( bool bVisible = true );
        
        void showScrollBars();
        void hideScrollBars();
        
        bool amIRaisable() const;
        
        void updateStatusBarGeometry();
        
        QSize calcPrefferedPageSize() const;

    public slots:
        virtual void show();
        bool close();
        void setWindowOpacity(qreal level);

        virtual void slotShowScrollBars();
        virtual void slotHideScrollBars();

        void slotNeedShowMaximized();
        void slotNeedShowNormal();
        
        void slotScrollTo( const QPoint& pnt );

    protected slots:
        virtual void slotSetViewAlwaysOnTop( bool bAndShow = true );
        virtual void slotSetViewAlwaysOnDesktop( bool bAndShow = true );
        virtual void slotSetViewAutoClose( bool bAndShow = true );
        void slotNeedGoBack();
        void slotNeedRefresh();
        void slotNeedGoForward();
        
        void slotNeedShowEditor();
        void slotNeedClose();
        
        void slotCloseButtonClicked();
        
        void slotSwitchOffQuickShowScrolls();
        void slotCloserTicked();
        void slotDecreaseOpacity();
        void slotIncreaseOpacity();
        void autoRestore();

#ifdef PLAY_WITH_RADIUSES
        void slotXChanged(int);
        void slotYChanged(int);
        void slotWChanged(int);
#endif //-- PLAY_WITH_RADIUSES        

        void slotStatusMessage( const QString& msg, int iPeriod = 0 );
        void slotClearStatus();
        void slotHorizontallScroll( int );
        void slotVerticalScroll( int );

    signals:
        void signalScrolled( const QPoint& scroll ) const;
        void signalGeometryChanged( const QRect& newGeometry );
        void signalXMsgColorChanged( const QColor& );
        void signalOpacityChanged(int value);
        
        void signaSiteWidgetModified();
        
    private:
        void initForUpdatesEnabled();
        friend class EWAUpdatesChecker;
};

#endif //-- EWA_SYSTRAYMESSAGE_H
