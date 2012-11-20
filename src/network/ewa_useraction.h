/*******************************************************************************
**
** file: ewa_useraction.h
**
** class: EWAUserAction
**
** description:
** base for classes of handle keyboard/mouse/focus events on webpages through JavaScript
**
** 14.10.2009
**
** ewauthor@indatray.com
**
*******************************************************************************/

#ifndef EWA_USERACTION_H
#define EWA_USERACTION_H

#include <QEvent>
#include <QSize>
#include <QPoint>

class QSettings;
class QKeyEvent;
class QMouseEvent;
class EWAWebView;
class QWebView;
class EWAUserAction 
{
    public:
        EWAUserAction( const QEvent *pEvent = 0, int iTime = -1, EWAWebView *pWebView = 0);
        EWAUserAction( const EWAUserAction& other );
        virtual ~EWAUserAction();

        EWAUserAction& operator=( const EWAUserAction& other );
        
        virtual void setTime( int t );
        virtual int getTime() const {return m_iTime;}
        
        virtual void setEvent( const QEvent* pEvent );
        virtual QEvent* getEvent() const {return m_pEvent;}
        
        void execute( QWebView *pWidget ) const;
            
        virtual void save( QSettings *pSettings );
        virtual void load( QSettings *pSettings );
        
        QSize getWebViewSize() const { return m_webViewSize;}
        void setWebViewSize( const QSize& sz ) { m_webViewSize = sz;}
        
        QString getActionValue() const;
        bool isKeyEvent() const;
        bool isMouseEvent() const;
        
        QPoint getClickCoords() const;
        void setClickCoords( const QPoint& pnt );
        QString getKeyValue() const;
    protected:
        int m_iTime;
        QEvent *m_pEvent;
        QSize m_webViewSize;
        EWAWebView *m_pWebView;
        
        bool isKeyEvent( QEvent::Type eType ) const;
        bool isMouseEvent( QEvent::Type eType ) const;
        
        void rememberKeyEvent( const QKeyEvent *pEvent );
        void rememberMouseEvent( const QMouseEvent *pEvent );
        
        bool saveMouseEvent( QSettings *pSettings, QMouseEvent *pEvent );
        bool saveKeyEvent( QSettings *pSettings, QKeyEvent *pEvent );
        
        QString getKeyValue( const QEvent *pEvent ) const;
        QPoint getClickCoords( const QEvent *pEvent ) const;
};

#endif //-- EWA_USERACTION_H
