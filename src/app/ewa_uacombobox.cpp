/*******************************************************************************
**
** file: ewa_uacombobox.h
**
** class: EWAUAComboBox
**
** description:
** Class of ComboBox that allow to setup Uer-Agent
**
** 24.09.2009
**
** ewauthor@indatray.com
**
*******************************************************************************/

#include "ewa_uacombobox.h"
#include "ewa_application.h"

#include <QInputDialog>

EWAUAComboBox::EWAUAComboBox( QWidget *pParent )
:QComboBox( pParent )
{
    connectMe();

    EWAApplication::initUserAgentsList( m_userAgentsList );
    
    m_strEwa = "EWA";
    m_strCustom  = "Custom...";
    
    QStringList names = m_userAgentsList.keys();
    names.removeAt( names.indexOf( m_strEwa ) );
    names.removeAt( names.indexOf( m_strCustom ) );
    addItems( names );
    
    m_IEWAId = 0;
    m_iCustomId = m_userAgentsList.count()-1;
    insertItem( 0, m_strEwa );
    insertItem( m_iCustomId, m_strCustom  );
    
}

EWAUAComboBox::~EWAUAComboBox()
{
}

void EWAUAComboBox::connectMe()
{
    connect( this, SIGNAL( activated(const QString&) ),
        this, SLOT( slotItemActivated(const QString&) ) );
}
void EWAUAComboBox::disconnectMe()
{
    disconnect( this, SIGNAL( activated(const QString&) ),
        this, SLOT( slotItemActivated(const QString&) ) );
}

void EWAUAComboBox::setUserAgent( const QString& ua )
{
    m_strUserAgent = ua;
    setToolTip( ua );
}

void EWAUAComboBox::slotItemActivated( const QString& text )
{
    QString ua;
    if( text.compare( m_strCustom ) )
	{
		ua = m_userAgentsList.value( text );
	}
	else
    {
        bool ok = false;
        QString newUA = QInputDialog::getText( this, tr( "User-Agent:" ), tr( "Input new value (&quot;<b>%1</b>&quot; will be replaced by current system's locale name):" ),
			QLineEdit::Normal, m_strLastSelected, &ok );
			
	    if( !ok )
	    {
	        QString prevName = m_userAgentsList.key( m_strLastSelected );
	        int prevId = findText( prevName );
	        return setCurrentIndex( prevId );
	    }
	    
		ua = newUA;
    }

    setUserAgent( ua );
	
    emit signalUserAgentChanged( ua );
    m_strLastSelected = ua;
}

void EWAUAComboBox::slotSetUserAgent( const QString& text )
{
    disconnectMe();
    
    QString uaName = m_userAgentsList.key( text );
	if( uaName.isEmpty() )
    {
        uaName = EWAApplication::translate( this, m_strCustom.toUtf8().data() );
    }

    setCurrentIndex( findText( uaName ) );
    setToolTip( text );

    emit signalUserAgentChanged( text );
    
    m_strLastSelected = text;

    connectMe();
}

QString EWAUAComboBox::userAgent() const
{
    QString userAgent;
    if( currentText().compare( EWAApplication::translate( this, m_strCustom.toUtf8().data() ) ) )
    {
        userAgent = toolTip();
    }
    else
    {
        userAgent = m_userAgentsList.value( currentText() );
    }

    return userAgent;
}

 void EWAUAComboBox::changeEvent( QEvent * event )
{
    if( event->type() == QEvent::LanguageChange )
    {
        setItemText( m_IEWAId, EWAApplication::translate( this, 
            m_strEwa.toUtf8().data()) );
        setItemText( m_iCustomId, EWAApplication::translate( this, 
            m_strCustom.toUtf8().data()) );
    }

    return QComboBox::changeEvent( event );
}
