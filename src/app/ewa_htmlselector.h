/****************************************************************************
**
**
****************************************************************************/
#ifndef EWA_HTMLSELECTOR_H
#define EWA_HTMLSELECTOR_H

#include <QWidget>
#include <QPen>
#include <QWebElement>

class EWAWebView;
class EWASiteWidget;
class EWAHtmlSelectorDlg;
class EWAHTMLSelector : public QWidget
{
    Q_OBJECT
    
    public:
        EWAHTMLSelector( QWidget *pParent = 0 );
        virtual ~EWAHTMLSelector();
        
        void setBase( EWASiteWidget *pWebView );
        void initElementFromPath( const QList<int>& listEl, const QList<int>& listFr );
        QList<int> getElementsPath();
        QList<int> getElementsFramePath();
        
        QString getSelectedElementSource() const;
        
    protected:
        EWAHtmlSelectorDlg *m_pDlg;
        QWebElement m_element, m_elSelected;
        
        EWAWebView *m_webViewPtr;
        EWASiteWidget *m_messagePtr;
        
        QPen m_bkgndPen, m_markerPen;
        QList<int> m_elementsNumbersPath, m_framesNumbersPath;
        
        virtual void paintEvent( QPaintEvent *event );
        virtual void leaveEvent( QEvent *event );
        virtual void mouseMoveEvent( QMouseEvent *event );
        virtual void mouseReleaseEvent( QMouseEvent *event );
        virtual void mouseDoubleClickEvent( QMouseEvent *event );
        bool eventFilter(QObject *obj, QEvent *event);
        
        void drawElementsRects( const QRect& visible = QRect() );
        void updateDialogGeometry();
        
        QList<int> initPathFromElement( const QWebElement& element ) const;
        
    public slots:
        void show();
        
    signals:
        void signalHtmlElement( const QString& str );
        void signalAccepted();
        void signalReject();
        
    protected slots:
        void slotSiteWidgetScrolled( const QPoint& );
        void slotSiteWidgetGeometryChanged( const QRect& );
        void slotUpdateSelection();
};
#endif //-- EWA_HTMLSELECTOR_H
