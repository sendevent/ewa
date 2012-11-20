/*******************************************************************************
**
** file: ewa_aswpage_greeting.h
**
** class: EWAASWPage_Greeting
**
** description:
** "Greeting" page for "Add site" wizard.
**
** 09.02.2009
**
** ewauthor@indatray.com
**
*******************************************************************************/

#ifndef EWA_ASWPAGE_GREETING_H
#define EWA_ASWPAGE_GREETING_H

#include <QWizardPage>
#include "ui_ewa_aswpage_greeting.h"

class EWAASWPage_Greeting :public QWizardPage
{
    Q_OBJECT
    
    public:
        EWAASWPage_Greeting( QWidget *parent = 0 );
        virtual ~EWAASWPage_Greeting();

    private:
        Ui::EWAASWPage_GreetingUi ui;
        virtual void changeEvent( QEvent * event )
        {
            if( event->type() == QEvent::LanguageChange )
            {
                ui.retranslateUi( this );
            }

            return QWidget::changeEvent( event );
        }
};

#endif //-- EWA_ASWPAGE_GREETING_H
