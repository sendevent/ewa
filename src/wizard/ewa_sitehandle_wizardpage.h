/*******************************************************************************
**
** file: ewa_sitehandle_wizardpage.h
**
** class: EWAASWPage_BrowserPage
**
** description:
** WizardPage with SiteHandle object.
** Used in "Add site" wizard.
**
** 09.02.2009
**
** ewauthor@indatray.com
**
*******************************************************************************/

#ifndef EWA_SITEHANDLE_WIZARDPAGE_H
#define EWA_SITEHANDLE_WIZARDPAGE_H

#include <QWizardPage>
#include "ui_ewa_sitehandle_wizardpage.h"

class EWASiteHandle;
class EWAASWPage_BrowserPage : public QWizardPage
{
    Q_OBJECT

    public:
        EWAASWPage_BrowserPage( QWidget *parent = 0 );

        void setSite( EWASiteHandle *pSite );
        virtual ~EWAASWPage_BrowserPage();
        void cleanupPage();

    protected:
        Ui::EWASiteHandle_WizardPageUi ui;
        virtual void changeEvent( QEvent * event )
        {
            if( event->type() == QEvent::LanguageChange )
            {
                ui.retranslateUi( this );
            }

            return QWidget::changeEvent( event );
        }
        
        void releaseSite();
        
        EWASiteHandle *m_pSite;
        QWidget *m_pPrevMsgParent;
        QSize m_PrevMinSz,m_PrevMaxSz;
        Qt::WindowFlags prevFlags;

        virtual void initializePage();
        
        void enableNavButtons( bool enable );

        void prepareSiteWidget( bool bForWizard );

    protected slots:
        void slotLoadStarted();
        void slotPageLoaded( bool );

friend class EWAAddSiteWizard;        
};

#endif //-- EWA_SITEHANDLE_WIZARDPAGE_H
