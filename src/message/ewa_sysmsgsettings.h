/*******************************************************************************
**
** file: ewa_sysmsgsettings.h
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

#ifndef EWA_SYSMSGSETTINGS_H
#define EWA_SYSMSGSETTINGS_H

#include <QWidget>
#include "ui_ewa_sysmsgsettings.h"

class EWASiteHandle;
class EWASysTrayMsgSettingsDlg : public QWidget
{
    Q_OBJECT
    
    public:
        EWASysTrayMsgSettingsDlg( QWidget *parent = 0 );
        virtual ~EWASysTrayMsgSettingsDlg();

        void setSitePtr( EWASiteHandle *pSite );

    protected:
        Ui::EWASysMsgSettingsUi ui;
        virtual void changeEvent( QEvent * event )
        {
            if( event->type() == QEvent::LanguageChange )
            {
                ui.retranslateUi( this );
            }

            return QWidget::changeEvent( event );
        }
        EWASiteHandle *m_sitePtr;
        void initTypesComboBox();
};

#endif //-- EWA_SYSMSGSETTINGS_H
