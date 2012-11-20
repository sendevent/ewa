/*******************************************************************************
**
** file: ewa_siteslistmodel.h
**
** class: EWASitesListModel
**
** description:
** Model for sites set.
**
** 23.03.2009
**
** ewauthor@indatray.com
**
*******************************************************************************/

#ifndef EWA_SITESLISTMODEL_H
#define EWA_SITESLISTMODEL_H

#include <QAbstractTableModel>
#include <QList>

class EWASiteHandle;
class EWANetworkAccessManager;
class EWASitesListModel : public QAbstractTableModel
{
    Q_OBJECT
    
    public:
        EWASitesListModel( QObject *parent = 0 );
        virtual ~EWASitesListModel();

        QVariant data( const QModelIndex &index, int role = Qt::DisplayRole ) const;
        Qt::ItemFlags flags( const QModelIndex &index ) const;

        virtual int rowCount( const QModelIndex &parent = QModelIndex() ) const;
        virtual int columnCount( const QModelIndex & parent = QModelIndex() ) const;

        EWASiteHandle* addSite( const EWASiteHandle *pCopyIt = 0 );
        bool removeSite( EWASiteHandle *pSite );
        bool removeRows( int row, int count, const QModelIndex &parent );
        bool removeRow( int row );

        EWASiteHandle *getSiteByNumber( int number ) const;
        int getSiteNumber( EWASiteHandle *pSite ) const;

        QModelIndex increaseSitePosition( int pos );
        QModelIndex decreaseSitePosition( int pos );
        
        void setModified( bool on ) {m_bModified = on;}
        bool isModified() const {return m_bModified;}
        
        void setRowUnderMouse( int iRow ) { m_iRowUnderMouse = iRow; }
        int getRowUnderMouse() const {return m_iRowUnderMouse;}

    private:
        QList<EWASiteHandle*> *m_pSitesList;
        QList<EWANetworkAccessManager*> *m_pNetworkManagersList;
        bool m_bModified;
        int m_iRowUnderMouse;
        
        void setDirty();
        void setClean();

    protected slots:
        void slotSiteChanged();
        void slotSetModified();
        void slotProcessLoadingStateChange();
};

#endif //-- EWA_SITESLISTMODEL_H
