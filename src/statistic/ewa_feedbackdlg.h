/*******************************************************************************
**
** file: ewa_feedbackdlg.h
**
** class: EWAFeedbackDialog
**
** description:
** Dialog for send user's feedbacks, feature requests, bug reports, etc.
**
** 05.11.2009
**
** sendevent@gmail.com
**
*******************************************************************************/

#ifndef EWA_FEEDBACKDLG_H
#define EWA_FEEDBACKDLG_H

#include <QDialog>
#include "ui_ewa_feedbackdlg.h"

class QNetworkAccessManager;
class QNetworkReply;

class EWAFeedbackDialog : public QDialog
{
    Q_OBJECT
    
    public:
        EWAFeedbackDialog( QWidget *pParent = 0 );
        virtual ~EWAFeedbackDialog();

    protected:
        Ui::EWAFeedBackDialogUi ui;
        QNetworkAccessManager *m_pNetwork;
        QString m_errorMsg;

        bool isMessageOk( bool bShowWarn = true );
        bool isSubjOk();
        bool isFromOk();
        
        void send();

    protected slots:
        void on_pSendButton_clicked();
        void on_pTextEdit_textChanged();
        void on_pSubjComboBox_editTextChanged( const QString& text );
        
        void slotRequestFinished( QNetworkReply *pReply );
        
    signals:
        void signalTmp();
};

#endif //-- EWA_FEEDBACKDLG_H
