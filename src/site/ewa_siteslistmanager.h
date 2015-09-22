/*******************************************************************************
**
** file: ewa_siteslistmanager.h
**
** class: EWASitesListManager
**
** description:
** Editor for sites list.
**
** 09.02.2009
**
** sendevent@gmail.com
**
*******************************************************************************/

#ifndef EWA_SITESLISTMANAGER_H
#define EWA_SITESLISTMANAGER_H

#include <QWidget>

#include "ui_ewa_siteslistmanager.h"

class EWASitesListModel;
class EWASitesModelView;
class EWANetworkAccessManager;
class EWASiteEditorDlg;
class QSettings;
class QTableView;
class EWASiteHandle;
class EWASitePtr;
class QNetworkProxy;
class EWATimer;

class EWASitesListManager : public QWidget
{
    Q_OBJECT
    
    public:
        EWASitesListManager( QWidget *parent = 0 );
        virtual ~EWASitesListManager();

        inline const QString& getListFileName() const { return m_qstrFileName; }
        QString getSitesFolder() const;

        bool createSitesList();
        bool loadSitesList( const QString& path = QString() );

        EWASiteHandle* createSite( const EWASiteHandle *pCopyIt = 0 );
        void deleteSite( EWASiteHandle *pSite );
        
        int getSitesCount() const;

        void save( bool bForce = false );
        void load( bool processVersion = true );
        void loadV10_11( QSettings& list );
        void loadV12( QSettings& list );
        
        EWASiteHandle* getCurrentSite() const;

        EWASiteHandle* getSiteByNumber( int num ) const;

        EWASiteHandle* getNextSite() const;
        EWASiteHandle* getPrevSite() const;

        void setDownloadTypeSerial();
        void setDownloadTypeParallely();
        bool isDownloadTypeParallel() const;

        bool startSites();
        void stopSites( bool bCloseMsgs = false );
        void pauseSites( bool bPause );

        void setProxyForList(const QNetworkProxy &proxy);

        int getSiteNumber( EWASiteHandle* pSite ) const;

        QAbstractTableModel* getModel() const;
        EWASitesModelView* getMenuTableView() const {return m_pContextMenuView;}
        
        void setClearSite();

        EWASiteHandle* getNearestSite() const;

        void closeSiteWidgets( int hideMode );
        
        void setCacheDir( const QString& dir );
        void setCacheCapacity( qint64 value );

        void showContextMenuForSite( const QPoint& pnt, EWASiteHandle *pSite );
        
        void showEditorDialogForSite( EWASiteHandle *pSite, bool bIsNew = false );
        
        bool isStarted() const {return m_bStarted;}
        bool isPaused() const {return m_bPaused;}

    protected:
        Ui::EWASitesListManagerUi ui;
        
        EWASiteHandle *m_pLastSite, *m_pPrevSite;
        EWASitesListModel *m_pModel;
        EWASitesModelView *m_listViewPtr, *m_pContextMenuView;
        EWASiteEditorDlg *m_pSiteEditorDlg;
        EWATimer *m_pTimer;
        
        bool m_bPaused
            ,m_bStarted
            ,m_bDownloadParallel;
            
        QString m_qstrFileName;
        int m_iSelectedSitesCount;
        
        virtual void changeEvent( QEvent * event )
        {
            if( event->type() == QEvent::LanguageChange )
            {
                ui.retranslateUi( this );
                updateButtonsTooltips();
                slotSitesListCountInfo();
            }
            
            return QWidget::changeEvent( event );
        }

        void construct();
        void initProperties();
        void initInternalConnects();

        void clearIniFile();
        void clearSitesModel();

        EWASiteHandle* getSelectedSite() const;
        EWASiteHandle* getNextSite( EWASiteHandle* pSite ) const;
        int getCurrentSiteIndex() const;

        void selectedTimeUnitSeconds();
        void selectedTimeUnitMinutes();
        void selectedTimeUnitHours();

        void setControllsEnabled( bool on );

        bool canStart();

        bool isOneAtLeastSelected() const;
        
        
        void processESLVersion( QString& strVersion );

    protected slots:
        void on_addSiteButton_clicked();
        void on_delSiteButton_clicked();

        void on_moveSiteUpButton_clicked();
        void on_moveSiteDownButton_clicked();

        void on_editSiteButton_clicked();
        void slotModelItemActivated( const QModelIndex& index );

        void slotSiteDownloaded( const EWASitePtr *pSite );

        void updateUi();
        void updateButtonsTooltips();

        void slotOneSecond();

        void slotShowContextMenu(const QPoint&);
        
        void slotSitesListCountInfo();
        
        void on_pUnSelectAllButton_clicked();
        
    signals:
        void signalSitesListChanged( const QString& fName );
        void signalContextMenuItemClicked( const QModelIndex& index );
        void signalContextMenuItemDoubleClicked( const QModelIndex& index );
};

#endif //-- EWA_SITESLISTMANAGER_H
