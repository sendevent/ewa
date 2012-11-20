#ifndef EWA_COLORCHOOSER_H
#define EWA_COLORCHOOSER_H

#include <QWidget>
#include <QCursor>
#include "ui_ewa_colorchooser.h"

class EWAColorChooser : public QWidget
{
    Q_OBJECT

    public:
        EWAColorChooser( QWidget *parent = 0 );
        virtual ~EWAColorChooser();

    protected:
        Ui::EWAColorChooserUi ui;
        virtual void changeEvent( QEvent * event )
        {
            if( event->type() == QEvent::LanguageChange )
            {
                ui.retranslateUi( this );
            }

            return QWidget::changeEvent( event );
        }

        virtual void mousePressEvent( QMouseEvent *event );
        virtual void mouseReleaseEvent( QMouseEvent *event );

        virtual void mouseMoveEvent( QMouseEvent *event );

        bool m_bCursorHandled, m_bSelectionStarted;
        QCursor m_pMyCursor;

        void setCursorInternal();
        void resetCursorInternal();

        void choseColor( const QPoint& globalPos ) const;

    signals:
        void signalColorChanged( const QRgb& rgb ) const;
};
#endif //-- EWA_COLORCHOOSER_H
