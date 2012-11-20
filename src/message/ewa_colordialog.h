#ifndef EWA_COLORDIALOG_H
#define EWA_COLORDIALOG_H

#include <QDialog>

#include "ui_ewa_colordialog.h"

class QColorDialog;
class QColor;
class EWAColorDialog : public QDialog
{
    Q_OBJECT

    public:
        EWAColorDialog( QWidget *parent = 0 );
        virtual ~EWAColorDialog();

        void setColor( const QColor& color );
        QColor getColor() const;
    
    protected:
        QColorDialog *m_pColorDlg;
        Ui::EWAColorDialogUi ui;
        virtual void changeEvent( QEvent * event )
        {
            if( event->type() == QEvent::LanguageChange )
            {
                ui.retranslateUi( this );
            }

            return QWidget::changeEvent( event );
        }

        void resortChildWidgets();

    protected slots:
        void slotColorChanged( const QRgb& rgb );

    public slots:
        void show();
};
#endif //-- EWA_COLORDIALOG_H
