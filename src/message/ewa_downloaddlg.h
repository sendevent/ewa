/*******************************************************************************
**
** file: ewa_downloaddlg.h
**
** class: EWADownloadDlg
**
** description:
** Handler for downloading files ("Save as..", etc)
**
** 18.12.2009
**
** ewauthor@indatray.com
**
*******************************************************************************/

#ifndef EWA_DOWNLOADDIALOG_H
#define EWA_DOWNLOADDIALOG_H

#include "ui_ewa_download.h"

#include <QWidget>
#include <QNetworkReply>

class QFile;
class EWADownloadDlg : public QWidget
{
    Q_OBJECT
    
    public:
        EWADownloadDlg( QWidget *pParent = 0, QNetworkReply *pReply = 0, const QString& fileName = QString() );
        virtual ~EWADownloadDlg();
        
        void init( QNetworkReply *pReply, const QString& fileName );
    protected:
        Ui::EWADownloadUi ui;
        QNetworkReply *m_pReply;
        QFile *m_pFile;
        qint64 m_iBytesReceived;
        bool m_bFinished;
        
        void resetInfo();
        
        bool isDownloaded() const;
        
        void connectReply();
        void disconnectReply();
        
        virtual void paintEvent( QPaintEvent* );
        void updateMinGeometry();
        
    protected slots:
        void slotReadyRead();
        void slotError(QNetworkReply::NetworkError err);
        void slotMetaDataChanged();
        void slotDownloadProgress(qint64, qint64);
        void finished();
        
        void on_pOpenButton_clicked();
        void on_pStopButton_clicked();
};

#endif //-- EWA_DOWNLOADDIALOG_H