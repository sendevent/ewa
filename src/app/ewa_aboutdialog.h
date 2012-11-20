/*******************************************************************************
**
** file: ewa_aboutdialog.h
**
** class: EWAAboutDialog
**
** description:
** "About..." dialog implementation
**
** 23.03.2009
**
** ewauthor@indatray.com
**
*******************************************************************************/

#ifndef EWA_ABOUTDIALOG_H
#define EWA_ABOUTDIALOG_H

#include <QDialog>
#include "ui_ewa_about.h"

class EWATimer;
class EWAAboutDialog : public QDialog
{
    Q_OBJECT

    public:
        EWAAboutDialog( QWidget *parent = 0 );
        ~EWAAboutDialog();

    protected:
        Ui::AboutDialogUi ui;
        QString m_strUsedText, m_strSrcText;
        EWATimer *m_pCharTimer;
        int maxTicks, minTicks;
        unsigned m_uiPrintCounter;
        
        QString m_strUserName
            ,m_strMachineName;

        void printNext();
        void echo( const QString& text );

        const QString getUserAtMachine() const;
        const QString getCommandMarkText( int mode ) const;
        const QString getAboutDefaultText() const;
        const QString getAboutCreditsText() const;
        const QString getAboutDebitsText() const;

        long calcWidth() const;
        long getMaxLineWidth( const QString& text, const QString& linesSeparator ) const;
        long getLineWidth( const QString& text ) const;
        
        virtual void changeEvent( QEvent * event );

    public slots:
        void show();
        void close();
        void on_creditsButton_toggled( bool on );
        void on_debitsButton_toggled( bool on );

    protected slots:
        void slotTick();
        void slotShowContextMenu( const QPoint& pt );
        void slotShowFeedbackForm();
};

#endif //-- EWA_ABOUTDIALOG_H
