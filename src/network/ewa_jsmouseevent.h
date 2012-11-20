/*******************************************************************************
**
** file: ewa_jsmouseevent.h
**
** class: EWAJSMouseEvent
**
** description:
** base for classes of handle keyboard/mouse/focus events on webpages through JavaScript
**
** 15.10.2009
**
** ewauthor@gmail.com
**
*******************************************************************************/

#ifndef EWA_JSMOUSEEVENT_H
#define EWA_JSMOUSEEVENT_H

#include "ewa_useraction.h"

class QSettings;
class EWAJSMouseEvent : public EWAUserAction
{
    Q_OBJECT
    Q_PROPERTY( QString m_strEventType READ getEventType WRITE setEventType )
    Q_PROPERTY( int m_iDetail READ getDetail WRITE setDetail )
    Q_PROPERTY( int m_iScreenX READ getScreenX WRITE setScreenX )
    Q_PROPERTY( int m_iScreenY READ getScreenY WRITE setScreenY )
    Q_PROPERTY( int m_iClientX READ getClientX WRITE setClientX )
    Q_PROPERTY( int m_iClientY READ getClientY WRITE setClientY )
    //Q_PROPERTY( bool m_belatedTarget READ button WRITE setButton )
    Q_PROPERTY( int m_iButton READ getButton WRITE setButton )
    Q_PROPERTY( int m_iLayerX READ getLayerX WRITE setLayerX )
    Q_PROPERTY( int m_iLayerY READ getLayerY WRITE setLayerY )
    Q_PROPERTY( int m_iOffsetX READ getOffsetX WRITE setOffsetX )
    Q_PROPERTY( int m_iOffsetY READ getOffsetY WRITE setOffsetY )
    Q_PROPERTY( int m_iPageX READ getPageX WRITE setPageX )
    Q_PROPERTY( int m_iPageY READ getPageY WRITE setPageY )
    Q_PROPERTY( int m_iX READ getX WRITE setX )
    Q_PROPERTY( int m_iY READ getY WRITE setY )
    
public:
    /*enum MouseAction
    {
        NoAction = 0
        ,Click
        ,Mousedown
        ,Mouseup
        ,Dblclick
    };*/
    
    EWAJSMouseEvent( const QString& type, QObject *pParent = 0 );
    virtual ~EWAJSMouseEvent();
    
    EWAJSMouseEvent& operator=( const EWAJSMouseEvent& other );

    QString typeAsString() const;

    virtual QString getEventType() const { return m_strEventType;}
    virtual int getDetail() const {return m_iDetail;}
    virtual int getScreenX() const { return m_iScreenX ;}
    virtual int getScreenY() const {return m_iScreenY;} 
    virtual int getClientX() const { return m_iClientX; }
    virtual int getClientY() const {return m_iClientY; }
    virtual int getButton() const {return m_iButton;}
    virtual int getLayerX() const {return m_iLayerX; }
    virtual int getLayerY() const {return m_iLayerY;}
    virtual int getOffsetX() const {return m_iOffsetX;}
    virtual int getOffsetY() const {return m_iOffsetY;}
    virtual int getPageX() const {return m_iPageX;}
    virtual int getPageY() const {return m_iPageY;}
    virtual int getX() const {return m_iX;}
    virtual int getY() const {return m_iY;}
    
    virtual void save( QSettings *pSettings );
    virtual void load( QSettings *pSettings );
    
    virtual QString execute() const;
    
public slots:
    virtual void setEventType( const QString& type );
    virtual void setDetail( int detail );
    virtual void setScreenX( int x );
    virtual void setScreenY( int y );
    virtual void setClientX( int x );
    virtual void setClientY( int y );
    
    virtual void setButton( int button );
    virtual void setLayerX( int x );
    virtual void setLayerY( int y );
    virtual void setOffsetX( int offset );
    virtual void setOffsetY( int offset );
    virtual void setPageX( int x );
    virtual void setPageY( int y );
    virtual void setX( int x );
    virtual void setY( int y );
protected:    
    QString m_strEventType; //-- the string to set the event's type to. Possible types for mouse events include: click, mousedown, mouseup, mouseover, mousemove, mouseout. 
    //view ; //-- the Event's AbstractView. You should pass the window object here. Sets the value of event.view. 
    int m_iDetail; //-- the Event's mouse click count. Sets the value of event.detail. 
    int m_iScreenX; //-- the Event's screen x coordinate. Sets the value of event.screenX. 
    int m_iScreenY; //-- the Event's screen y coordinate. Sets the value of event.screenY. 
    int m_iClientX; //-- the Event's client x coordinate. Sets the value of event.clientX. 
    int m_iClientY; //-- the Event's client y coordinate. Sets the value of event.clientY. 
    bool m_belatedTarget; //-- the Event's related EventTarget. Only used with some event types (e.g. mouseover and mouseout). In other cases, pass null. 
    int m_iButton;
    int m_iLayerX;
    int m_iLayerY;
    int m_iOffsetX;
    int m_iOffsetY;
    int m_iPageX;
    int m_iPageY;
    int m_iX;
    int m_iY;
};

#endif //-- EWA_JSMOUSEEVENT_H