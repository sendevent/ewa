#ifndef EWA_TEXTSEARCH_H
#define EWA_TEXTSEARCH_H

#include <QWidget>
#include <QTextDocument>

#include "ui_ewa_textsearch.h"

class EWATextSearch : public QWidget
{
    Q_OBJECT

    public:
        EWATextSearch( QWidget *parent = 0 );
        virtual ~EWATextSearch();

    protected:
        Ui::EWATextSearchUi ui;
        
        bool canSearch();
        void askSearch( bool next = true );
        virtual void changeEvent( QEvent * event )
        {
            if( event->type() == QEvent::LanguageChange )
            {
                ui.retranslateUi( this );
            }

            return QWidget::changeEvent( event );
        }
        
    protected slots:
        void on_lineEdit_textChanged( const QString& text );
        void on_findPrevButton_clicked();
        void on_findNextButton_clicked();
        void on_csCheckBox_toggled( bool on );
        void on_wwCheckBox_toggled( bool on );

    signals:
        void signalNeedSearch( const QString& text, QTextDocument::FindFlags options, bool& success );
};

#endif //-- EWA_TEXTSEARCH_H