/*******************************************************************************
**
** file: ewa_aswpage_inputurl.cpp
**
** class: EWAASWPage_InputUrl
**
** description:
** "Input URL" page for "Add site" wizard.
**
** 09.02.2009
**
** ewauthor@indatray.com
**
*******************************************************************************/

#include "ewa_aswpage_inputurl.h"

EWAASWPage_InputUrl::EWAASWPage_InputUrl( QWidget *parent )
:QWizardPage( parent )
{
    ui.setupUi( this );
    ui.initialUrlLineEdit->setText( QLatin1String("http://") );

    registerField( "initialUrl*", ui.initialUrlLineEdit, "text", SIGNAL(textChanged(const QString&)));
    registerField( "siteDescriptionLineEdit", ui.siteDescriptionLineEdit, "text", SIGNAL(textChanged(const QString&)));
    registerField( "userAgent", ui.userAgentCombo, "m_strUserAgent", SIGNAL(signalUserAgentChanged(const QString&)) );
}

EWAASWPage_InputUrl::~EWAASWPage_InputUrl()
{
}

 void EWAASWPage_InputUrl::initializePage()
{
    ui.userAgentCombo->setCurrentIndex( 0 );
    ui.userAgentCombo->slotItemActivated( QLatin1String( "EWA" ) );
}