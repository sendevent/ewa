#ifndef EWA_XMESSAGECOLORMANAGER_H
#define EWA_XMESSAGECOLORMANAGER_H

#include <QWidget>

#include "ui_ewa_sitewidgetcolormanager.h"

class QPixmap;
class EWASiteHandle;
class EWASiteWidgetColorManager : public QWidget
{
    Q_OBJECT
    
    public:
        EWASiteWidgetColorManager( QWidget *pParent = 0  );
        ~EWASiteWidgetColorManager();
        
        void setSite( EWASiteHandle *pSite );
        void resetSite();
        
    protected:
        Ui::EWASiteWidgetColorManagerUi ui;
        EWASiteHandle *m_sitePtr;
        QPixmap *m_pPix;
        
        virtual void changeEvent( QEvent * event )
        {
            if( event->type() == QEvent::LanguageChange )
            {
                ui.retranslateUi( this );
            }

            return QWidget::changeEvent( event );
        }
        
        void connectSiteWidget();
        void disconnectSiteWidget();

    public slots:
        void slotSetButtonColor( const QColor& color );

    protected slots:
        void on_changeColorButton_clicked();
        void on_transparencySpinBox_valueChanged( int val );
        void slotSiteWidgetOpacityChanged( int value );
};

#endif //-- EWA_XMESSAGECOLORMANAGER_H