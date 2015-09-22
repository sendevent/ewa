/*******************************************************************************
**
** file: ewa_sysmsgsettings.cpp
**
** class: EWASysTrayMsgSettingsDlg
**
** description:
** Settings dialog for system tray messages.
**
** 23.03.2009
**
** sendevent@gmail.com
**
*******************************************************************************/

#include "ewa_sysmsgsettings.h"
#include "ewa_sitehandle.h"
#include <QSystemTrayIcon>

EWASysTrayMsgSettingsDlg::EWASysTrayMsgSettingsDlg( QWidget *parent )
:QWidget( parent )
{
    ui.setupUi( this );
    initTypesComboBox();
}

EWASysTrayMsgSettingsDlg::~EWASysTrayMsgSettingsDlg()
{
}

void EWASysTrayMsgSettingsDlg::initTypesComboBox()
{
    ui.typeComboBox->addItem( tr( "Simple" ), QSystemTrayIcon::NoIcon );
    ui.typeComboBox->addItem( style()->standardIcon( QStyle::SP_MessageBoxInformation ),
                             tr( "Information" ), QSystemTrayIcon::Information );
    ui.typeComboBox->addItem( style()->standardIcon( QStyle::SP_MessageBoxWarning ),
                             tr( "Warning" ), QSystemTrayIcon::Warning );
    ui.typeComboBox->addItem( style()->standardIcon( QStyle::SP_MessageBoxCritical ),
                             tr( "Error" ), QSystemTrayIcon::Critical );
    ui.typeComboBox->setCurrentIndex( 1 );
}

void EWASysTrayMsgSettingsDlg::setSitePtr( EWASiteHandle *pSite )
{
    if( !pSite )
    {
        return;
    }
    m_sitePtr = pSite;

    int type = m_sitePtr->getSysTrayMsgType();
    ui.typeComboBox->setCurrentIndex( type );
    ui.msgLengthGroupBox->setChecked( m_sitePtr->useMsgLengthLimit() );
    ui.charCountSpinBox->setValue( m_sitePtr->getMsgLengthLimit() );
    ui.spacesCheckBox->setChecked( m_sitePtr->noMsgWordsWrap() );
}
