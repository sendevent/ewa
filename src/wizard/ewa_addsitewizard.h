/*******************************************************************************
**
** file: ewa_addsitewizard.h
**
** class: EWAAddSiteWizard
**
** description:
** "Add site" wizard = )
**
** 09.02.2009
**
** sendevent@gmail.com
**
*******************************************************************************/

#ifndef EWA_ADDSITEWIZARD_H
#define EWA_ADDSITEWIZARD_H

#include <QWizard>

class EWASiteHandle;
class EWAASWPage_Greeting;
class EWAASWPage_InputUrl;
class EWAASWPage_BrowserPage;
class EWAAddSiteWizard : public QWizard
{
    Q_OBJECT
    
    public:
        EWAAddSiteWizard( QWidget *parent = 0, EWASiteHandle *pSite = 0 );
        virtual ~EWAAddSiteWizard();

        QString getUrl() const;

        enum PagesIDs
        {
            Page_Greeting = 0,
            Page_InputUrl,
            Page_Record
        };

    protected:
        QString m_qstrWizardTitle;

        EWAASWPage_Greeting *m_pPageGreeting;
        EWAASWPage_InputUrl *m_pPageInputUrl;
        EWAASWPage_BrowserPage *m_pPageBrowser;

        inline const QString getTitle() const { return m_qstrWizardTitle; }

        QWizardPage *getGreetingPage();
        QWizardPage *getInputUrlPage();
        QWizardPage *getBrowserPage( EWASiteHandle *pSite );
        
    protected slots:
        void slotCurrentPageChanged( int id );

    public slots:
        virtual void show();
        virtual void accept();
};
#endif //-- EWA_ADDSITEWIZARD_H
