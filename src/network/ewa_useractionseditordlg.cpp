/*******************************************************************************
**
** file: ewa_useractionseditordlg.cpp
**
** class: EWAUserActionsEditorDialog
**
** description:
** Dialog for editing saved user's activity
**
** 25.10.2009
**
** ewauthor@indatray.com
**
*******************************************************************************/

#include "ewa_useractionseditordlg.h"
#include "ewa_sitehandle.h"

#include "ewa_networkaccessmanager.h"
#include "ewa_useractionsplayer.h"

#include "ui_ewa_pointeditordlg.h"

#include <QTableView>
#include <QStandardItemModel>
#include <QScrollBar>
#include <QInputDialog>

#define PTR2PAGE_ROLE Qt::UserRole+1
#define PTR2GEST_ROLE PTR2PAGE_ROLE+1

EWAUserActionsEditorDialog::EWAUserActionsEditorDialog( QWidget *pParent )
:QDialog( pParent  )
{
    ui.setupUi( this );
    m_sitePtr = 0;
    m_tableViewPtr = ui.pTableView;
    m_tableViewPtr->setSelectionBehavior( QAbstractItemView::SelectRows );
    m_tableViewPtr->setSelectionMode( QAbstractItemView::SingleSelection );

    m_pModel = 0;
    
    m_szIcons = QSize( 24, 24 );
    
    m_pixKeyDown = QPixmap( ":/images/kbd_pressed.png" ).scaled( m_szIcons, Qt::KeepAspectRatioByExpanding, Qt::SmoothTransformation );
    m_pixKeyUp = QPixmap( ":/images/kbd_normal.png" ).scaled( m_szIcons, Qt::KeepAspectRatioByExpanding, Qt::SmoothTransformation );
    m_pixMouseUp = QPixmap( ":/images/mouse_normal.png" ).scaled( m_szIcons, Qt::KeepAspectRatioByExpanding, Qt::SmoothTransformation );
    m_pixMouseDown = QPixmap( ":/images/mouse_pressed.png" ).scaled( m_szIcons, Qt::KeepAspectRatioByExpanding, Qt::SmoothTransformation );

    m_tableViewPtr->setHorizontalScrollBarPolicy( Qt::ScrollBarAlwaysOff );
    m_tableViewPtr->setVerticalScrollBarPolicy( Qt::ScrollBarAsNeeded );

    ui.pWebView->installEventFilter( this );
    
    m_bActionExecutionStarted = false;
}

EWAUserActionsEditorDialog::~EWAUserActionsEditorDialog()
{
    clearUserActivitys( m_pStoredPages );
    delete m_pStoredPages;
}

void EWAUserActionsEditorDialog::setSitePtr( EWASiteHandle* pSite )
{
    if( pSite )
    {
        m_sitePtr = pSite;
        m_pStoredPages = new QVector<EWAUserActionsCollection*>();
        createUserActivitysCopy( m_sitePtr->getEWANetworkManagerPtr()->getGesturesPlayer()->getPagesPtr(),
            m_pStoredPages );
        
        if( m_pStoredPages->first()->getActionsCount() )
        {
            QSize initSize = m_pStoredPages->first()->getActionsPtr()->at( 0 )->getWebViewSize();
            ui.pWebView->resize( initSize );
            ui.scrollArea->widget()->resize( initSize );
            ui.scrollArea->update();
        }
        
        connect( ui.pWebView, SIGNAL( loadStarted() ),
            this, SLOT( slotLoadStarted() ) );
        connect( ui.pWebView, SIGNAL( loadFinished(bool) ),
            this, SLOT( slotLoadFinished(bool) ) );
            
        setWindowTitle( tr( "User's Activities Editor - [%1]" ).arg( m_sitePtr->getVisibleMsgTitle() ) );
        
        initModel();
        
        ui.pWebView->setUrl( pSite->getUrl() );
    }
}
 
void EWAUserActionsEditorDialog::createUserActivitysCopy( const QVector<EWAUserActionsCollection*> *pSrc, 
                             QVector<EWAUserActionsCollection*> *pDst ) const
{
    if( !pSrc || !pDst )
    {
        return;
    }
    clearUserActivitys( pDst );

    for( int i = 0; i < pSrc->size(); i++ )
    {
        EWAUserActionsCollection* pActionsCollectionSrc = pSrc->at( i );
        EWAUserActionsCollection* pActionsCollectionDst = new EWAUserActionsCollection( pActionsCollectionSrc );
        pDst->append( pActionsCollectionDst );
    }
}

void EWAUserActionsEditorDialog::clearUserActivitys( QVector<EWAUserActionsCollection*> *pCollectionsVector ) const
{
    if( pCollectionsVector && pCollectionsVector->size() )
    {
        for( int i = pCollectionsVector->size()-1; i>=0; --i )
        {
            EWAUserActionsCollection* pActionsCollection = pCollectionsVector->at( i );
            pCollectionsVector->remove( i );
            pActionsCollection->clear();
            delete pActionsCollection;
        }
        pCollectionsVector->clear();
    }
}

void EWAUserActionsEditorDialog::accept()
{
    createUserActivitysCopy( m_pStoredPages, 
                             m_sitePtr->getEWANetworkManagerPtr()->getGesturesPlayer()->getPagesPtr() );

    invalidateDlg();
    QDialog::accept();
}
void EWAUserActionsEditorDialog::reject()
{
    invalidateDlg();
    QDialog::reject();
}

void EWAUserActionsEditorDialog::invalidateDlg()
{
}

void EWAUserActionsEditorDialog::slotLoadFinished(bool)
{
    enableControlls();
    enbleItem( ui.pTableView->currentIndex().row() );
}

void EWAUserActionsEditorDialog::slotLoadStarted()
{
    disableControlls();
}

void EWAUserActionsEditorDialog::enableControlls()
{
    setControllsEnabled( true );
}

void EWAUserActionsEditorDialog::disableControlls()
{
    setControllsEnabled( false );
}

void EWAUserActionsEditorDialog::setControllsEnabled( bool bEnabled )
{
    ui.pWebView->setEnabled( bEnabled );
    ui.pOkButton->setEnabled( bEnabled );
    ui.pExecButton->setEnabled( bEnabled );
    ui.pDeleteButton->setEnabled( bEnabled );
    ui.pChangeCoordsButton->setEnabled( bEnabled );
    ui.pNormalizeDelaysButton->setEnabled( bEnabled );
}

void EWAUserActionsEditorDialog::initModel()
{
    if( m_pModel )
    {
        disconnectModel();
        delete m_pModel;
    }
    
    int iPagesCount = m_pStoredPages->size();
    int iRowsCount = 0;
    
    for( int i = 0; i<iPagesCount; i++ )
    {
        iRowsCount += m_pStoredPages->at( i )->getActionsCount();
    }

    m_pModel = new QStandardItemModel( iRowsCount, 2, this );
    m_tableViewPtr->setColumnWidth( 0, m_szIcons.width() );
    m_tableViewPtr->setColumnWidth( 1, m_szIcons.width() );
    
    m_tableViewPtr->setModel( m_pModel );

    int iCurrentRow = 0;
    for( int iPagesCounter = 0; iPagesCounter < iPagesCount; iPagesCounter++ )
    {
        EWAUserActionsCollection *pPage = m_pStoredPages->at( iPagesCounter );
        
        for( int iActionsCounter = 0; iActionsCounter < pPage->getActionsCount(); iActionsCounter++ )
        {
            EWAUserAction *pAction = pPage->getActionsPtr()->at( iActionsCounter );
            
            inserActionIntoModel( iCurrentRow, pAction, iPagesCounter, iActionsCounter );
            
            iCurrentRow++;
        }
    }
    
    updateWidth();
    connectModel();
    
    enbleItem( 0 );
}


void EWAUserActionsEditorDialog::connectModel()
{
}

void EWAUserActionsEditorDialog::disconnectModel()
{
}

void EWAUserActionsEditorDialog::inserActionIntoModel( int iPos, const EWAUserAction *pAction, int iPageNum, int iActInPageNum )
{
    if( !m_pModel || !pAction )
    {
        return;
    }
    
    QString strActDelay = tr( "%1 sec" ).arg( 0.001*pAction->getTime() );
    QPixmap imgType;
    QString strActValue = pAction->getActionValue();
    
    QEvent *pActEvent = pAction->getEvent();
    
    switch( pActEvent->type()  )
    {
        case QEvent::KeyPress:
        {
            imgType = m_pixKeyDown;
            break;
        }
        case QEvent::KeyRelease:
        {
            imgType = m_pixKeyUp;
            break;
        }
        case QEvent::MouseButtonPress:
        {
            imgType = m_pixMouseDown;
            break;
        }
        case QEvent::MouseButtonRelease:
        {
            imgType = m_pixMouseUp;
            break;
        }
    default:
        {
            return;
        }
    }
    
    m_pModel->setData( m_pModel->index( iPos, 0 ), imgType, Qt::DecorationRole );
    
    m_pModel->setData( m_pModel->index( iPos, 0 ), iPageNum, PTR2PAGE_ROLE );
    m_pModel->setData( m_pModel->index( iPos, 0 ), iActInPageNum, PTR2GEST_ROLE );
    
    m_pModel->setData( m_pModel->index( iPos, 0 ), strActValue );
    m_pModel->setData( m_pModel->index( iPos, 1 ), strActDelay );
}


void EWAUserActionsEditorDialog::updateWidth()
{
    if( m_tableViewPtr && m_pModel )
    {
        m_tableViewPtr->resizeColumnsToContents();
        int iNewWidth = 30;
        if( m_tableViewPtr->verticalScrollBar()->isVisible() )
        {
            iNewWidth += m_tableViewPtr->verticalScrollBar()->width()*2;
        }

        for( int i = 0; i< m_pModel->columnCount()-1; i++ )
        {
            iNewWidth += m_tableViewPtr->columnSpan( 0, i )*2;
            iNewWidth += m_tableViewPtr->columnWidth( i );
        }

        m_tableViewPtr->resize( iNewWidth, m_tableViewPtr->height() );
    }
}

void EWAUserActionsEditorDialog::show()
{
    QDialog::show();
    updateWidth();
}

int EWAUserActionsEditorDialog::exec()
{
    show();
    return QDialog::exec();
}


bool EWAUserActionsEditorDialog::eventFilter( QObject *obj, QEvent *event )
{
    if( event->type() == QEvent::Wheel 
        || event->type() == QEvent::MouseMove )
    {
        updateCoordinatesInfo();
    }
    
    if( event->type() == QEvent::KeyPress 
        || event->type() == QEvent::KeyRelease 
        || event->type() == QEvent::MouseButtonPress
        || event->type() == QEvent::MouseButtonRelease )
    {
        m_bActionExecutionStarted = !m_bActionExecutionStarted;
        
        if( m_bActionExecutionStarted )
        {
            event->accept();
            return true;
        }
        else
        {
            event->ignore();
            return false;
        }
    }
    
    return QObject::eventFilter( obj, event );
}

void EWAUserActionsEditorDialog::normalizeStringsLengths( QString& strX, QString& strY ) const
{
    QString *strLenMin, *strLenMax;
    strLenMin = strLenMax = 0;
    if( strX.length() < strY.length() )
    {
        strLenMin = &strX;
        strLenMax = &strY;
    }
    else if( strY.length() < strX.length() )
    {
        strLenMin = &strY;
        strLenMax = &strX;
    }
    
    if( strLenMin && strLenMax )
    {
        while( strLenMin->length() != strLenMax->length() )
        {
            strLenMin->prepend( "0" );
        }
    }
}

void EWAUserActionsEditorDialog::updateCoordinatesInfo()
{
    QPoint pnt = ui.pWebView->mapFromGlobal( QCursor::pos() ) + ui.pWebView->page()->mainFrame()->scrollPosition();
    
    QString strXCoord = tr( "%1" ).arg( pnt.x() );
    QString strYCoord = tr( "%1" ).arg( pnt.y() );
    normalizeStringsLengths( strXCoord, strYCoord );
    
    ui.pCoordsLabel->setText( tr( "%1x%2" ).arg( strXCoord ).arg( strYCoord ) );
}

void EWAUserActionsEditorDialog::enbleItem( int iNum )
{
	if( iNum < 0 || iNum >= m_pModel->rowCount() )
		return;

    ui.pChangeCoordsButton->setEnabled( false );
    for( int i = 0; i<m_pModel->rowCount(); i++ )
    {
        for( int j = 0; j<m_pModel->columnCount(); j++ )
        {
            m_pModel->item( i, j )->setEnabled( i == iNum );
        }
    }
    
    for( int j = 0; j<m_pModel->columnCount(); j++ )
    {
        m_pModel->item( iNum, j )->setEnabled( true );
    }
    QModelIndex index = m_pModel->index( iNum, 0 );
    if( index.isValid() && index != ui.pTableView->currentIndex() )
    {
        ui.pTableView->setCurrentIndex( index );
        ui.pTableView->scrollTo( index );
    }
    
    EWAUserAction *pAction = getCurrentAction();
    if( pAction && pAction->isMouseEvent() )
    {
        ui.pChangeCoordsButton->setEnabled( true );
    }
}

void on_pChangeCoordsButton_clicked();

void EWAUserActionsEditorDialog::execute()
{
    m_bActionExecutionStarted = true;
    
    EWAUserAction *pAction = getCurrentAction();
    if( pAction )
    {
        QSize dstSz = pAction->getWebViewSize();
        if( ui.pWebView->page()->preferredContentsSize() != dstSz )
        {
            ui.pWebView->page()->setPreferredContentsSize( dstSz );
        }
        
        pAction->execute( ui.pWebView );
        enbleItem( ui.pTableView->currentIndex().row() + 1 );
    }
}

void EWAUserActionsEditorDialog::on_pExecButton_clicked()
{
    execute();
}

void EWAUserActionsEditorDialog::on_pNormalizeDelaysButton_clicked()
{
    bool ok = false;
    double dMinDelay = 0.001*(double)EWAUserActionsProcessor::getMinActDelay();
    double dCurrTime = dMinDelay;
    double dMaxDelay = 3.;
    EWAUserAction *pAction = getCurrentAction();
    if( pAction )
    {
        dCurrTime = 0.001*pAction->getTime();
    }
    
    double dNewPeriod = QInputDialog::getDouble( 
    this, 
    tr( "Common delay" ), 
    tr( "Input new value [%1...%2]:" ).arg( dMinDelay ).arg( dMaxDelay ),
    qMax( dCurrTime, dMinDelay ),
    dMinDelay,
    dMaxDelay, 
    3, 
    &ok );
    if( ok && m_sitePtr )
    {
        m_sitePtr->getEWANetworkManagerPtr()->setCommonDelayForActions( m_pStoredPages, dNewPeriod*1000 );
        
        QModelIndex currModelId = ui.pTableView->currentIndex();
        
        initModel();
        
        enbleItem( currModelId.row() );
    }
}

EWAUserAction* EWAUserActionsEditorDialog::getCurrentAction()
{
    QModelIndex currModelId = ui.pTableView->currentIndex();
    
    if( currModelId.isValid() )
    {
        QStandardItem *pItem = m_pModel->item( currModelId.row() );
        if( pItem )
        {
            int iPageNum = pItem->data( PTR2PAGE_ROLE ).toInt();
            EWAUserActionsCollection* pCollection = m_pStoredPages->at( iPageNum );
            if( pCollection )
            {
                int iActNum = pItem->data( PTR2GEST_ROLE ).toInt();
                if( iActNum >= 0 && iActNum < pCollection->getActionsCount() )
                {
                    return pCollection->getActionsPtr()->at( iActNum );
                }
            }
        }
    }
    
    return 0;
}

void EWAUserActionsEditorDialog::on_pDeleteButton_clicked()
{
    QModelIndex currModelId = ui.pTableView->currentIndex();
    
    if( currModelId.isValid() )
    {
        QStandardItem *pItem = m_pModel->item( currModelId.row() );
        if( pItem )
        {
            int iPageNum = pItem->data( PTR2PAGE_ROLE ).toInt();
            EWAUserActionsCollection* pCollection = m_pStoredPages->at( iPageNum );
            if( pCollection )
            {
                int iActNum = pItem->data( PTR2GEST_ROLE ).toInt();
                if( iActNum >= 0 && iActNum < pCollection->getActionsCount() )
                {
                    EWAUserAction *pAction = pCollection->getActionsPtr()->at( iActNum );
                    if( pAction )
                    {
                        pCollection->getActionsPtr()->remove( iActNum );
                        delete pAction;
                        
                        initModel();
                        
                        enbleItem( currModelId.row() );
                    }
                }
            }
        }
    }
}

void EWAUserActionsEditorDialog::on_pChangeCoordsButton_clicked()
{
    EWAUserAction *pAction = getCurrentAction();
    if( !pAction || !pAction->isMouseEvent() )
    {
        return;
    }
    
    QDialog *pDlg = new QDialog( this );
    Ui::EWAPointEditorDlgUi dlgUi;
    dlgUi.setupUi( pDlg );
    
    QPoint pnt = pAction->getClickCoords();
    dlgUi.pXCoordSpinBox->setValue( pnt.x() );
    dlgUi.pYCoordSpinBox->setValue( pnt.y() );
    
    if( pDlg->exec() == QDialog::Accepted )
    {
        QModelIndex currModelId = ui.pTableView->currentIndex();
        
        pAction->setClickCoords( QPoint( dlgUi.pXCoordSpinBox->value(), dlgUi.pYCoordSpinBox->value() ) );
        initModel();
        
        enbleItem( currModelId.row() );
    }
    
    delete pDlg;
}