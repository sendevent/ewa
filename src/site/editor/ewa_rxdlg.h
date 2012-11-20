/*******************************************************************************
**
** file: ewa_regexpdialog.h
**
** class: EWARegExpDialog
**
** description:
** Dialog for creating regexps.
**
** 09.02.2009
**
** ewauthor@indatray.com
**
*******************************************************************************/

#ifndef EWA_RXDLG_H
#define EWA_RXDLG_H

#include <QDialog>

#include "ui_ewa_rxdlg.h"

class EWAParsingRule;
class EWARegExpDialog : public QDialog
{
    Q_OBJECT

    public:
        EWARegExpDialog( const QString& srcText = tr( "( 10 + delta4 ) * 32" ), bool isMinimal = true,
            bool isCaseSensitive = true, const QString& pattern = QString( "(.*)" ),
            const QString& dst = QString( "%" ),
            QWidget *parent = 0, bool bHideReplaceWithFields = false );
        virtual ~EWARegExpDialog();

        EWAParsingRule getRule() const;
    protected:
        Ui::EWARegExpDialogUi ui;
        virtual void changeEvent( QEvent * event )
        {
            if( event->type() == QEvent::LanguageChange )
            {
                ui.retranslateUi( this );
            }

            return QWidget::changeEvent( event );
        }

        void showHelp();

    protected slots:
        bool testRegExp();
        void slotBtnClicked( QAbstractButton * button );
        void slotSearchInSource( const QString& text, QTextDocument::FindFlags options, bool& res );

    public slots:
        void accept();
        void show();
};
#endif //-- EWA_RXDLG_H
