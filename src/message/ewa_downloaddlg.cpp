/*******************************************************************************
**
** file: ewa_downloaddlg.cpp
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

#include "ewa_downloaddlg.h"

#include <QFile>
#include <QFileInfo>
#include <QDesktopServices>
#include <QPainter>
#include <QMessageBox>
#include <QPaintEvent>

EWADownloadDlg::EWADownloadDlg( QWidget *pParent, QNetworkReply *pReply, const QString& fileName )
:QWidget( pParent )
{
    ui.setupUi( this );
    m_bFinished = false;
    m_pReply = 0;
    m_pFile = 0;
    
    ui.pProgressBar->setMaximum(0);
    init( pReply, fileName );
}

void EWADownloadDlg::resetInfo()
{
    m_iBytesReceived = 0;
    ui.pInfoLabel->setText( tr( "Starting..." ) );
    ui.pUrlLabel->setText( tr( "Uncknown" ) );
    ui.pPathLabel->setText( tr( "Uncknown" ) );
    ui.pProgressBar->setMaximum( 0 );
    ui.pProgressBar->setValue( 0 );
    ui.pOpenButton->setEnabled( false );
    m_bFinished = false;
}

void EWADownloadDlg::init( QNetworkReply *pReply, const QString& fileName )
{
    resetInfo();

    if( pReply && !fileName.isEmpty() )
    {
    
        if( m_pReply )
        {
            disconnectReply();
            m_pFile->close();
        }
        if( m_pFile )
        {
            if( m_pFile->isOpen() )
            {
                m_pFile->close();
            }
            delete m_pFile;
            m_pFile = 0;
        }

        m_pFile = new QFile( fileName );
        if( !m_pFile->open( QIODevice::WriteOnly ) )
        {
            ui.pInfoLabel->setText( tr( "Can't open file %1:\n%2" ).arg( fileName ).arg( m_pFile->errorString() ) );
            return;
        }

        m_pReply = pReply;
        connectReply();

        //-- reset info
        slotReadyRead();
        update();

        if ( m_pReply->error() != QNetworkReply::NoError )
        {
            slotError( m_pReply->error() );
            finished();
        }
    updateMinGeometry();
    }
}

void EWADownloadDlg::connectReply()
{
    if( m_pReply )
    {
        m_pReply->setParent(this);
        connect(m_pReply, SIGNAL(readyRead()), 
            this, SLOT(slotReadyRead()));
        connect(m_pReply, SIGNAL(error(QNetworkReply::NetworkError)),
            this, SLOT(slotError(QNetworkReply::NetworkError)));
        connect(m_pReply, SIGNAL(downloadProgress(qint64, qint64)),
            this, SLOT(slotDownloadProgress(qint64, qint64)));
        connect(m_pReply, SIGNAL(metaDataChanged()),
            this, SLOT(slotMetaDataChanged()));
        connect(m_pReply, SIGNAL(finished()),
            this, SLOT(finished()));
    }
}

void EWADownloadDlg::disconnectReply()
{
    if( m_pReply )
    {
        disconnect(m_pReply, SIGNAL(readyRead()), 
            this, SLOT(slotReadyRead()));
        disconnect(m_pReply, SIGNAL(error(QNetworkReply::NetworkError)),
            this, SLOT(slotError(QNetworkReply::NetworkError)));
        disconnect(m_pReply, SIGNAL(downloadProgress(qint64, qint64)),
            this, SLOT(slotDownloadProgress(qint64, qint64)));
        disconnect(m_pReply, SIGNAL(metaDataChanged()),
            this, SLOT(slotMetaDataChanged()));
        disconnect(m_pReply, SIGNAL(finished()),
            this, SLOT(finished()));
        
        m_pReply->abort();
        delete m_pReply;
        m_pReply = 0;
    }
}


EWADownloadDlg::~EWADownloadDlg()
{
    if( m_pFile )
    {
        if( m_pFile->isOpen() )
        {
            m_pFile->close();
        }
        
        delete m_pFile;
    }
    
    if( m_pReply )
    {
        disconnectReply();
    }
}

void EWADownloadDlg::slotReadyRead()
{
    QString strFileName = QFileInfo( *m_pFile ).absoluteFilePath();
    if( !m_pFile->isOpen() )
    {
        ui.pInfoLabel->setText( tr( "Can't open file\n%1\nfor writing:\n%2" ).arg( strFileName ).arg( m_pFile->errorString() ) );
    }
    else if( m_pReply )
    {
        ui.pUrlLabel->setText( m_pReply->url().toString() );
        ui.pPathLabel->setText( strFileName );
        ui.pInfoLabel->setText( tr( "Ready to download..." ) );
    }
    
    QVariant header = m_pReply->header( QNetworkRequest::ContentLengthHeader );
    bool ok;
    qint64 iSz = header.toLongLong( &ok );
    if( ok && iSz )
    {
        ui.pInfoLabel->setText( tr( "Downloading..." ) );
        qint64 iRes = m_pFile->write( m_pReply->readAll() );
        if( -1 == iRes ) 
        {
            ui.pInfoLabel->setText( tr( "\nError saving: %1\n").arg( m_pFile->errorString() ) );
        }
        else if ( iRes == iSz )
        {
            finished();
        }
    }
    updateMinGeometry();
}

void EWADownloadDlg::updateMinGeometry()
{
    int iWidth = ui.pUrlLabel->width();
    iWidth = qMax( ui.pPathLabel->width(), iWidth );
    iWidth = qMax( ui.pInfoLabel->width(), iWidth );
    
    int iStaticLabelssWidth = ui.label->width();
    iStaticLabelssWidth = qMax( iStaticLabelssWidth, ui.label_2->width() );
    iStaticLabelssWidth = qMax( iStaticLabelssWidth, ui.label_3->width() );
    
    iWidth += iStaticLabelssWidth;
    
    int iMarginL, iMarginT, iMarginR, iMarginB;
    ui.verticalLayout->getContentsMargins ( &iMarginL, &iMarginT, &iMarginR, &iMarginB ) ;
    iWidth += iMarginL + iMarginR;
    
    int iHeight = ui.pUrlLabel->height() + ui.pPathLabel->height() + ui.pInfoLabel->height();
    iHeight += iMarginT + iMarginB + ui.verticalLayout->spacing();
    
    this->setMinimumSize( iWidth, iHeight );
}

void EWADownloadDlg::slotError( QNetworkReply::NetworkError )
{
    ui.pInfoLabel->setText( tr("Network Error: %1").arg(m_pReply->errorString() ) );
}

void EWADownloadDlg::slotMetaDataChanged()
{
    qWarning( "EWADownloadDlg::slotMetaDataChanged: not handled." );
}

void EWADownloadDlg::slotDownloadProgress( qint64 bytesReceived, qint64 bytesTotal )
{
    if( bytesTotal == -1 ) 
    {
        ui.pProgressBar->setValue( 0 );
        ui.pProgressBar->setMaximum( 0 );
    }
    else
    {
        ui.pProgressBar->setValue( bytesReceived );
        ui.pProgressBar->setMaximum( bytesTotal );
    }

    m_iBytesReceived = bytesReceived;
}

void EWADownloadDlg::finished()
{
    m_bFinished = true;
    ui.pProgressBar->setMaximum( 1 );
    ui.pProgressBar->setValue( 1 );
    
    ui.pOpenButton->setEnabled( true );
    m_pFile->close();

    ui.pInfoLabel->setText( tr("Downloaded.") );
    updateMinGeometry();
}

bool EWADownloadDlg::isDownloaded() const
{
    return m_bFinished;
}

void EWADownloadDlg::on_pOpenButton_clicked()
{
    if( m_pFile )
    {
        QFileInfo info( *m_pFile );
        QUrl url = QUrl::fromLocalFile(info.absolutePath());
        QDesktopServices::openUrl(url);
    }
}

void EWADownloadDlg::on_pStopButton_clicked()
{
    if( !isDownloaded() )
    {
        if( QMessageBox::question( this, tr( "Downloading" ), tr( "Abort downloading?" ),
            tr( "Abort and close" ), tr( "Continue downloading" ) ) == 1 )
        {
            return;
        }
    }
    
    close();
}

 void EWADownloadDlg::paintEvent( QPaintEvent* pEvent )
{
    QWidget *pParent = qobject_cast<QWidget*>( parent() );
    if( isVisible() && pParent )
    {
        QRect r = pParent->rect();

        r = pEvent->rect();
        
        QPainter p( this );
        p.setPen( Qt::NoPen );
        p.setBrush( QColor( 55, 55, 55 ) );
        p.setOpacity(0.6);
        p.drawRect( r );
    }
}
