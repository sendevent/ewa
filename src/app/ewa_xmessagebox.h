/*******************************************************************************
**
** file: ewa_xmessagebox.h
**
** class: EWAXMessageBox
**
** description:
** Customized MessageBox with "Don't show this message again" checkbox.
**
** 20.09.2010
**
** ewauthor@indatray.com
**
*******************************************************************************/

#ifndef EWA_XMESSAGEBOX_H
#define EWA_XMESSAGEBOX_H

#include <QDialog>

#include "ui_ewa_xmessagebox.h"

class EWAXMessageBox : public QDialog
{
    Q_OBJECT
    public:
        EWAXMessageBox( QWidget *pParent = 0
            ,bool bShowAgain = true
            ,const QString& strTitle = QString()
            ,const QString& strMessage = QString()
            ,const QPixmap& pm = QPixmap() );
            
        virtual ~EWAXMessageBox();
        
        bool isShowAgain() const {return m_bShowAgain;}
        void setShowAgain( bool bShow ) 
        {
            m_bShowAgain = bShow;
            ui.pCheckBox->setChecked( m_bShowAgain );
        }
    
    protected:
        Ui::EWAXMessageBoxUi ui;
        
        virtual void changeEvent( QEvent * event )
        {
            if( event->type() == QEvent::LanguageChange )
            {
                ui.retranslateUi( this );
            }

            return QDialog::changeEvent( event );
        }
    
        bool m_bShowAgain;
    
    protected slots:
        void on_pCheckBox_stateChanged( int );
    
    public slots:
        void setText( const QString& text );
        void setPixmap( const QPixmap& pm );
    
    signals:
        void signalShowAgainChanged( bool );
};

#endif //-- EWA_XMESSAGEBOX_H