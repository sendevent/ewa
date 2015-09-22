/*******************************************************************************
**
** file: ewa_sitesmodelview.h
**
** class: EWASitesModelView
**
** description:
** TableView for sites list model.
**
** 23.03.2009
**
** sendevent@gmail.com
**
*******************************************************************************/

#ifndef EWA_SITESMODELVIEW_H
#define EWA_SITESMODELVIEW_H

#include <QTableView>

class EWASitesListModel;
class EWASitesModelView : public QTableView
{
    Q_OBJECT
    
    public:
        EWASitesModelView( QWidget *parent = 0 );
        virtual ~EWASitesModelView();
        
        virtual void setModel( QAbstractItemModel *model );

    protected:
        EWASitesListModel *m_modelPtr;
        
        void mouseReleaseEvent( QMouseEvent *event );
        void mouseMoveEvent( QMouseEvent *event );
        void leaveEvent( QEvent *event );
        void resizeEvent( QResizeEvent *event );
        void showEvent( QShowEvent *event );
        
        void normalizeColumnsWidths();
        
        void updateRow( int iRow );
        
    protected slots:
        void dataChanged ( const QModelIndex& topLeft, const QModelIndex& bottomRight );
        
    public slots:
        void reset();
};
#endif //-- EWA_SITESMODELVIEW_H
