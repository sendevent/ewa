/*******************************************************************************
**
** file: ewa_sitewidgetsettingsdlg.h
**
** class: EWASiteWidgetSettingsDlg
**
** description:
** Settings dialog for "enhanced" ( internal - EWASiteWidget ) messages.
**
** 23.03.2009
**
** ewauthor@indatray.com
**
*******************************************************************************/
#ifndef EWA_SYSMSGSETTINGSEX_H
#define EWA_SYSMSGSETTINGSEX_H

#include <QWidget>
#include "ui_ewa_sitewidgetsettingsdlg.h"

class EWASiteHandle;
class EWASiteWidgetSettingsDlg : public QWidget
{
    Q_OBJECT
    
    public:
        EWASiteWidgetSettingsDlg( QWidget *parent = 0 );
        virtual ~EWASiteWidgetSettingsDlg();

        void setSitePtr( EWASiteHandle *ptr );
        void resetSite();
        
        QStackedWidget *stackedWidget() const {return ui.pSiteWidgetSettingsStackedWidget;}

    protected:
        Ui::EWASiteWidgetSettingsDlgUi ui;
        
        EWASiteHandle *m_sitePtr;
        bool m_bSiteReady;
        QString m_strContentsSizeLabelTemplate;
        QColor m_qcCurrentColor;
        
        bool b1stResizeW, b1stResizeH;
        
        virtual void changeEvent( QEvent * event )
        {
            if( event->type() == QEvent::LanguageChange )
            {
                ui.retranslateUi( this );
            }

            return QWidget::changeEvent( event );
        }

        void init();
        void initCombos();
        
        void connectSiteWidget();
        void disconnectSiteWidget();

    protected slots:
        void on_widthSpinBox_valueChanged( int val );
        void on_heigtSpinBox_valueChanged( int val );
        void on_scrXSpinBox_valueChanged( int val );
        void on_scrYSpinBox_valueChanged( int val );

        void on_adjustSizeButton_clicked();

        void slotWebViewScrolled( const QPoint& view );
        void slotMoveSelection( int );
        void selectionChangedByMove( const QPoint& pnt );

        void on_pageWidthSpinBox_valueChanged( int v );
        void on_pageHeightSpinBox_valueChanged( int v );

        void updateSiteWidgetPropertiesFromSite();
        void updateWebPagePropertiesFromSite();

        void updateControlls();

        void slotSiteWidgetGeometryChanged( const QRect& newGeometry );

        void slotWebPageContentsSizeChanged( const QSize& sz = QSize() );

        void on_backCheckBox_toggled( bool on );
        void on_refreshCheckBox_toggled( bool on );
        void on_forwardCheckBox_toggled( bool on );
        
        void on_titleCheckBox_toggled( bool on );
        void on_stickCheckBox_toggled( bool on );
        void on_maxminCheckBox_toggled( bool on );
        void on_closeCheckBox_toggled( bool on );

    signals:
        void signalXMsgWidthChanged( int val );
        void signalXMsgHeightChanged( int val );
        void signalColorChanged( const QColor& color );
};

#endif //-- EWA_SYSMSGSETTINGSEX_H
