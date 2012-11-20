#include "ewa_htmlselector.h"
#include "ewa_htmlselectordlg.h"
#include "ewa_sitewidget.h"
#include "ewa_webview.h"

#include <QMouseEvent>
#include <QWebFrame>

EWAHTMLSelector::EWAHTMLSelector( QWidget *pParent )
:QWidget( pParent )
{
    m_pDlg = new EWAHtmlSelectorDlg( this );
    connect(  m_pDlg, SIGNAL( signalOk() ),
        this, SIGNAL ( signalAccepted() ) );
    connect(  m_pDlg, SIGNAL( signalCancel() ),
        this, SIGNAL ( signalReject() ) );

    m_webViewPtr = 0;
    m_messagePtr = 0;
    
    m_bkgndPen = QPen( Qt::gray );
    m_markerPen = QPen( Qt::yellow );
    
    setMouseTracking( true );
}

QList<int> EWAHTMLSelector::initPathFromElement( const QWebElement& element ) const
{
    return m_webViewPtr ? m_webViewPtr->getPathFromWebElement( element ) : QList<int>();
}


void EWAHTMLSelector::initElementFromPath( const QList<int>& listEl, const QList<int>& listFr )
{
    if( m_webViewPtr )
    {
        m_elementsNumbersPath = listEl;
        m_framesNumbersPath = listFr;
        m_elSelected = m_webViewPtr->getWebElementFromPath( m_framesNumbersPath, m_elementsNumbersPath );
    }
}
    
EWAHTMLSelector::~EWAHTMLSelector()
{
}

void EWAHTMLSelector::setBase( EWASiteWidget *pMsg )
{
    if( m_messagePtr )
    {
        disconnect( m_messagePtr, SIGNAL( signalScrolled(const QPoint&) ),
            this, SLOT( slotSiteWidgetScrolled(const QPoint&) ) );
        disconnect( m_messagePtr, SIGNAL( signalGeometryChanged(const QRect&) ),
            this, SLOT( slotSiteWidgetGeometryChanged(const QRect&) ) );
    }
    if( m_webViewPtr )
    {
        m_webViewPtr->removeEventFilter( this );
        disconnect( m_webViewPtr, SIGNAL(loadFinished(bool)),
            this, SLOT(slotUpdateSelection()));
    }
    
    if( pMsg )
    {
        m_messagePtr = pMsg;
        connect( m_messagePtr, SIGNAL( signalScrolled(const QPoint&) ),
            this, SLOT( slotSiteWidgetScrolled(const QPoint&) ) );
        connect( m_messagePtr, SIGNAL( signalGeometryChanged(const QRect&) ),
            this, SLOT( slotSiteWidgetGeometryChanged(const QRect&) ) );
    
        m_webViewPtr = m_messagePtr->getWebView();
        this->resize( m_webViewPtr->size() );
        m_webViewPtr->installEventFilter( this );
        connect( m_webViewPtr, SIGNAL(loadFinished(bool)),
            this, SLOT(slotUpdateSelection()));
        
        updateDialogGeometry();
    }
}

 void EWAHTMLSelector::paintEvent( QPaintEvent *event )
{
    drawElementsRects( event->rect() );
    QWidget::paintEvent( event );
}

void EWAHTMLSelector::drawElementsRects( const QRect& visible )
{
    QPainter p( this );
    if( !p.isActive() )
    {
        return;
    }
    
    QRect r = visible;
    if( r.isNull() )
    {
        r = rect();
    }
    
    p.setPen( m_bkgndPen );
    p.setBrush( m_bkgndPen.brush() );
    p.setOpacity(0.3);
    p.drawRect( r );
    
    if( !m_element.isNull() )
    {
        QRect elementUnderCursor = m_element.geometry();
        if( r.intersects( elementUnderCursor ) )
        {
            elementUnderCursor = elementUnderCursor.intersected( r );
            if( elementUnderCursor.isValid() )
            {
                QWebFrame *pFrame = m_element.webFrame();
                if( !pFrame )
                {
                    pFrame = m_webViewPtr->page()->mainFrame();
                }
                QPoint pnt = elementUnderCursor.center() - pFrame->scrollPosition();
                elementUnderCursor.moveCenter( pnt );
                p.setPen( m_markerPen );
                p.setBrush( m_markerPen.brush() );
                p.setOpacity(0.5);
                p.drawRoundedRect( elementUnderCursor, 14, 14 );
            }
        }
    }
    
    if( !m_elSelected.isNull() )
    {
        QRect selectedElement = m_elSelected.geometry();
        if( r.intersects( selectedElement ) )
        {
            selectedElement = selectedElement.intersected( r );
            if( selectedElement.isValid() )
            {
                QWebFrame *pFrame = m_elSelected.webFrame();
                if( !pFrame )
                {
                    pFrame = m_webViewPtr->page()->mainFrame();
                }
                QPoint pnt = selectedElement.center() - pFrame->scrollPosition();
                selectedElement.moveCenter( pnt );
                p.setPen( Qt::white );
                p.setBrush( Qt::white );
                p.setOpacity(0.7);
                p.drawRoundedRect( selectedElement, 14, 14 );
            }
        }
    }
}

void EWAHTMLSelector::mouseMoveEvent( QMouseEvent *event )
{
    if( !m_webViewPtr )
    {
        return;
    }
    
    QPoint pnt = mapFromGlobal( event->globalPos() );
    QWebFrame *pFrame = m_webViewPtr->page()->frameAt( pnt );
    if( pFrame )
    {
        QWebElement element = pFrame->hitTestContent( m_webViewPtr->mapFromGlobal( event->globalPos() ) ).element();
        if( m_element != element )
        {
            m_element = element;
            update();
            QString str = tr( "Inner xml: %1\nOuter xml: %2\nPlain text: %3" )
        .arg( m_element.toInnerXml() )
        .arg( m_element.toOuterXml() )
        .arg( m_element.toPlainText() );
            emit signalHtmlElement( str );
            
        }
    }
}

void EWAHTMLSelector::mouseReleaseEvent( QMouseEvent *event )
{
    Q_UNUSED( event );
    
    if( m_elSelected == m_element )
    {//-- clear selection
        m_elSelected = QWebElement();
    }
    else
    {
        m_elSelected = m_element;
    }
    
    if( m_webViewPtr )
    {
        m_elementsNumbersPath = m_webViewPtr->getPathFromWebElement( m_elSelected );
    }
    
    drawElementsRects();
    update();
}

void EWAHTMLSelector::mouseDoubleClickEvent( QMouseEvent *event )
{
    Q_UNUSED( event );
}

bool EWAHTMLSelector::eventFilter( QObject *obj, QEvent *event )
{
    if( event->type() == QEvent::Resize )
    {//-- update our size:
        QResizeEvent *rszEvent = static_cast<QResizeEvent*>( event );
        if( rszEvent )
        {
            this->resize( rszEvent->size() );
        }
        return QObject::eventFilter( obj, event );
    }
    else if( event->type() == QEvent::KeyPress
        || event->type() == QEvent::KeyRelease 
        || event->type() == QEvent::MouseButtonPress
        || event->type() == QEvent::MouseButtonRelease
        || event->type() == QEvent::MouseButtonDblClick   )
    {//-- ignore user input:
        return true;
    }

    //-- standard event processing all other events:
    return QObject::eventFilter( obj, event );
}

void EWAHTMLSelector::show()
{
    QWidget::show();
    m_pDlg->show();
}

 void EWAHTMLSelector::leaveEvent( QEvent *event )
{
    Q_UNUSED( event );
    if( !m_element.isNull() )
    {
        m_element = QWebElement();
        
        update();
    }
}

void EWAHTMLSelector::slotSiteWidgetScrolled( const QPoint& scroll )
{
    Q_UNUSED( scroll );
    
    updateDialogGeometry();
}

void EWAHTMLSelector::slotSiteWidgetGeometryChanged( const QRect& newGeometry )
{
    Q_UNUSED( newGeometry );
    
    updateDialogGeometry();
}

void EWAHTMLSelector::updateDialogGeometry()
{
    if( m_messagePtr )
    {
        QRect msgRect = m_messagePtr->getVisibleWebViewRect();
        msgRect.moveTo( m_messagePtr->getCurrnetScrollPoint() );
        
        QRect dlgRect = m_pDlg->rect();
        dlgRect.moveCenter( QPoint( msgRect.center().x(), msgRect.top() + m_messagePtr->getCustomizedVerticalOffset() ) );
        m_pDlg->setGeometry( dlgRect );
    }
    update();
}

void EWAHTMLSelector::slotUpdateSelection()
{
    initElementFromPath( m_elementsNumbersPath, m_framesNumbersPath );
}

QList<int> EWAHTMLSelector::getElementsPath() 
{ 
    if( m_webViewPtr )
    {
        m_elementsNumbersPath = m_webViewPtr->getPathFromWebElement( m_elSelected );
        
        /*QWebElement tmpEl = m_webViewPtr->getWebElementFromPath( m_elementsNumbersPath );
        QList<int> tmpList = m_webViewPtr->getPathFromWebElement( tmpEl );
        
        Q_ASSERT( tmpEl == m_elSelected );
        Q_ASSERT( tmpList == m_elementsNumbersPath );*/
    }
    
    return m_elementsNumbersPath;
}

QList<int> EWAHTMLSelector::getElementsFramePath()
{
    if( m_webViewPtr 
        && !m_elSelected.isNull() )
    {
        m_framesNumbersPath = m_webViewPtr->getPath2ElementsFrame( m_elSelected );
    }
    
    return m_framesNumbersPath;
}

QString EWAHTMLSelector::getSelectedElementSource() const
{
    return m_elSelected.isNull() ? QString() : m_elSelected.toOuterXml();
}