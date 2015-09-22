/*******************************************************************************
**
** file: ewa_statisticmodelview.h
**
** class: EWAStatisticModelView
**
** description:
** View for data sending as statistic
**
** 22.04.2010
**
** sendevent@gmail.com
**
*******************************************************************************/

#ifndef EWA_STATISTICMODELVIEW_H
#define EWA_STATISTICMODELVIEW_H

#include <QTableWidget>

class EWASitesListModel;
class EWAStatisticModelView : public QTableWidget
{
    Q_OBJECT
    
    public:
        EWAStatisticModelView( QWidget *parent = 0 );
        ~EWAStatisticModelView();
        
        virtual void setModel( QAbstractItemModel *model );
    
    protected:
        EWASitesListModel *m_modelPtr;
        
    signals:
    
        void signalWantUpdateWidth( int delta );
};

#endif //-- EWA_STATISTICMODELVIEW_H