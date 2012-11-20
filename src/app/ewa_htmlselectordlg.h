/*******************************************************************************
**
** file: ewa_htmlselectordlg.h
**
** class: EWAHtmlSelectorDlg
**
** description:
** Dialog for customizing regexp for monitoring changes on the sites
**
** 26.01.2010
**
** ewauthor@indatray.com
**
*******************************************************************************/

#ifndef EWA_HTMLSELECTORDLG_H
#define EWA_HTMLSELECTORDLG_H

#include <QWidget>

#include "ui_ewa_htmlselectordlg.h"

class EWAHtmlSelectorDlg :public QWidget
{
    Q_OBJECT
    
    public:
        EWAHtmlSelectorDlg( QWidget *pParent = 0 );
        virtual ~EWAHtmlSelectorDlg();
        
    protected:
        Ui::EWAHtmlSelectorDlgUi ui;
        QPoint m_ptPrevPos;
        
        virtual void paintEvent( QPaintEvent *event );
        virtual void mouseMoveEvent( QMouseEvent *event );
        virtual void mousePressEvent( QMouseEvent *event );
        virtual void mouseReleaseEvent( QMouseEvent *event );

    protected slots:
        void on_pButtonOk_clicked();
        void on_pButtonCancel_clicked();
        
    signals:
        void signalOk();
        void signalCancel();
};

#endif //-- EWA_HTMLSELECTORDLG_H