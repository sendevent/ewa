/*******************************************************************************
**
** file: ewa_sendstatisticdlg.h
**
** class: EWASendStatisticsDlg
**
** description:
** Dialog for data sending as statistic
**
** 22.04.2010
**
** ewauthor@indatray.com
**
*******************************************************************************/

#ifndef EWA_SENDSTATISTICDLG_H
#define EWA_SENDSTATISTICDLG_H

#include <QDialog>
#include "ui_ewa_sendstatisticdlg.h"

class EWAStatisticModelView;
class EWASendStatisticsDlg : public QDialog
{
    Q_OBJECT
    
    public:
        EWASendStatisticsDlg( QWidget *pParent = 0 );
        ~EWASendStatisticsDlg();
    
    protected:
        Ui::EWASendStatisticDlgUi ui;
    
    protected slots:
        void slotUpdateWidth( int deltaW );
};


#endif //-- EWA_SENDSTATISTICDLG_H