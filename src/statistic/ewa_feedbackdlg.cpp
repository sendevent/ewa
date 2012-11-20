/*******************************************************************************
**
** file: ewa_feedbackdlg.cpp
**
** class: EWAFeedbackDialog
**
** description:
** Dialog for send user's feedbacks, feature requests, bug reports, etc.
**
** 05.11.2009
**
** ewauthor@indatray.com
**
*******************************************************************************/

#include "ewa_feedbackdlg.h"
#include "ewa_webpage.h"
#include "ewa_application.h"

#include <QMessageBox>
#include <QNetworkAccessManager>
#include <QNetworkRequest>

EWAFeedbackDialog::EWAFeedbackDialog( QWidget *pParent )
:QDialog( pParent )
{
    ui.setupUi( this );
    ui.pFromLineEdit->setText( QString( "%1@%2" ).arg( EWAApplication::getUserName() ).arg( EWAApplication::getMachineName() ) );
    
    on_pTextEdit_textChanged();
    
    m_pNetwork = new QNetworkAccessManager( this );

    connect( m_pNetwork, SIGNAL( finished(QNetworkReply*) ),
        this, SLOT( slotRequestFinished(QNetworkReply*) ) );
    connect( ui.pCloseButton, SIGNAL( clicked() ),
        this, SLOT( close() ) );
    m_errorMsg = tr( "Message not accepted.<br>You can use e-mail:<br><a href=\"mailto:feedback@indatray.com\">feedback@indatray.com</a>" );
}

EWAFeedbackDialog::~EWAFeedbackDialog(void)
{
}

void EWAFeedbackDialog::on_pSendButton_clicked()
{
    if( isFromOk() && isSubjOk() && isMessageOk() )
    {
        ui.stackedWidget->setCurrentIndex( 1 );
        ui.pSendButton->setEnabled( false );
        send();
    }
}

void EWAFeedbackDialog::on_pTextEdit_textChanged()
{
    ui.pSendButton->setEnabled( isMessageOk( false ) );
}

bool EWAFeedbackDialog::isMessageOk( bool bShowWarn )
{
    int iMsgLength = ui.pTextEdit->toPlainText().length();

    QString strWarnMsg;
    if( iMsgLength < 4 )
    {
        strWarnMsg = tr( "It's seems that you forget write message." );
    }
    else if ( iMsgLength > 1024 )
    {
        strWarnMsg = tr( 
            "Your message is too long."
            "<br>Max allowed length - 1024 symbols."
            "<br>Please, decrease message's length,"
            "<br>or use the e-mail."
            "<br>(Brevity - the soul of wit)."  );
    }

    if( !strWarnMsg.isEmpty() )
    {
        if( bShowWarn )
        {
            QMessageBox::information( this, tr( "Message" ), strWarnMsg );
        }
        return false;
    }

    return true;
}

bool EWAFeedbackDialog::isSubjOk()
{
    QString strMsg = ui.pSubjComboBox->currentText();
    int iMsgLength = strMsg.length();

    QString strWarnMsg;
    if( !iMsgLength )
    {
        strWarnMsg = tr( "It's seems that you forget write Subject." );
    }
    else if ( iMsgLength > 128 )
    {
        strMsg = strMsg.left( 128 );
        ui.pTextEdit->clear();
        ui.pTextEdit->insertPlainText( strMsg );

        strWarnMsg = tr( 
            "Your Subject is too long."
            "<br>Max allowed length - 128 symbols."
            "<br>Please, decrease Subject's length,"
            "<br>or use the e-mail."
            "<br>(Brevity - the soul of wit)."  );
    }

    if( !strWarnMsg.isEmpty() )
    {
        QMessageBox::information( this, tr( "Message" ), strWarnMsg );
        return false;
    }

    return true;
}

void EWAFeedbackDialog::on_pSubjComboBox_editTextChanged( const QString& )
{
}

bool EWAFeedbackDialog::isFromOk() 
{
    QString strMsg = ui.pFromLineEdit->text();
    int iMsgLength = strMsg.length();

    QString strWarnMsg;
    if( !iMsgLength )
    {
        strWarnMsg = tr( "Please, input your name and/or e-mail in \"From\" field." );
    }

    if( !strWarnMsg.isEmpty() )
    {
        QMessageBox::information( this, tr( "Message" ), strWarnMsg );
        return false;
    }

    return true;
}

void EWAFeedbackDialog::send()
{
    QString strUa = EWAApplication::getEWAUserAgent();
    QNetworkRequest m_myRequest( QUrl( "http://indatray.com/msg.php" ) );
    m_myRequest.setRawHeader( "User-Agent", strUa.toUtf8() );
    m_myRequest.setRawHeader( "Referer", QString( "http://indatray.com/from_application" ).toUtf8() );
    
    QString strFrom = ui.pFromLineEdit->text();
    QString strSubj = ui.pSubjComboBox->currentText();
    QString strMsg = ui.pTextEdit->toPlainText();
    
    QByteArray baData = "recipient=contact%40indatray.com&email="
        + QUrl::toPercentEncoding( strFrom )
        + "&subject="
        + QUrl::toPercentEncoding( strSubj )
        + "&text="
        + QUrl::toPercentEncoding( strMsg )
        + "&ewa_id="
        + EWAApplication::getXORedString( EWAApplication::getUniqueId().toUtf8().toBase64(), strUa.toUtf8() ).toBase64()
        + "&ok=Send"
        + "&env_report=REMOTE_HOST,REMOTE_ADDR,REMOTE_USER,HTTP_USER_AGENT";
    
    
    m_pNetwork->post( m_myRequest, baData );
}

void EWAFeedbackDialog::slotRequestFinished( QNetworkReply *pReply )
{
    QString strHtml = QString::fromLocal8Bit( pReply->readAll() );
    if( strHtml.isEmpty() )
    {
        strHtml = m_errorMsg;
    }

    ui.pResultLabel->setText( strHtml );
    ui.pResultLabel->adjustSize();
    pReply->deleteLater();
}
