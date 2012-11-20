/*******************************************************************************
**
** file: ewa_jskeyboardevent.h
**
** class: EWAJSKeyboardEvent
**
** description:
** base for classes of handle keyboard/mouse/focus events on webpages through JavaScript
**
** 15.10.2009
**
** ewauthor@gmail.com
**
*******************************************************************************/

#ifndef EWA_JSKEYBOARDEVENT_H
#define EWA_JSKEYBOARDEVENT_H

#include "ewa_useraction.h"

class EWAJSKeyboardEvent : public EWAUserAction
{
    Q_OBJECT
public:
    enum KeyboardAction
    {
        NoAction = 0
        ,Keydown = 1
        ,Keypress = 2
        ,Keyup = 3
    };
    
    EWAJSKeyboardEvent( const QString& keyAct, QObject *pParent = 0 );
    virtual ~EWAJSKeyboardEvent();
    
    QString getEventType() const { return m_strKbdType;}
    
    EWAJSKeyboardEvent& operator=( const EWAJSKeyboardEvent& other );
    
    virtual int getCharCode() const {return m_iCharCode;}
    virtual int getKeyIdentifier() const { return m_iKeyIdentifier; }
    virtual int getKeyLocation() const { return m_iKeyLocation;}
    virtual int getKeyCode() const { return m_iKeyCode; }
    virtual int getWhich() const { return m_iWhich ; }
    
    virtual void save( QSettings *pSettings );
    virtual void load( QSettings *pSettings );
    virtual QString execute() const;

    
public slots:
    virtual void setEventType( const QString& act );
    virtual void setCharCode( int code );
    virtual void setKeyIdentifier( int keyId );
    virtual void setKeyLocation( int keyLoc );
    virtual void setKeyCode( int code );
    virtual void setWhich( int val );
protected:
    int m_iCharCode;
    int m_iKeyIdentifier;
    int m_iKeyLocation;
    int m_iKeyCode;
    int m_iWhich;
    QString m_strKbdType;//-- The type of event.
};

#endif //-- EWA_JSKEYBOARDEVENT_H