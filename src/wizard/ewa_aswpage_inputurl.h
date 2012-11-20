/*******************************************************************************
**
** file: ewa_aswpage_inputurl.h
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

#ifndef EWA_ASWPAGE_INPUTURL_H
#define EWA_ASWPAGE_INPUTURL_H

#include <QWizardPage>
#include "ui_ewa_aswpage_inputurl.h"

class EWAASWPage_InputUrl : public QWizardPage
{
    Q_OBJECT
    
    public:
        EWAASWPage_InputUrl( QWidget *parent = 0 );
        virtual ~EWAASWPage_InputUrl();
    
    protected:
        virtual void initializePage();

    private:
        Ui::EWAASWPage_InputUrlUi ui;
        virtual void changeEvent( QEvent * event )
        {
            if( event->type() == QEvent::LanguageChange )
            {
                ui.retranslateUi( this );
            }

            return QWidget::changeEvent( event );
        }
};


#endif //-- EWA_ASWPAGE_INPUTURL_H
