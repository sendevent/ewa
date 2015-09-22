/*******************************************************************************
**
** file: ewa_siteslistmanager.cpp
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

#include "ewa_siteslistmanager.h"

#include "ewa_timer.h"
#include "ewa_sitehandle.h"
#include "ewa_sitesmodelview.h"
#include "ewa_addsitewizard.h"
#include "ewa_siteeditordlg.h"
#include "ewa_mainwindow.h"
#include "ewa_networkaccessmanager.h"
#include "ewa_siteslistmodel.h"
#include "ewa_useractionsrecorder.h"
#include "ewa_useractionsplayer.h"

#include <QFileDialog>
#include <QNetworkDiskCache>
#include <QSettings>
#include <QListView>

EWASitesListManager::EWASitesListManager( QWidget *parent )
:QWidget( parent )
{
    ui.setupUi( this );
    construct();
    updateUi();
}

void EWASitesListManager::construct()
{
    m_iSelectedSitesCount = 0;
    initProperties();
    initInternalConnects();
    m_bStarted = false;
    m_bPaused = false;
    m_bDownloadParallel = false;
}

void EWASitesListManager::initProperties()
{
    m_listViewPtr = ui.sitesTableView;
    m_listViewPtr->setMouseTracking( true );

    m_pSiteEditorDlg = new EWASiteEditorDlg( this );

    m_pModel = new EWASitesListModel( this );
    m_listViewPtr->setModel( m_pModel );

    m_pContextMenuView = new EWASitesModelView( this );
    m_pContextMenuView->setModel( m_pModel );
    m_pContextMenuView->setMaximumHeight( EWAApplication::getScreensRange().x()/10 );

    m_qstrFileName = EWAApplication::settings()->getSitesListFolder();
    m_pLastSite = 0;
    m_pPrevSite = 0;

    m_pTimer = new EWATimer( this );
}

void EWASitesListManager::initInternalConnects()
{
    if( m_listViewPtr )
    {
        connect( m_listViewPtr, SIGNAL( doubleClicked(const QModelIndex&) ),
            this, SLOT( slotModelItemActivated(const QModelIndex&) ) );
        connect( m_listViewPtr->selectionModel(), SIGNAL( selectionChanged(const QItemSelection&,const QItemSelection&) ),
            this, SLOT( updateUi() ) );
        connect( m_listViewPtr, SIGNAL( customContextMenuRequested(const QPoint&) ),
            this, SLOT( slotShowContextMenu(const QPoint&) ) );
    }

    if( m_pContextMenuView )
    {
        connect( m_pContextMenuView, SIGNAL( doubleClicked(const QModelIndex&) ),
            this, SIGNAL( signalContextMenuItemDoubleClicked(const QModelIndex&) ) );
        connect( m_pContextMenuView, SIGNAL( doubleClicked(const QModelIndex&) ),
            this, SLOT( slotModelItemActivated(const QModelIndex&) ) );
        connect( m_pContextMenuView, SIGNAL( clicked(const QModelIndex&) ),
            this, SIGNAL( signalContextMenuItemClicked(const QModelIndex&) ) );
    }

    if( m_pTimer )
    {
        connect( m_pTimer, SIGNAL( signalTimeOut() ), this, SLOT( slotOneSecond() ) );
    }
}

 EWASitesListManager::~EWASitesListManager()
{
    delete m_pModel;
    delete m_listViewPtr;
}

EWASiteHandle* EWASitesListManager::createSite( const EWASiteHandle *pCopyIt )
{
    EWASiteHandle *pSite = m_pModel->addSite( pCopyIt );
    
    connect( pSite, SIGNAL( signalUsageChanged() ),
        this, SLOT( slotSitesListCountInfo() ) );

    connect( pSite, SIGNAL( signalDownloaded(const EWASitePtr*) ),
        this, SLOT( slotSiteDownloaded(const EWASitePtr*) ) );
        
    slotSitesListCountInfo();
    
    return pSite;
}

void EWASitesListManager::deleteSite( EWASiteHandle *pSite )
{
    if( pSite )
    {
        if( QMessageBox::Yes == QMessageBox::question( 0,
        tr( "Delete site" ),
        tr( "<html>Is impossible to restore deleted sites, sorry.<br>"
        "So, do you really want to delete <br><b>[<a href=%1>%1</a>]</b>?</html>" )
        .arg( pSite->getUrl() ), QMessageBox::Yes, QMessageBox::No ) )
        {
            pSite->stop();
            pSite->clearIniFile();
            m_pModel->removeSite( pSite );
            
            slotSitesListCountInfo();
        }
    }
}
bool EWASitesListManager::createSitesList()
{
    QString fileName = QFileDialog::getSaveFileName( this, tr( "Input name of new list file:" ),
                                m_qstrFileName,
                                tr( "EWA Sites List (*.esl)\nAll Files (*.*)" ) );
    if( fileName.isEmpty() )
    {
        return false;
    }

    m_qstrFileName = fileName;
    clearIniFile();
    load( false );
    updateUi();
    on_addSiteButton_clicked();
    emit signalSitesListChanged( m_qstrFileName );

    return true;
}

bool EWASitesListManager::loadSitesList( const QString& path )
{
    if( m_pModel->isModified() )
    {
        save();
    }
    
    QString fileName = path;
    if( fileName.isEmpty() )
    {
        fileName = QFileDialog::getOpenFileName( this, tr( "Select list file:" ),
                                    m_qstrFileName,
                                    tr( "EWA Sites List (*.esl)\nAll Files (*.*)" ) );
        if( fileName.isEmpty() || !QFileInfo(fileName).exists() )
        {
            return false;
        }
    }

    m_qstrFileName = fileName;

    load();

    updateUi();
    
    emit signalSitesListChanged( m_qstrFileName );

    return true;
}

void EWASitesListManager::clearIniFile()
{
    if( !m_qstrFileName.isEmpty() )
    if( QFileInfo( m_qstrFileName ).exists() )
    {
    //-- data stored with QSettings mechanism,
    //-- so, if user try overwrite existing list,
    //-- it'll be edited, but not overwrited.
    //-- we can jast delete this file, it'll
    //-- reinitialized in EWASiteList:
        QFile::remove( m_qstrFileName );
    }
}

void EWASitesListManager::updateUi()
{
    bool bNotStarted = !m_bStarted;
    int selectedRow = m_listViewPtr->currentIndex().row();

    bool canMoveUp = false;
    bool canMoveDown = false;

    if( selectedRow != -1 )
    {
        if( selectedRow > 0 )
            canMoveUp = true;
        if( selectedRow < m_pModel->rowCount()-1 )
            canMoveDown = true;
    }

    ui.moveSiteUpButton->setEnabled( canMoveUp && bNotStarted );
    ui.moveSiteDownButton->setEnabled( canMoveDown && bNotStarted );

    m_listViewPtr->update();
}

void EWASitesListManager::on_addSiteButton_clicked()
{
    EWASiteHandle* pNewSite = createSite();

    EWAAddSiteWizard *siteWizard = new EWAAddSiteWizard( 0, pNewSite );

    bool bUnusedSite = false;

    if( siteWizard->exec() == QDialog::Accepted )
    {
#ifdef Q_OS_LINUX
        /*pNewSite->save();
        EWASiteHandle *pTmpSite = createSite();
        pTmpSite->load( pNewSite->getFileName() );
        m_pModel->removeSite( pNewSite );
        pNewSite = pTmpSite;*/
#endif
        pNewSite->getEWANetworkManagerPtr()->getGesturesRecorder()->stop();
        pNewSite->getEWANetworkManagerPtr()->setCommonDelayForActions( pNewSite->getEWANetworkManagerPtr()->getUserActivitiesPtr(), 15 );
        
        showEditorDialogForSite( pNewSite, true );
        
        pNewSite->getWidget()->showSexy();
    }
    else
    {
        bUnusedSite = true;
    }
    
    delete siteWizard;
    if( bUnusedSite )
    {
        m_pModel->removeSite( pNewSite );
    }

    updateUi();
}

void EWASitesListManager::on_delSiteButton_clicked()
{
    QModelIndex currentPos = m_listViewPtr->currentIndex();
    int id = currentPos.row();
    if( id < 0 )
        return;
        
    EWASiteHandle *sitePtr = m_pModel->getSiteByNumber( id );
    deleteSite( sitePtr );

    save();
    
    int iSitesCount = m_pModel->rowCount();
    if( iSitesCount )
    {
        if( currentPos.row() >= iSitesCount )
        {
            currentPos = m_pModel->index( m_pModel->rowCount() - 1, 0 );
        }
        
        m_listViewPtr->setCurrentIndex( currentPos );
        m_pContextMenuView->setCurrentIndex( currentPos );
    }

    updateUi();
}

void EWASitesListManager::on_moveSiteUpButton_clicked()
{
    int id = m_listViewPtr->currentIndex().row();
    if( id <= 0 || id >= m_pModel->rowCount() )
    {
        return;
    }
    
    QModelIndex newPos = m_pModel->decreaseSitePosition( id );
    
    m_listViewPtr->setCurrentIndex( newPos );
    m_pContextMenuView->setCurrentIndex( newPos );

    save();
}
void EWASitesListManager::on_moveSiteDownButton_clicked()
{
    int id = m_listViewPtr->currentIndex().row();
    if( id < 0 || id >= m_pModel->rowCount()-1 )
    {
        return;
    }
    
    QModelIndex newPos = m_pModel->increaseSitePosition( id );
    m_listViewPtr->setCurrentIndex( newPos );
    m_pContextMenuView->setCurrentIndex( newPos );
    
    save();
}


void EWASitesListManager::save( bool bForce )

{
    if( !m_pModel->isModified() && !bForce )
    {
        return;
    }
    if( m_qstrFileName.isEmpty() )
    {
        QString fileName = QFileDialog::getSaveFileName( this, tr( "Input name of new list file:" ),
                                    m_qstrFileName,
                                    tr( "EWA Sites List (*.esl)\nAll Files (*.*)" ) );
        if( fileName.isEmpty() )
        {
            return;
        }

        m_qstrFileName = fileName;
    }

    clearIniFile();

    QSettings iniFile( m_qstrFileName, QSettings::IniFormat, 0 );
    
    iniFile.setValue( QLatin1String( "File" ), QLatin1String( "Enhanced Web Assistant - Sites List" ) );
    iniFile.setValue( QLatin1String( "Filev" ), QString( "%1.%2" ).arg( EWAApplication::settings()->getESLVersionMajor() ).arg( EWAApplication::settings()->getESLVersionMinor() ) );

    qApp->setOverrideCursor( Qt::WaitCursor );  
    this->setEnabled( false );
    
    int sitesCount = m_pModel->rowCount();
    iniFile.beginWriteArray( "Sites" );
    for( int sitesCounter = 0; sitesCounter < sitesCount; ++sitesCounter )
    {
        iniFile.setArrayIndex( sitesCounter );
        EWASiteHandle *pSite = getSiteByNumber( sitesCounter );
        iniFile.setValue( "sitefile", pSite->getFileName() );
        pSite->save();
    }
    iniFile.endArray();
    
    this->setEnabled( true );
    qApp->restoreOverrideCursor();
    
    m_pModel->setModified( false );
}

void EWASitesListManager::processESLVersion( QString& strVersion ) 
{
    QString strMajor = strVersion.left( strVersion.indexOf( "." ) );
    QString strMinor = strVersion.right( strVersion.indexOf( "." ) );

    bool bConverted = false;
    int iMajor = strMajor.toInt( &bConverted );
    if( !bConverted )
    {
        iMajor = 0;
    }

    int iMinor = strMinor.toInt( &bConverted );
    if( !bConverted )
    {
        iMinor = 0;
    }

    if( iMajor < EWAApplication::settings()->getESLVersionMajor() 
        || iMinor < EWAApplication::settings()->getESLVersionMinor() )
    {
        QMessageBox::information( this, tr( "EWA Sites List" ),
            tr( "The version of <b>%1</b><br>"
                "is too old.<br>"
                "Some features can be inaccesible.<br>"
                "It's recommended to readd this sites through Add Site Wizard").arg( m_qstrFileName ) );
    }
}


void EWASitesListManager::load( bool processVersion )
{
    int iSitesExists = m_pModel->rowCount();
    if( iSitesExists )
    {
        m_pModel->removeRows(0, m_pModel->rowCount(), QModelIndex() );
    }
    QSettings iniFile( m_qstrFileName, QSettings::IniFormat, 0 );

    QString strOur = iniFile.value( QLatin1String( "File" ), QLatin1String( "" ) ).toString();
    
    if( strOur.compare( "Enhanced Web Assistant - Sites List" ) )
        qWarning( "file %s don't contain \"File\" tage.", qPrintable( m_qstrFileName ) );
        
    QString strVersion = iniFile.value( QLatin1String( "Filev" ), QLatin1String( "1.0" ) ).toString();
    if( processVersion )
    {
        processESLVersion( strVersion );
    }
    
    qApp->setOverrideCursor( Qt::WaitCursor );
    //this->setEnabled( false );
    EWAApplication::getMainWindow()->updateUiWhileLoadingSitesList( false );
    
    if( !strVersion.compare( "1.0" ) || !strVersion.compare( "1.1" ) )
    {
        loadV10_11( iniFile );
        
        save();//-- update old file
    }
    else if( !strVersion.compare( "1.2" ) )
    {
        loadV12( iniFile );
    }
    
    slotSitesListCountInfo();
    setCacheCapacity( EWAApplication::getAppCacheSizeMax() );
    
    //this->setEnabled( true );
    EWAApplication::getMainWindow()->updateUiWhileLoadingSitesList( true );
    qApp->restoreOverrideCursor();
    
    m_pLastSite = 0;
    m_pPrevSite = 0;
    
    m_pModel->setModified( false );
}

void EWASitesListManager::loadV10_11( QSettings& list )
{
    int sitesCount = list.beginReadArray( "Sites" );
    for( int sitesCounter = 0; sitesCounter < sitesCount; ++sitesCounter )
    {
        qApp->processEvents();
        list.setArrayIndex( sitesCounter );
        EWASiteHandle *pSite = createSite();
        pSite->load( &list );
        
        pSite->save( pSite->getFileName(), true );//-- create single file for imported site
    }
    list.endArray();
}

void EWASitesListManager::loadV12( QSettings& list )
{
    int sitesCount = list.beginReadArray( "Sites" );
    for( int sitesCounter = 0; sitesCounter < sitesCount; ++sitesCounter )
    {
        EWAApplication::instance()->slotShowSplashMessage( tr( "Loading sites list... %1/%2" ).arg( sitesCounter+1 ).arg( sitesCount ) );
        
        qApp->processEvents();
        list.setArrayIndex( sitesCounter );
        
        EWASiteHandle *pSite = createSite();
        pSite->load( list.value( "sitefile", "" ).toString() );
        
    }
    list.endArray();
}
        
EWASiteHandle* EWASitesListManager::getSelectedSite() const
{
    EWASiteHandle *pSite = 0;

    int id = m_listViewPtr->currentIndex().row();
    if( id >= 0 )
    {
        pSite = m_pModel->getSiteByNumber( id );
    }

    return pSite;
}

void EWASitesListManager::slotModelItemActivated( const QModelIndex& index )
{
    int column = index.column();
    if( column == 3 )
    {
        showEditorDialogForSite( getSiteByNumber( index.row() ) );
        return;
    }
    emit signalContextMenuItemClicked( index );
}

void EWASitesListManager::showEditorDialogForSite( EWASiteHandle *pSite, bool bIsNew )
{
    m_pSiteEditorDlg->setSite( pSite, bIsNew );
    m_pSiteEditorDlg->show();
}

void EWASitesListManager::on_editSiteButton_clicked()
{
    if( ui.editSiteButton->isEnabled() )
    {
        EWASiteHandle *pSite = getSelectedSite();
        
        if( pSite )
        {
            showEditorDialogForSite( pSite );
        }
    }
}

void EWASitesListManager::setClearSite()
{
    m_pLastSite = 0;
    m_pPrevSite = 0;
}

int EWASitesListManager::getCurrentSiteIndex() const
{
    EWASiteHandle *pSite = getCurrentSite();
    if( pSite )
    {
        return m_pModel->getSiteNumber( pSite );
    }
    return -1;
}

EWASiteHandle* EWASitesListManager::getCurrentSite() const
{
    EWASiteHandle* pSite = 0;
    if( m_pLastSite )
    {
        pSite = m_pLastSite;
    }
    else
        pSite = m_pModel->getSiteByNumber( 0 );

    return pSite;
}

EWASiteHandle* EWASitesListManager::getNextSite() const
{
    EWASiteHandle *pSite = 0;

    int currentId = getCurrentSiteIndex();
    if( currentId != -1 )
    {
        if( currentId >= m_pModel->rowCount()-1 )
            currentId = -1;//-- next increment'll point to first site

        currentId++;

        pSite = m_pModel->getSiteByNumber( currentId );
    }

    return pSite;
}

EWASiteHandle* EWASitesListManager::getPrevSite() const
{
    EWASiteHandle *pSite = 0;

    if( m_pPrevSite )
    {
        pSite = m_pPrevSite;
    }
    else
    {
        int currentId = getCurrentSiteIndex();
        if( currentId != -1 )
        {
            if( currentId <= 0 )
                currentId = m_pModel->rowCount();//-- next decrement'll point to last site

            --currentId;

            pSite = m_pModel->getSiteByNumber( currentId );
        }
    }

    return pSite;
}

bool EWASitesListManager::isOneAtLeastSelected() const
{
    int sitesCount = m_pModel->rowCount();
    if( sitesCount <= 0 )
    {
        return false;
    }

    for( int i = 0; i<sitesCount; i++ )
    {
        EWASiteHandle *pSite = getSiteByNumber( i );
        if( pSite && pSite->isUsed() )
        {
            return true;
        }
    }

    return false;
}

bool EWASitesListManager::canStart()
{
    int sitesCount = m_pModel->rowCount();
    if( sitesCount <= 0 )
    {
        QMessageBox::warning( this, tr( "Sites List" ),
        tr( "Can't start downloading - sites list is empty.\nPlease, open correct list or create new one." ).arg( sitesCount ) );
        return false;
    }

    if( !isOneAtLeastSelected() )
    {
        QMessageBox::information( this, tr( "Sites List" ),
            tr( "You need to mark as \"<b>included</b>\" at least one site." ) );
        return false;
    }

    return true;
}

bool EWASitesListManager::startSites()
{
    //-- stopSites(); - not needed,
    //-- each site'll be autostopped
    
    if( m_bStarted )
    {
        if( !isPaused() )
        {
            pauseSites( false );
        }
        return false;
    }
    
    m_bStarted = false;
    if( canStart() )
    {
        pauseSites( false );
        EWASiteHandle *p1stSite = m_pModel->getSiteByNumber( 0 );
        int sitesCount = m_pModel->rowCount();
        if( p1stSite )
        {
            if( isDownloadTypeParallel() )
            {
                //-- start all site, except p1stSite (it'll be started last)
                for( int sitesCounter = 1; sitesCounter < sitesCount; sitesCounter++ )
                {
                    EWASiteHandle *pSite = m_pModel->getSiteByNumber( sitesCounter );
                    if( pSite && pSite->isUsed() )
                    {
                        pSite->slotDownload();
                        m_bStarted = true;
                    }
                }
            }
            else //-- serial downloading
            {
                if( !p1stSite->isUsed() )
                {
                    p1stSite = 0;
                    for( int sitesCounter = 1; sitesCounter < sitesCount; sitesCounter++ )
                    {
                        EWASiteHandle *pSite = m_pModel->getSiteByNumber( sitesCounter );
                        if( pSite && pSite->isUsed() )
                        {
                            p1stSite = pSite;
                            break;
                        }
                    }
                }
            }

            if( p1stSite && p1stSite->isUsed() )
            {
                p1stSite->slotDownload();
                m_bStarted = true;
            }

            m_pTimer->start( 1000 );

            setControllsEnabled( false );
        }
    }
    
    if( !m_bStarted && m_pTimer->isActive() )
    {
        m_pTimer->stop();
    }
    
    return m_bStarted;
}

void EWASitesListManager::stopSites( bool bCloseMsgs )
{
    if( m_bPaused )
    {
        pauseSites( false );
    }
    
    m_pTimer->stop();

    for( int i = 0; i< m_pModel->rowCount(); i++ )
    {

        EWASiteHandle *pSite = m_pModel->getSiteByNumber( i );
        Q_ASSERT( pSite );
        pSite->stopEx();
        if( bCloseMsgs )
        {
            pSite->getWidget()->close();
        }
    }

    setControllsEnabled( true );
    
    m_bStarted = false;
}

void EWASitesListManager::pauseSites( bool bPause )
{
    if( m_bPaused == bPause )
    {
        return;
    }
    
    m_bPaused = bPause;
    if( m_pTimer->isActive() && m_bPaused )
    {
        m_pTimer->stop();
    }
    else if( !m_pTimer->isActive() && !m_bPaused )
    {
        m_pTimer->start( 1000 );
    }
}

void EWASitesListManager::setControllsEnabled( bool on )
{
    ui.addSiteButton->setEnabled( on );
    ui.delSiteButton->setEnabled( on );
    ui.moveSiteUpButton->setEnabled( on );
    ui.moveSiteDownButton->setEnabled( on );

    updateButtonsTooltips();
}

void EWASitesListManager::updateButtonsTooltips()
{
    QString strDynTooltip = tr( "<br>Not available while Site List downloading" );
    if( m_bStarted )
    {
        strDynTooltip.append( tr( "<br>(Press \"Stop\" button for use this)" ) );
    }
    ui.addSiteButton->setToolTip( tr( "Add new site to list.%1" ).arg( strDynTooltip ) );
    ui.delSiteButton->setToolTip( tr( "Remove site from list.%1" ).arg( strDynTooltip ) );

    ui.moveSiteUpButton->setToolTip( tr( "Move site up.%1" ).arg( strDynTooltip ) );
    ui.moveSiteDownButton->setToolTip( tr( "Move site down.%1" ).arg( strDynTooltip ) );
}

void EWASitesListManager::setProxyForList(const QNetworkProxy &proxy)
{
    for( int i = 0; i< m_pModel->rowCount(); i++ )
    {
        EWASiteHandle *pSite = m_pModel->getSiteByNumber( i );
        Q_ASSERT( pSite );
        pSite->getEWANetworkManagerPtr()->setProxy( proxy );
    }
}

void EWASitesListManager::setDownloadTypeSerial()
{
    m_bDownloadParallel = false;
}

void EWASitesListManager::setDownloadTypeParallely()
{
    m_bDownloadParallel = true;
}

bool EWASitesListManager::isDownloadTypeParallel() const
{
    return m_bDownloadParallel;
}

EWASiteHandle* EWASitesListManager::getNextSite( EWASiteHandle* pSite ) const
{
    EWASiteHandle* pNextSite = 0;
    if( pSite )
    {
        int id = m_pModel->getSiteNumber( pSite );
        if( id == -1 )
        {
            return 0;
        }
        if( id == m_pModel->rowCount()-1 )
        {
            id = -1;
        }
        if( id < m_pModel->rowCount()-1 )
        {
            pNextSite = m_pModel->getSiteByNumber( ++id );
            while( !pNextSite->isUsed() && id < m_pModel->rowCount()-1 )
            {
                pNextSite = m_pModel->getSiteByNumber( ++id );
            }

            if( !pNextSite->isUsed() )
            {
                pNextSite = m_pModel->getSiteByNumber( 0 );
                if( !pNextSite->isUsed() )
                {
                    pNextSite = getNextSite( pNextSite );
                }
            }
        }
    }

    return pNextSite;
}

void EWASitesListManager::slotSiteDownloaded( const EWASitePtr *pSitePtr )
{
    if( !EWAApplication::getMainWindow()->isStarted() )
        return;

    EWASiteHandle *pSite = pSitePtr->m_pSite;

    if( !isDownloadTypeParallel() )
    {
        pSite->stop();
        pSite = getNextSite( pSite );
    }

    if( !pSite->isDownloadingActive() )
        pSite->start();
}

EWASiteHandle* EWASitesListManager::getSiteByNumber( int num ) const
{
    return m_pModel->getSiteByNumber( num );
}

int EWASitesListManager::getSiteNumber( EWASiteHandle* pSite ) const
{
    return m_pModel->getSiteNumber( pSite );
}

void EWASitesListManager::slotOneSecond()
{
    if( m_bPaused || !m_bStarted )
    {
        return;
    }
    
    for( int i = 0; i< m_pModel->rowCount(); i++ )
    {

        EWASiteHandle *pSite = m_pModel->getSiteByNumber( i );
        if( pSite )
        {
            bool bIsDownloadActive = !pSite->isDownloadingActive();
            bool bIsUsed = pSite->isUsed();
            bool bIsStarted = pSite->isStarted();
            bool bProcessOneSecond = bIsDownloadActive && bIsUsed && bIsStarted;// && bNotMaximized;
            if( bProcessOneSecond )
            {
                pSite->oneSecondTicked();
                
            }
        }
    }
}

EWASiteHandle* EWASitesListManager::getNearestSite() const
{
    if( !EWAApplication::getMainWindow()->isStarted() )
    {
        return 0;
    }

    EWASiteHandle *pSite = m_pModel->getSiteByNumber( 0 );
    if( !pSite )
    {
        return 0;
    }

    QMap<int, int> sites;
    QList<int> progresses;

    for( int i = 0; i<m_pModel->rowCount(); i++ )
    {
        EWASiteHandle *pCurrSite = m_pModel->getSiteByNumber( i );
        if( pCurrSite && pCurrSite->isStarted() )
        {
            sites[i] = pCurrSite->getCountdownCurrent();
            progresses.append( pCurrSite->getCountdownCurrent() );
        }
    }

    qSort( progresses );

    return m_pModel->getSiteByNumber( sites.key( progresses.first() ) );
}

void EWASitesListManager::closeSiteWidgets( int hideMode )
{
    for( int i = 0; i< m_pModel->rowCount(); i++ )
    {
        EWASiteHandle *pSite = m_pModel->getSiteByNumber( i );
        Q_ASSERT( pSite );
        pSite->closeWidgetByHideViewMode( hideMode );
    }
}

void EWASitesListManager::setCacheDir( const QString& dir  )
{
    for( int i = 0; i< m_pModel->rowCount(); i++ )
    {
        EWASiteHandle *pSite = m_pModel->getSiteByNumber( i );
        Q_ASSERT( pSite );
        QNetworkDiskCache* pCache = pSite->getEWANetworkManagerPtr()->cache();
        if( pCache->cacheDirectory().compare( dir ) )
        {
            pCache->setCacheDirectory( dir );
        }
    }
}

void EWASitesListManager::setCacheCapacity( qint64 value )
{
    for( int i = 0; i< m_pModel->rowCount(); i++ )
    {
        EWASiteHandle *pSite = m_pModel->getSiteByNumber( i );
        Q_ASSERT( pSite );
        QNetworkDiskCache* pCache = pSite->getEWANetworkManagerPtr()->cache();
        if( pCache->maximumCacheSize() != value )
        {
            pCache->setMaximumCacheSize( value );
        }
        pSite->getEWANetworkManagerPtr()->setCache( pCache );
    }
    
    int iValueBytes = value / 8;
    QWebSettings::setObjectCacheCapacities( iValueBytes, iValueBytes, value );
}

void EWASitesListManager::slotShowContextMenu( const QPoint& pnt )
{
    EWASiteHandle *pSite = getSelectedSite();
    showContextMenuForSite( pnt, pSite );
}

void EWASitesListManager::showContextMenuForSite( const QPoint& pnt, EWASiteHandle *pSite )
{    
    if( !pSite )
    {
        return;
    }
    
    int sitNumber = getSiteNumber( pSite );
    m_listViewPtr->selectRow( sitNumber );
    m_pContextMenuView->selectRow( sitNumber );

    QMenu contextMenu;
    QAction *pTitleAction = contextMenu.addAction( QIcon( pSite->getFavicoPixmap() ), tr( "%1:" ).arg( pSite->getVisibleMsgTitle() ) );
    pTitleAction->setEnabled( false );
    
    contextMenu.addSeparator();

    contextMenu.addAction( QIcon( QLatin1String( ":/images/configure.png" ) ), tr( "Edit" ), 
        ui.editSiteButton, SLOT( click() ) );
    
    if( ui.delSiteButton->isEnabled() )
    {
        contextMenu.addAction( QIcon( QLatin1String( ":/images/remove.png" ) ), tr( "Delete" ), ui.delSiteButton, SLOT( click() ) );
    }
    
    if( ui.addSiteButton->isEnabled() )
    {
        contextMenu.addAction( QIcon( QLatin1String( ":/images/add.png" ) ), tr( "Add new site" ), ui.addSiteButton, SLOT( click() ) );
    }
    
    contextMenu.addSeparator();

    QString action;
    QString iconFileName;
    if( pSite->isDownloadingActive() )
    {
        action = tr( "Stop" );
        iconFileName = QLatin1String( "stop" );
    }
    else
    {
        action = tr( "Start" );
        iconFileName = QLatin1String( "play" );
    }

    contextMenu.addAction( QIcon( tr( ":/images/%1.png" ).arg( iconFileName ) ), tr( "%1 Download" ).arg( action ), pSite, SLOT( slotActionDelegatActivated() ) );

    contextMenu.addSeparator();
    
    if( ui.moveSiteUpButton->isEnabled() )
    {
        contextMenu.addAction( QIcon( QLatin1String( ":/images/arrow_up.png" ) ), tr( "Move Up" ), ui.moveSiteUpButton, SLOT( click() ) );
    }
    if( ui.moveSiteDownButton->isEnabled() )
    {
        contextMenu.addAction( QIcon( QLatin1String( ":/images/arrow_down.png" ) ), tr( "Move Down" ), ui.moveSiteDownButton, SLOT( click() ) );
    }
    
    if( pSite->getWidget()->wasShown() )
    {
        contextMenu.addSeparator();
        contextMenu.addAction( QIcon( QLatin1String( ":/images/close.png" ) ), tr( "Close message widget" ), pSite->getWidget(), SLOT( close() ) );
    }

    contextMenu.exec( pnt );
}

void EWASitesListManager::slotSitesListCountInfo()
{
    int iTotal = m_pModel->rowCount();
    m_iSelectedSitesCount = 0;
    for( int i = 0; i<iTotal; i++ )
    {
        EWASiteHandle *pSite = m_pModel->getSiteByNumber( i );
        if( pSite->isUsed() )
        {
            m_iSelectedSitesCount++;
        }
    }
    
    ui.pSitesListGroupBox->setTitle( tr( "Auto-downloadable sites: %1 of %2" )
    .arg( m_iSelectedSitesCount ).arg( iTotal ) );
}

void EWASitesListManager::on_pUnSelectAllButton_clicked()
{
    int iTotal = m_pModel->rowCount();
    if( m_iSelectedSitesCount < 0 || iTotal <= 0 )
    {
        return;
    }
    
    int iCurrentUnselected = iTotal - m_iSelectedSitesCount;
    bool bSelected = m_iSelectedSitesCount
                     ? iCurrentUnselected > 0
                        ? iCurrentUnselected <= m_iSelectedSitesCount
                            :false
                         : true;
    
    for( int i = 0; i<iTotal; i++ )
    {
        EWASiteHandle *pSite = m_pModel->getSiteByNumber( i );
        pSite->setUsed( bSelected );
    }
}

QAbstractTableModel* EWASitesListManager::getModel() const {return m_pModel;}

int EWASitesListManager::getSitesCount() const {return m_pModel->rowCount();}

QString EWASitesListManager::getSitesFolder() const
{
    QFileInfo fInfo( getListFileName() );
    
    return QString( "%1/.%2/" ).arg( fInfo.absolutePath() ).arg( fInfo.baseName() );
}
