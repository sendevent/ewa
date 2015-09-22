/*******************************************************************************
**
** file: ewa_siteeditordlg.
**
** class: EWASiteEditorDlg
**
** description:
** Dialog with sites editing functionality.
**
** 09.02.2009
**
** sendevent@gmail.com
**
*******************************************************************************/

#include "ewa_siteeditordlg.h"

#include "ewa_sitehandle.h"
#include "ewa_siteslistmanager.h"
#include "ewa_networkaccessmanager.h"
#include "ewa_trayiconmanager.h"
#include "ewa_webview.h"
#include "ewa_rxdlg.h"
#include "ewa_useractionseditordlg.h"
#include "ewa_htmlselector.h"
#include "ewa_faderwidget.h"
#include "ewa_xmessagebox.h"
#include "ewa_parsingrule.h"
#include "ewa_simplesiteproperties.h"
#include "ewa_splashscreen.h"

#include <QMessageBox>
#include <QColorDialog>
#include <QSettings>
#include <QItemSelectionModel>
#include <QStandardItemModel>
#include <QToolBar>

const QString strLimitsMsg = QObject::tr( "While site options dialog is open, site's widget is in \"preview\" mode. It means that some functionality, like as history navigation, widget closing, etc. is temporary disabled." );

EWASiteEditorDlg::EWASiteEditorDlg( QWidget *parent )
:QDialog( parent ), m_bExtendedOptions( true )
{
    ui.setupUi( this );
    
    m_pSrcSettings = new QSettings( QSettings::IniFormat, 
        QSettings::UserScope, 
        EWAApplication::organizationName(),
        EWAApplication::applicationName(), 
        this );
    
    m_pErrorMessageDialog = new EWAXMessageBox( this, true );
    m_pErrorMessageDialog->setText( EWAApplication::translate( this, ::strLimitsMsg.toUtf8().data() ) );
    connect( m_pErrorMessageDialog, SIGNAL( signalShowAgainChanged(bool) ),
        EWAApplication::settings()->m_pGroupApp, SLOT( setEditedXMessageWarning(bool) ) );
    
    connectStackWidgets();
    
    initToolBar();
    
    initSimplePropertiesWidget();
    
    setWindowFlags( Qt::Dialog|Qt::CustomizeWindowHint|Qt::WindowCloseButtonHint );
    m_editedSitePtr = 0;
    ui.timeValueSpinBox->setMinimum( 1 );
    m_simplePropertiesPtr->timeValueSpinBox()->setMinimum( 1 );

    m_rulesTableViewPtr = ui.pRulesTableView;
    
    model = 0;
    initModel();
    connectModel();

    connect( ui.addRuleButton, SIGNAL( clicked() ), SLOT( slotAddRule() ) );
    connect( ui.delRuleButton, SIGNAL( clicked() ), SLOT( slotDeleteRule() ) );
    connect( ui.moveRuleUpButton, SIGNAL( clicked() ), SLOT( slotMoveUpRule() ) );
    connect( ui.moveRuleDownButton, SIGNAL( clicked() ), SLOT( slotMoveDownRule() ) );
    connect( ui.srcTextEdit, SIGNAL( textChanged() ), this, SLOT( slotSrcChanged() ) );

    connect( ui.okButton, SIGNAL( clicked() ), this, SLOT( accept() ) );

    connect( ui.pRulesTableView->selectionModel(),
        SIGNAL( currentChanged(const QModelIndex&,const QModelIndex&) ), this,
        SLOT( slotCurrentRowChanged(const QModelIndex&,const QModelIndex&) ) );
    connect( m_rulesTableViewPtr, SIGNAL( doubleClicked(const QModelIndex&) ),
        this, SLOT( slotEditRule(const QModelIndex&) ) );
        
    normalizeListviewsWidths();
    on_pChangeDlgButton_clicked();
    updateUi();

    m_pHtmlSelect = 0;
}

 EWASiteEditorDlg::~EWASiteEditorDlg()
{
    if( model )
    {
        delete model;
    }
}

void EWASiteEditorDlg::setSite( EWASiteHandle *pSite, bool bNewSite )
{
    invalidateDlg();
    
    if( !pSite )
    {
        return;
    }
    
    m_editedSitePtr = pSite;
    
    //-- save values for restore on cancel:
    m_editedSitePtr->save( m_pSrcSettings, true );
    
    if( !m_editedSitePtr->getWidget()->isViewNormal() )
    {
        m_editedSitePtr->getWidget()->slotNeedShowNormal();
    }
    m_editedSitePtr->getWidget()->setAsPreview( true );
    
    //-- if site is new - widget'll be shown after closing this dlg
    if( bNewSite )
    {
        m_editedSitePtr->save( m_editedSitePtr->getFileName(), true );
        EWAApplication::getSitesManager()->save();
        m_bOriginalMsgVisible = true;
#ifndef Q_OS_WIN
        m_editedSitePtr->getWidget()->show();
#endif //--  Q_OS_WIN
    }
    else
    {
        if( m_editedSitePtr->extendedMessages() )
        {
            if( m_editedSitePtr->replayTypeIsHttp() )
            {
                m_bOriginalMsgVisible = m_editedSitePtr->getWidget()->isVisible();
            }
            else
            {
                m_bOriginalMsgVisible = m_editedSitePtr->getWidget()->wasShown();
            }
        }
    }
    m_iDnldCount = m_editedSitePtr->getDownloadCounter();

    ui.ewaSysTrayMsgSettings->setSitePtr( m_editedSitePtr );

    int period = 0;
    unsigned int units = m_editedSitePtr->getPeriodAndUnits( period );
    ui.timeUnitsComboBox2->setCurrentIndex( units );
    m_simplePropertiesPtr->timeUnitsComboBox()->setCurrentIndex( units );

    switch( units )
    {
        case 0:
        case 1:
        {
            ui.timeValueSpinBox->setMaximum( 59 );
            break;
        }
        case 2:
        {
            ui.timeValueSpinBox->setMaximum( 24 );
            break;
        }
    }

    ui.timeValueSpinBox->setValue( period );

    ui.urlLineEdit->setText( m_editedSitePtr->getUrl() );

    ui.durationSpinBox->setValue( m_editedSitePtr->getMsgTTL() );
    m_simplePropertiesPtr->durationSpinBox()->setValue( m_editedSitePtr->getMsgTTL() );
    
    ui.titleLineEdit->setText( m_editedSitePtr->getMsgTitle() );
    m_simplePropertiesPtr->titleLineEdit()->setText( m_editedSitePtr->getMsgTitle() );
    
    QString strTitleLEToolTip = tr( "Message Title<br><b>%url%</b> will be replaced by<br>%1" ).arg( pSite->getUrlStrLimit32() );
    ui.titleLineEdit->setToolTip( strTitleLEToolTip );
    m_simplePropertiesPtr->titleLineEdit()->setToolTip( strTitleLEToolTip );
    
    bool bIsHttp = pSite->replayTypeIsHttp();
    bool bIsGestures = !bIsHttp;
    ui.replayStyleIsHttpButton->setChecked( bIsHttp );
    ui.replayStyleIsNotHttpButton->setChecked( bIsGestures );
    
    setWindowTitle( tr( "Settings - [%1]" ).arg( m_editedSitePtr->getVisibleMsgTitle() ) );

    EWAShowPolicy* pShowPolicy = m_editedSitePtr->getShowPolicy();
    bool bDbg = pShowPolicy->isUsed();
    ui.matchPopupGroupBox->setChecked( bDbg );
    m_simplePropertiesPtr->changesCheckBox()->setChecked( bDbg );
    ui.matchedComboBox->setCurrentIndex( ( int )pShowPolicy->showIfChanged() );
    ui.minCheckBox->setChecked( pShowPolicy->getRegExp().isMinimal() );
    ui.csCheckBox->setChecked( pShowPolicy->getRegExp().caseSensitivity() == Qt::CaseSensitive );
    ui.patternLineEdit->setText( pShowPolicy->getRegExp().pattern() );
    
    if( m_editedSitePtr->extendedMessages() )
    {
        ui.msgSysExRadioButton->setChecked( true );
        m_simplePropertiesPtr->msgSysExRadioButton()->setChecked( true );
    }
    else if( EWAApplication::getTrayIconManager()->supportBaloons() )
    {
        ui.msgSysRadioButton->setChecked( true );
        m_simplePropertiesPtr->msgSysRadioButton()->setChecked( true );
    }
    
    if( !EWAApplication::getTrayIconManager()->supportBaloons() )
    {
        disableBaloons();
    }

    updateUi();
    updateModelView();

    connect( m_editedSitePtr, SIGNAL( signalDownloaded(const EWASitePtr*) ),
        this, SLOT( slotPageSourceRecived(const EWASitePtr*) ) );
    
    QString ua = m_editedSitePtr->getWebView()->page()->getUserAgent();
    ui.userAgentComboBox->slotSetUserAgent( ua );
    
    QString pageSource = m_editedSitePtr->getBodyAsHtml();
    ui.srcTextEdit->setPlainText( pageSource );
    ui.resulttBrowser->setPlainText( pageSource );

    ui.m_pSimpleProperties->setSite( m_editedSitePtr );
    //-- This should be done at the and of all!
    ui.ewaXMsgSettings->setSitePtr( m_editedSitePtr );
}



void EWASiteEditorDlg::updateUi()
{
    bool bSiteExist = ( bool )m_editedSitePtr;
    bool bHaveSiteSource = false;

    bool bUseShowPolicy = false;
    int iShowMatched = 0;
    bool bRxMin = false;
    bool bRxCs = false;
    QString pattern = "";

    if( bSiteExist )
    {
        bHaveSiteSource = ( bool )ui.srcTextEdit->toPlainText().compare( EWAApplication::instance()->getBlankPageSource() );

        EWAShowPolicy *pShowPolicy = m_editedSitePtr->getShowPolicy();
        bUseShowPolicy = pShowPolicy->isUsed();
        iShowMatched = ( int )pShowPolicy->showIfChanged();
        bRxMin = pShowPolicy->getRegExp().isMinimal();
        bRxCs = ( pShowPolicy->getRegExp().caseSensitivity() == Qt::CaseSensitive );
        pattern = pShowPolicy->getRegExp().pattern();
    }

    ui.addRuleButton->setEnabled( bHaveSiteSource );
    ui.delRuleButton->setEnabled( model->rowCount() );

    int selectedRow = m_rulesTableViewPtr->currentIndex().row();
    bool bRuleSelected = ( selectedRow >= 0 && selectedRow < model->rowCount() );
    ui.moveRuleDownButton->setEnabled( bRuleSelected && selectedRow < model->rowCount()-1 );
    ui.moveRuleUpButton->setEnabled( bRuleSelected && selectedRow );

    ui.runButton->setEnabled( bRuleSelected && bHaveSiteSource );

    ui.matchPopupGroupBox->setChecked( bUseShowPolicy );
    ui.matchedComboBox->setCurrentIndex( iShowMatched );
    ui.minCheckBox->setChecked( bRxMin );
    ui.csCheckBox->setChecked( bRxCs );
    ui.patternLineEdit->setText( pattern );
}

void EWASiteEditorDlg::updateDownloadPeriodSpinboxesTooltip(  int id )
{
    int iMin = ui.timeValueSpinBox->minimum();
    int iMax = ui.timeValueSpinBox->maximum();

    QString strUnits;
    
    switch( id )
    {
        case 0:
        {//-- seconds:
            strUnits = tr( "seconds" );
            iMin = 10;
            iMax = 60*60*24;
            break;
        }
        case 1:
        {//-- minutes:
            strUnits = tr( "minutes" );
            iMin = 1;
            iMax = 60*24;
            break;
        }
        case 2:
        {//-- hours:
            strUnits = tr( "hours" );
            iMin = 1;
            iMax = 24;
            break;
        }
    }

    ui.timeValueSpinBox->setRange( iMin, iMax );
    m_simplePropertiesPtr->timeValueSpinBox()->setRange( iMin, iMax );
    QString strTooltip = tr( "Download period (%1 - %2 %3)" ).arg( iMin ).arg( iMax ).arg( strUnits );
    ui.timeValueSpinBox->setToolTip( strTooltip );
    m_simplePropertiesPtr->timeValueSpinBox()->setToolTip( strTooltip );
}

void EWASiteEditorDlg::updateSitePeriodTime( int periodUnits )
{
    int periodCount = ui.timeValueSpinBox->value();
    
    if( m_editedSitePtr )
    {
        int period;
        unsigned int count = m_editedSitePtr->getPeriodAndUnits( period );
        if( period != periodUnits || periodCount != (int)count )
        {
            m_editedSitePtr->setPeriodAndUnits( periodCount, periodUnits );
            updateDownloadPeriodSpinboxesTooltip( periodUnits );
        }
    }
}

void EWASiteEditorDlg::on_timeUnitsComboBox_activated( int id )
{
    updateSitePeriodTime( id );
}

void EWASiteEditorDlg::on_timeUnitsComboBox2_activated( int id )
{
    updateSitePeriodTime( id );
}

void EWASiteEditorDlg::on_timeValueSpinBox_valueChanged( int time )
{
    int periodUnits = ui.timeUnitsComboBox2->currentIndex();
    if( m_editedSitePtr )
    {
        int period;
        unsigned int count = m_editedSitePtr->getPeriodAndUnits( period );
        if( period != periodUnits || time != (int)count )
        {
            m_editedSitePtr->setPeriodAndUnits( time, periodUnits );
            updateDownloadPeriodSpinboxesTooltip( periodUnits );
        }
    }
}

void EWASiteEditorDlg::initModel()
{
    int iRowsCount = 0;

    if( model )
    {
        model->removeRows ( 0, model->rowCount() );
    }
    else
    {
        if( m_editedSitePtr && m_editedSitePtr->getRulesCollection()->getPtr2Rules() )
        {
            iRowsCount = m_editedSitePtr->getRulesCollection()->getPtr2Rules()->size();
        }

        model = new QStandardItemModel( iRowsCount, 4, this );
        model->setHeaderData( 0, Qt::Horizontal, tr( "Search for" ) );
        model->setHeaderData( 1, Qt::Horizontal, tr( "Replace with" ) );
        model->setHeaderData( 2, Qt::Horizontal, tr( "Casesensetive" ) );
        model->setHeaderData( 3, Qt::Horizontal, tr( "Minimal" ) );
        m_rulesTableViewPtr->setModel( model );
    }

    for( int i = 0; i<iRowsCount; i++ )
    {
        EWAParsingRule *pRule = m_editedSitePtr->getRulesCollection()->getPtr2Rules()->at( i );

        updateModelItem( i, pRule );
    }
}

void EWASiteEditorDlg::slotAddRule()
{
    EWARegExpDialog *pRXDlg = new EWARegExpDialog( 
        ui.resulttBrowser->toPlainText()
        ,true
        ,true
        ,"(.*)"
        ,"%"
        ,0
        ,false );
    if( pRXDlg->exec() == QDialog::Accepted )
    {
        EWAParsingRule createdRule = pRXDlg->getRule();
        EWAParsingRule *pRule = m_editedSitePtr->getRulesCollection()->addRule( &createdRule );
        pRule->cloneDataFrom( pRXDlg->getRule() );
        
        int rowIndex = model->rowCount();
        disconnectModel();
        //-- add new item 2 model,
        if( model->insertRow( rowIndex ) )
        {
            model->setData( model->index( rowIndex, 0 ), pRule->getPattern() );
            model->setData( model->index( rowIndex, 1 ), pRule->getDestination() );
            model->setData( model->index( rowIndex, 2 ), pRule->isCaseSensitive() );
            model->setData( model->index( rowIndex, 3 ), pRule->isMinimal() );

            //-- navigate to created item - it'll call
            m_rulesTableViewPtr->setCurrentIndex( model->index( rowIndex, 0 ) );
        }
        connectModel();
    }

    delete pRXDlg;
    updateUi();
}

bool EWASiteEditorDlg::isSiteRulesCorrect()
{
    for( int i = 0; i<model->rowCount(); i++ )
    {
        if( !slotUpdate( i ) )
        {
            return false;
        }
    }

    return true;
}

/**
** Resize edited item to inputed text width.
*/
void EWASiteEditorDlg::slotDataChanged( const QModelIndex & topLeft, const QModelIndex & bottomRight )
{
    Q_UNUSED( bottomRight );

    int row = topLeft.row();
    int column = topLeft.column();

    if( row >= m_editedSitePtr->getRulesCollection()->getPtr2Rules()->size() ) return;

    EWAParsingRule *pRule = m_editedSitePtr->getRulesCollection()->getPtr2Rules()->at( row );

    switch( column )
    {
        case 0:
        {
            pRule->setPattern( topLeft.data().toString() );
            break;
        }
        case 1:
        {
            pRule->setDestination( topLeft.data().toString() );
            break;
        }
        case 2:
        {
            pRule->setCaseSensitive( topLeft.data().toBool() );
            break;
        }
        case 3:
        {
            pRule->setMinimal( topLeft.data().toBool() );
            break;
        }

    }
}


void EWASiteEditorDlg::slotDeleteRule()
{
    if( model->rowCount() <= 0 ) return;

    int currentRow = m_rulesTableViewPtr->currentIndex().row();
    if( currentRow >= 0 )
    {
        model->removeRow( currentRow );
        if( currentRow<m_editedSitePtr->getRulesCollection()->getPtr2Rules()->count()
            && currentRow >= 0 )
        {
            m_editedSitePtr->getRulesCollection()->getPtr2Rules()->remove( currentRow );
        }
    }

    updateUi();
}

bool EWASiteEditorDlg::moveRow( int delta )
{
    if( !delta || model->rowCount() <= 1  ) return false;

    delta = delta/abs( delta );

    int currentRow = m_rulesTableViewPtr->currentIndex().row();

    if( delta<0 )
    {
         if( currentRow < 1 ) return false;
    }
    else
    {
        if( currentRow >= model->rowCount()-1 || currentRow < 0 )
        return false;
    }
    
    disconnectModel();
    
    int neighbourRow = currentRow + delta;

    EWAParsingRule *pCurrentRule, *pNeighbourRule;
    pCurrentRule = m_editedSitePtr->getRulesCollection()->getPtr2Rules()->at( currentRow );
    pNeighbourRule = m_editedSitePtr->getRulesCollection()->getPtr2Rules()->at( neighbourRow );

    pCurrentRule->setPattern( model->data( model->index( currentRow, 0 ) ).toString() );
    pCurrentRule->setDestination( model->data( model->index( currentRow, 1 ) ).toString() );
    pCurrentRule->setCaseSensitive( model->data( model->index( currentRow, 2 ) ).toBool() );
    pCurrentRule->setMinimal( model->data( model->index( currentRow, 3 ) ).toBool() );

    pNeighbourRule->setPattern( model->data( model->index( neighbourRow, 0 ) ).toString() );
    pNeighbourRule->setDestination( model->data( model->index( neighbourRow, 1 ) ).toString() );
    pNeighbourRule->setCaseSensitive( model->data( model->index( neighbourRow, 2 ) ).toBool() );
    pNeighbourRule->setMinimal( model->data( model->index( neighbourRow, 3 ) ).toBool() );

    model->setData( model->index( neighbourRow, 0 ), pCurrentRule->getPattern() );
    model->setData( model->index( neighbourRow, 1 ), pCurrentRule->getDestination() );
    model->setData( model->index( neighbourRow, 2 ), pCurrentRule->isCaseSensitive() );
    model->setData( model->index( neighbourRow, 3 ), pCurrentRule->isMinimal() );

    model->setData( model->index( currentRow, 0 ), pNeighbourRule->getPattern() );
    model->setData( model->index( currentRow, 1 ), pNeighbourRule->getDestination() );
    model->setData( model->index( currentRow, 2 ), pNeighbourRule->isCaseSensitive() );
    model->setData( model->index( currentRow, 3 ), pNeighbourRule->isMinimal() );

    m_editedSitePtr->getRulesCollection()->getPtr2Rules()->replace( neighbourRow, pCurrentRule );
    m_editedSitePtr->getRulesCollection()->getPtr2Rules()->replace( currentRow, pNeighbourRule );
    
    m_rulesTableViewPtr->setCurrentIndex( model->index( neighbourRow, 0 ) );

    m_rulesTableViewPtr->update();
    
    disconnectModel();

    return true;
}

//-- Move current ( selected ) button.
void EWASiteEditorDlg::slotMoveUpRule()
{
    moveRow( -1 );
    updateUi();
}

//-- Move current ( selected ) button.
void EWASiteEditorDlg::slotMoveDownRule()
{
    moveRow();
    updateUi();
}

//-- Work with data files:
bool EWASiteEditorDlg::updateModelView()
{
    if( !m_editedSitePtr->getRulesCollection()->getPtr2Rules() )
        return false;

    model->removeRows( 0, model->rowCount() );

    disconnectModel();

    int rulesCount = m_editedSitePtr->getRulesCollection()->getPtr2Rules()->count();
    for( int i = 0; i < rulesCount; ++i )
    {
        EWAParsingRule rule = m_editedSitePtr->getRulesCollection()->getPtr2Rules()->at( i );

        model->insertRow( i );
        model->setData( model->index( i, 0 ), rule.getPattern() );
        model->setData( model->index( i, 1 ), rule.getDestination() );
        model->setData( model->index( i, 2 ), rule.isCaseSensitive() );
        model->setData( model->index( i, 3 ), rule.isMinimal() );
    }

    connectModel();
    m_rulesTableViewPtr->setCurrentIndex( model->index( 0, 0 ) );
    updateUi();
    return true;
}

void EWASiteEditorDlg::slotItemDataChanged( QStandardItem *item )
{
    if( item )
    {
        slotDataChanged( item->index(), item->index() );
        slotUpdate( item->row() );
    }
}

void EWASiteEditorDlg::connectModel()
{
    connect( model, SIGNAL( dataChanged(const QModelIndex&,const QModelIndex&) ),
        this, SLOT( slotDataChanged(const QModelIndex&,const QModelIndex&) ) );

    connect( model, SIGNAL( itemChanged(QStandardItem*) ),
        this, SLOT( slotItemDataChanged(QStandardItem*) ) );
}

void EWASiteEditorDlg::disconnectModel()
{
    disconnect( model, SIGNAL( dataChanged (const QModelIndex&,const QModelIndex&) ),
        this, SLOT( slotDataChanged(const QModelIndex&,const QModelIndex&) ) );

    disconnect( model, SIGNAL( itemChanged(QStandardItem*) ),
        this, SLOT( slotItemDataChanged(QStandardItem*) ) );
}

bool EWASiteEditorDlg::slotSelectRule( int num )
{
    if( model->rowCount() <= 0 )
        return false;

    if( num < 0 )
        num = 0;

    if( num >= model->rowCount() )
        num = model->rowCount()-1;

    m_rulesTableViewPtr->setCurrentIndex( model->index( num, 0 ) );
    updateUi();
    return true;
}

void EWASiteEditorDlg::slotPageSourceRecived( const EWASitePtr *pSite )
{
    Q_UNUSED( pSite );
    if( m_editedSitePtr )
    {
        ui.srcTextEdit->setPlainText( m_editedSitePtr->getBodyAsHtml() );

        update();
    }
    updateUi();
}

void EWASiteEditorDlg::on_downloadButton_clicked()
{
    if( m_editedSitePtr )
        m_editedSitePtr->slotDownload();
}

void EWASiteEditorDlg::slotSrcChanged()
{
    QString srcText = ui.srcTextEdit->toPlainText();
    ui.resulttBrowser->setPlainText( srcText );
    updateUi();
}

bool EWASiteEditorDlg::slotUpdate( int currentRuleNum )
{
    ui.resulttBrowser->clear();

    if( currentRuleNum < 0
	|| currentRuleNum > m_editedSitePtr->getRulesCollection()->getPtr2Rules()->count()
	|| ui.srcTextEdit->toPlainText().isEmpty() )
        return false;

    EWAParsingRule *pCurrentRule = m_editedSitePtr->getRulesCollection()->getPtr2Rules()->at( currentRuleNum );
    QRegExp rx = pCurrentRule->makeRegExp();

    if( !rx.isValid() )
    {
        QMessageBox::warning( this, tr( "Warning!" ),
        tr( "Syntax error in RegEx num %1:\n"
        "%2\n"
        "Please correct it to continue." ).arg( currentRuleNum+1 ).arg( rx.pattern() ) );
        return false;
    }

    setUpdatesEnabled( false );
    QString srcText = ui.srcTextEdit->toPlainText();
    QString dstText = srcText;
    bool processed = pCurrentRule->processString( dstText );
    if( !processed )
    {
        ui.resulttBrowser->clear();
    }
    else
    {
        ui.resulttBrowser->setPlainText( dstText );
    }
    setUpdatesEnabled( true );

    return true;
}

bool EWASiteEditorDlg::isTrackChangesRxOk()
{
    bool res = true;
    if( ui.matchPopupGroupBox->isChecked() )
    {
        QString rxPattern = ui.patternLineEdit->text();
        if( rxPattern.isEmpty() )
        {
            res = false;
        }
        else
        {
            QRegExp testRX( rxPattern );
            testRX.setMinimal( ui.minCheckBox->isChecked() );
            testRX.setCaseSensitivity( ui.csCheckBox->isChecked() ? Qt::CaseSensitive : Qt::CaseInsensitive );

            res = testRX.isValid();
        }
    }

    if( !res )
    {
        QMessageBox::information( this, tr( "Site Options" ),
            tr( "Regular expression, used for tracking changes,<br>"
                "is incorrect. You need to edit pattern or switch off<br>"
                "\"<b>Track Changes</b>\" checkbox on \"<b>Access</b>\" tab." ) );
    }
    return res;
}

void EWASiteEditorDlg::accept()
{
    if( !isTrackChangesRxOk() || !isSiteRulesCorrect() )
    {
        return;
    }
    
    if( m_editedSitePtr->isModifyed() )
        m_editedSitePtr->save( m_editedSitePtr->getFileName() );
    
    invalidateDlg();
    QDialog::accept();
}

void EWASiteEditorDlg::reject()
{
    if( m_editedSitePtr->isModifyed() )
        m_editedSitePtr->load( m_pSrcSettings );
    
    invalidateDlg();
    QDialog::reject();
}

void EWASiteEditorDlg::invalidateDlg()
{
    ui.m_pSimpleProperties->resetSite();
    ui.ewaXMsgSettings->resetSite();

    if( m_pHtmlSelect )
    {
        slotWebElementSelectingCanceled();
    }

    if( m_editedSitePtr )
    {
        m_editedSitePtr->getWidget()->setAsPreview( false );
        
        if( !m_bOriginalMsgVisible && m_editedSitePtr->getDownloadCounter() == m_iDnldCount )
        {
            m_editedSitePtr->getWidget()->realClose();
        }
    }
    
    m_bOriginalMsgVisible = false;
    m_editedSitePtr = 0;
    m_iDnldCount = 0;
    
    m_pSrcSettings->clear();
}

void EWASiteEditorDlg::show()
{
    if( m_editedSitePtr )
    {
        QDialog::show();
        
        if( EWAApplication::settings()->m_pGroupApp->getEditedXMessageWarning() )
        {
            m_pErrorMessageDialog->show();
        }
        
        m_editedSitePtr->getWidget()->slotHideScrollBars();
        m_editedSitePtr->getWidget()->slotShowScrollBars();
        
        if( !m_bOriginalMsgVisible && m_editedSitePtr->extendedMessages() )
        {
            m_editedSitePtr->getWidget()->restoreLocation();
            m_editedSitePtr->getWidget()->makeInvisible( false );
            m_editedSitePtr->getWidget()->show();
        }
        
        if( EWAApplication::settings()->m_pGroupApp->useRichWM() )
            m_editedSitePtr->getWidget()->setWindowOpacity( m_editedSitePtr->getWidget()->getBaseOpacity() );
    }
}

void EWASiteEditorDlg::slotCurrentRowChanged( const QModelIndex& current,
const QModelIndex& next )
{
    Q_UNUSED( current );
    Q_UNUSED( next );

    updateUi();
}

void EWASiteEditorDlg::on_runButton_clicked()
{
    int selectedRow = m_rulesTableViewPtr->currentIndex().row();
    if( selectedRow < 0
        || selectedRow >= model->rowCount()
        || !m_editedSitePtr->getRulesCollection()->getPtr2Rules()
        || !m_editedSitePtr->getRulesCollection()->getPtr2Rules()->count()  )
    {
        return;
    }

    EWAParsingRule rule = m_editedSitePtr->getRulesCollection()->getPtr2Rules()->at( selectedRow );

    QString src = ui.srcTextEdit->toPlainText();
    if( rule.processString( src ) )
    {
        ui.srcTextEdit->setPlainText( src );
        m_rulesTableViewPtr->setCurrentIndex( model->index( selectedRow+1, 0 ) );
    }
}

void EWASiteEditorDlg::on_patternLineEdit_textChanged( const QString& pattern )
{
    QRegExp rx( pattern );

    QPalette palette = ui.patternLineEdit->palette();
    palette.setColor( QPalette::Text, Qt::red );

    if( rx.isValid() )
    {
        palette.setColor( QPalette::Text, Qt::black );
    }

    ui.patternLineEdit->setPalette( palette );
}

void EWASiteEditorDlg::disableBaloons()
{
    ui.msgSysRadioButton->setChecked( false );
    ui.msgSysRadioButton->setEnabled( false );
    m_simplePropertiesPtr->msgSysRadioButton()->setChecked( false );
    m_simplePropertiesPtr->msgSysRadioButton()->setEnabled( false );
}

void EWASiteEditorDlg::on_msgSysRadioButton_toggled( bool on )
{
    if( !EWAApplication::getTrayIconManager()->supportBaloons() )
    {
        disableBaloons();
        return;
    }
    
    if( m_editedSitePtr )
    {
        m_editedSitePtr->setExtendedMessages( !on );
        m_editedSitePtr->getWidget()->setVisible( !on );
    }
    
    ui.pExtendedSettingsListWidget->item( 2 )->setHidden( on );
    ui.pExtendedSettingsListWidget->setCurrentRow( 0 );
    updateMessageSettingsActionTooltip();
}
void EWASiteEditorDlg::on_msgSysExRadioButton_toggled( bool on )
{
    if( !EWAApplication::getTrayIconManager()->supportBaloons() )
    {
        disableBaloons();
        return;
    }
    
    if( m_editedSitePtr )
    {
        m_editedSitePtr->setExtendedMessages( on );
        m_editedSitePtr->getWidget()->setVisible( on );
    }

    ui.pExtendedSettingsListWidget->item( 2 )->setHidden( !on );
    ui.pExtendedSettingsListWidget->setCurrentRow( 0 );
    updateMessageSettingsActionTooltip();
}

void EWASiteEditorDlg::on_durationSpinBox_valueChanged( int val )
{
    if( m_editedSitePtr )
    {
        m_editedSitePtr->setMsgTTL( val );
    }
}

void EWASiteEditorDlg::on_typeComboBox_activated( int num )
{
    if( m_editedSitePtr )
    {
        m_editedSitePtr->setSysTrayMsgType( num );
    }
}

void EWASiteEditorDlg::on_msgLengthGroupBox_toggled( bool on )
{
    if( m_editedSitePtr )
    {
        m_editedSitePtr->setUseMsgLengthLimit( on );
    }
}

void EWASiteEditorDlg::on_charCountSpinBox_valueChanged( int val )
{
    if( m_editedSitePtr )
    {
        m_editedSitePtr->setMsgLengthLimit( val );
    }
}

void EWASiteEditorDlg::on_spacesCheckBox_toggled( bool on )
{
    if( m_editedSitePtr )
    {
        m_editedSitePtr->setNoMsgWordsWrap( on );
    }
}

void EWASiteEditorDlg::on_userAgentComboBox_signalUserAgentChanged( const QString& newUA )
{
    if( m_editedSitePtr )
    {
        m_editedSitePtr->setUserAgent( newUA );
    }
}

void EWASiteEditorDlg::on_pChangeDlgButton_clicked()
{
    m_bExtendedOptions = !m_bExtendedOptions;
    QString btnText, btnIconName, btnToolTip;
    if( m_bExtendedOptions )
    {
        btnText = tr( "Simple" );
        btnIconName = QLatin1String( ":/images/remove.png" );
        btnToolTip = tr( "Show major properties only" );
        showExtendedOptions();
    }
    else
    {
        btnText = tr( "Expert" );
        btnIconName = QLatin1String( ":/images/add.png" );
        btnToolTip = tr( "Show all properties" );
        hideExtendedOptions();
    }
    
    ui.pChangeDlgButton->setText( btnText );
    ui.pChangeDlgButton->setIcon( QIcon( btnIconName ) );
    ui.pChangeDlgButton->setToolTip( btnToolTip );
}

void EWASiteEditorDlg::showExtendedOptions()
{
    m_pToolBar->removeAction( m_pMainSettingsAction );
    m_pToolBar->addAction( m_pMessageSettingsAction );
    m_pMessageSettingsAction->setChecked( true );
    slotChangeTab( m_pMessageSettingsAction );
    m_pToolBar->addAction( m_pAccessSettingsAction );
    m_pToolBar->addAction( m_pParsingSetingsAction );
}

void EWASiteEditorDlg::hideExtendedOptions()
{
    m_pToolBar->addAction( m_pMainSettingsAction );
    m_pMainSettingsAction->setChecked( true );
    slotChangeTab( m_pMainSettingsAction );
    m_pToolBar->removeAction( m_pMessageSettingsAction );
    m_pToolBar->removeAction( m_pAccessSettingsAction );
    m_pToolBar->removeAction( m_pParsingSetingsAction );
}

void EWASiteEditorDlg::initSimplePropertiesWidget()
{   
    m_simplePropertiesPtr = ui.m_pSimpleProperties;
    
    connect( m_simplePropertiesPtr->titleLineEdit(), SIGNAL( textChanged(const QString&) ),
        this, SLOT( slotMsgTitleChanged(const QString&) ) );
    connect( ui.titleLineEdit, SIGNAL( textChanged(const QString&) ),
        this, SLOT( slotMsgTitleChanged(const QString&) ) );
        
    connect( m_simplePropertiesPtr->durationSpinBox(), SIGNAL( valueChanged(int) ),
        ui.durationSpinBox, SLOT( setValue(int) ) );
    connect( ui.durationSpinBox, SIGNAL( valueChanged(int) ),
        m_simplePropertiesPtr->durationSpinBox(), SLOT( setValue(int) ) );
    
    connect( m_simplePropertiesPtr->timeUnitsComboBox(), SIGNAL( activated(int) ),
        ui.timeUnitsComboBox2, SLOT( setCurrentIndex(int) ) );
    connect( ui.timeUnitsComboBox2, SIGNAL( activated(int) ),
        m_simplePropertiesPtr->timeUnitsComboBox(), SLOT( setCurrentIndex(int) ) );
    
    connect( m_simplePropertiesPtr->timeValueSpinBox(), SIGNAL( valueChanged(int) ),
        ui.timeValueSpinBox, SLOT( setValue(int) ) );
    connect( ui.timeValueSpinBox, SIGNAL( valueChanged(int) ),
        m_simplePropertiesPtr->timeValueSpinBox(), SLOT( setValue(int) ) );
    
    connect( m_simplePropertiesPtr->msgSysRadioButton(), SIGNAL( toggled(bool) ),
        ui.msgSysRadioButton, SLOT( setChecked(bool) ) );
    connect( ui.msgSysRadioButton, SIGNAL( toggled(bool) ),
        m_simplePropertiesPtr->msgSysRadioButton(), SLOT( setChecked(bool) ) );
    
    connect( m_simplePropertiesPtr->selectTargetButton(), SIGNAL( clicked() ),
        ui.selectTargetButton, SLOT( click() ) );
    connect( ui.selectTargetButton, SIGNAL( clicked() ),
        this, SLOT( slotSelectTargetButtonClicked() ) );
        
    connect( m_simplePropertiesPtr->changesCheckBox(), SIGNAL( toggled(bool) ),
        ui.matchPopupGroupBox, SLOT( setChecked(bool) ) );
    connect( ui.matchPopupGroupBox, SIGNAL( toggled(bool) ),
        m_simplePropertiesPtr->changesCheckBox(), SLOT( setChecked(bool) ) );    
    
    connect( m_simplePropertiesPtr->msgSysExRadioButton(), SIGNAL( toggled(bool) ),
        ui.msgSysExRadioButton, SLOT( setChecked(bool) ) );
    connect( ui.msgSysExRadioButton, SIGNAL( toggled(bool) ),
        m_simplePropertiesPtr->msgSysExRadioButton(), SLOT( setChecked(bool) ) );
}

void EWASiteEditorDlg::slotMsgTitleChanged( const QString& title )
{
    if( m_editedSitePtr && m_editedSitePtr->getMsgTitle().compare( title ) )
    {
        m_editedSitePtr->setMsgTitle( title );
        m_editedSitePtr->getWidget()->setTitle( m_editedSitePtr->getVisibleMsgTitle() );
        
        QLineEdit *pLE = qobject_cast<QLineEdit *>( sender() );
        if( pLE )
        {
            QLineEdit *pLE2Update;
            if( pLE == ui.titleLineEdit )
                pLE2Update = m_simplePropertiesPtr->titleLineEdit();
            else
                pLE2Update = ui.titleLineEdit;
                
            pLE2Update->setText( title );
        }
    }
}

void EWASiteEditorDlg::on_replayStyleIsHttpButton_toggled( bool on )
{
    if( m_editedSitePtr )
    {
        m_editedSitePtr->setReplayTypeIsHttp( on );
    }
}

void EWASiteEditorDlg::on_replayStyleIsNotHttpButton_toggled( bool on )
{
    if( m_editedSitePtr )
    {
        m_editedSitePtr->setReplayTypeIsHttp( !on );
    }
}

void EWASiteEditorDlg::on_addTrackingRegexpButton_clicked()
{
    if( !m_editedSitePtr )
    {
        return;
    }
    
    disconnect( m_editedSitePtr, SIGNAL( signalDownloaded(const EWASitePtr*) ),
                    this, SLOT( on_addTrackingRegexpButton_clicked() ) );
    if( m_editedSitePtr->getWebView()->isBlankPage() )
    {
        if( QMessageBox::question( this, tr( "Changes monitor" ), 
            tr( "There is no loaded content. Download it now?" ),
            tr( "Download" ), tr( "Cancel" ) ) == 0 )
            {
                connect( m_editedSitePtr, SIGNAL( signalDownloaded(const EWASitePtr*) ),
                    this, SLOT( on_addTrackingRegexpButton_clicked() ) );
                m_editedSitePtr->slotDownload();
            }
            return;
    }
        
    QString currentPattern = ui.patternLineEdit->text();
    if( currentPattern.isEmpty() )
    {
        currentPattern = "(.*)";
    }
    
    QString strSource = m_editedSitePtr->getWatchedElementSource();
    if( strSource.isEmpty() )
    {
        strSource = ui.resulttBrowser->toPlainText();
    }
    EWARegExpDialog *dlg = new EWARegExpDialog( 
        strSource
        ,ui.minCheckBox->isChecked()
        ,ui.csCheckBox ->isChecked()
        ,currentPattern
        ,"%"
        ,0
        ,true );
    if( dlg->exec() == QDialog::Accepted )
    {
        EWAParsingRule rule = dlg->getRule();
        QString strPattern = rule.getPattern();
        ui.patternLineEdit->setText( strPattern );
        ui.minCheckBox->setChecked( rule.isMinimal() );
        ui.csCheckBox ->setChecked( rule.isCaseSensitive() );
        
        if( m_editedSitePtr )
        {
            m_editedSitePtr->getShowPolicy()->setRegExp( rule.makeRegExp() );
        }
    }

    delete dlg;
}

void EWASiteEditorDlg::on_pEditGesturesButton_clicked()
{
    EWAUserActionsEditorDialog *pGesturesEditor = new EWAUserActionsEditorDialog( this );
    pGesturesEditor->setAttribute( Qt::WA_DeleteOnClose );
    pGesturesEditor->setSitePtr( m_editedSitePtr );
    pGesturesEditor->setWindowModified( Qt::WindowModal );
    pGesturesEditor->show();
    
    //if( QDialog::Accepted == pGesturesEditor->exec() )
    //{
    ////-- not implemented yet
    //}
    //delete pGesturesEditor;
}

void EWASiteEditorDlg::initToolBar()
{
    m_pToolBar = new QToolBar( this );
    m_pToolBar->setContextMenuPolicy( Qt::CustomContextMenu );
    m_pToolBar->setFloatable( false );
	m_pToolBar->setMovable( false );
	m_pToolBar->setOrientation( Qt::Horizontal );
	m_pToolBar->setToolButtonStyle( Qt::ToolButtonTextUnderIcon );

	m_pToolBar->setAllowedAreas( Qt::TopToolBarArea );

    QActionGroup *pActions = new QActionGroup( this );
    m_pMainSettingsAction = pActions->addAction( 
        QIcon( ":/images/view.png" ), 
        tr( "View" ) );
    m_pMessageSettingsAction = pActions->addAction( 
        QIcon( ":/images/view.png" ), 
        tr( "View" ) );
    m_pAccessSettingsAction = pActions->addAction( 
        QIcon( ":/images/web.png" ), 
        tr( "Access" ) );
    m_pParsingSetingsAction = pActions->addAction( 
        QIcon( ":/images/eat.png" ), 
        tr( "Parsing" ) );
    
    m_pToolBar->addAction( m_pMainSettingsAction );
    m_pToolBar->addAction( m_pMessageSettingsAction );
    m_pToolBar->addAction( m_pAccessSettingsAction );
    m_pToolBar->addAction( m_pParsingSetingsAction );

    m_pMainSettingsAction->setData( 0 );
    m_pMessageSettingsAction->setData( 1 );
    m_pAccessSettingsAction->setData( 2 );
    m_pParsingSetingsAction->setData( 3 );

    m_pMainSettingsAction->setCheckable( true );
    m_pMessageSettingsAction->setCheckable( true );
    m_pAccessSettingsAction->setCheckable( true );
    m_pParsingSetingsAction->setCheckable( true );

    connect( m_pToolBar,SIGNAL( actionTriggered(QAction*) ),
        this, SLOT( slotChangeTab(QAction*) ) );
    
    ui.pToolbarLayout->addWidget( m_pToolBar );
}

void EWASiteEditorDlg::slotChangeTab(QAction* pAction)
{
    if( !pAction )
    {
        return;
    }
    
    int iPageNum = pAction->data().toInt();
    QStackedWidget *pStackedWidget = ui.pStackedWidgetModes;
    if( iPageNum > 0 )
    {
        pStackedWidget->setCurrentIndex( 1 );
        pStackedWidget = ui.pExtendedStackedWidget;
        --iPageNum;
        
    }
    pStackedWidget->setCurrentIndex( iPageNum );
}

void EWASiteEditorDlg::on_pExtendedSettingsListWidget_currentRowChanged(int iRow )
{
    if( iRow == 0 )
    {
        ui.stackedWidget->setCurrentIndex( iRow ); 
        return;
    }
    
    bool bExtended = m_editedSitePtr && m_editedSitePtr->extendedMessages();
    
    if( bExtended)
    {
        ui.stackedWidget->setCurrentIndex( 2 );
        ui.ewaXMsgSettings->stackedWidget()->setCurrentIndex( iRow-1 );
    }
    else
    {
        if( iRow > 1 )
        {
            iRow = 1;
        }
        
        ui.stackedWidget->setCurrentIndex( iRow );
    }
}

void EWASiteEditorDlg::updateModelItem( const int iRow, const EWAParsingRule *pRule )
{
    if( iRow < 0 || iRow >= model->rowCount() || !pRule )
    {
        return;
    }
    
    model->setData( model->index( iRow, 0 ), pRule->getPattern() );
    model->setData( model->index( iRow, 1 ), pRule->getDestination() );
    model->setData( model->index( iRow, 2 ), pRule->isCaseSensitive() );
    model->setData( model->index( iRow, 3 ), pRule->isMinimal() );
}

void EWASiteEditorDlg::slotEditRule( const QModelIndex& index )
{
    if( index.isValid() && m_editedSitePtr->getRulesCollection()->getPtr2Rules() )
    {
        int iRow = index.row();
        EWAParsingRule *pRule = m_editedSitePtr->getRulesCollection()->getPtr2Rules()->at( iRow );
        if( pRule )
        {
            EWARegExpDialog *pRXDlg = new EWARegExpDialog(
            ui.resulttBrowser->toPlainText()
            ,pRule->isMinimal()
            ,pRule->isCaseSensitive()
            ,pRule->getPattern()
            ,pRule->getDestination()
            ,0
            ,false );
            if( pRXDlg->exec() == QDialog::Accepted )
            {
                EWAParsingRule newRule = pRXDlg->getRule();
                pRule->setPattern( newRule.getPattern() );
                pRule->setDestination( newRule.getDestination() );
                pRule->setMinimal( newRule.isMinimal() );
                pRule->setCaseSensitive( newRule.isCaseSensitive() );
                
                updateModelItem( iRow, pRule );
            }
            delete pRXDlg;
        }
    }
}

 void EWASiteEditorDlg::changeEvent( QEvent * event )
{
    if( event->type() == QEvent::LanguageChange )
    {
        ui.retranslateUi( this );
        
        model->setHeaderData( 0, Qt::Horizontal, EWAApplication::translate( this, "Search for") );
        model->setHeaderData( 1, Qt::Horizontal, EWAApplication::translate( this, "Replace with") );
        model->setHeaderData( 2, Qt::Horizontal, EWAApplication::translate( this, "Casesensetive") );
        model->setHeaderData( 3, Qt::Horizontal, EWAApplication::translate( this, "Minimal") );
        
        QString strEmptyTextTranslated = EWAApplication::instance()->getBlankPageSource();
        if( !ui.srcTextEdit->toPlainText().compare( strEmptyTextTranslated ) )
        {
            ui.srcTextEdit->setPlainText( strEmptyTextTranslated );
        }
        if( !ui.resulttBrowser->toPlainText().compare( strEmptyTextTranslated ) )
        {
            ui.resulttBrowser->setPlainText( strEmptyTextTranslated );
        }
        
        translateActions();
        normalizeListviewsWidths();

        m_pErrorMessageDialog->setText( EWAApplication::translate( this, ::strLimitsMsg.toUtf8().data() ) );
    }

    return QWidget::changeEvent( event );
}

void EWASiteEditorDlg::updateMessageSettingsActionTooltip()
{
    QString strToolTip = tr( "<b>%1</b>:<ul>%2%3" )
    .arg( m_pMessageSettingsAction->text() )
    .arg( actTooltip( ":/images/configure.png", ui.pExtendedSettingsListWidget->item(0)->text() ) )
    .arg( actTooltip( ":/images/view.png", ui.pExtendedSettingsListWidget->item(1)->text() ) );

    if( !ui.pExtendedSettingsListWidget->item( 2 )->isHidden() )
    {
        strToolTip.append( 
            actTooltip( ":/images/web.png", ui.pExtendedSettingsListWidget->item(2)->text() ) );
    }

    m_pMessageSettingsAction->setToolTip( strToolTip );
}

void EWASiteEditorDlg::translateActions()
{
    m_pMainSettingsAction->setText( EWAApplication::translate( this, "View") );
    m_pMessageSettingsAction->setText( EWAApplication::translate( this, "View") );
    m_pAccessSettingsAction->setText( EWAApplication::translate( this, "Access") );
    m_pParsingSetingsAction->setText( EWAApplication::translate( this, "Parsing") );

    updateActionsTooltips();
}

void EWASiteEditorDlg::normalizeListviewsWidths()
{
    int iMaxWidth = EWAApplication::calculateListWidgetsWidth( ui.pExtendedSettingsListWidget );
    iMaxWidth = qMax( iMaxWidth, EWAApplication::calculateListWidgetsWidth( ui.pAccessSettingsListWidget ) );
    iMaxWidth = qMax( iMaxWidth, EWAApplication::calculateListWidgetsWidth( ui.pParsingSettingsListWidget ) );

    ui.pExtendedSettingsListWidget->resize( iMaxWidth, ui.pExtendedSettingsListWidget->height() );
    ui.pExtendedSettingsListWidget->setFixedWidth( iMaxWidth );
    ui.pAccessSettingsListWidget->resize( iMaxWidth, ui.pAccessSettingsListWidget->height() );
    ui.pAccessSettingsListWidget->setFixedWidth( iMaxWidth );
    ui.pParsingSettingsListWidget->resize( iMaxWidth, ui.pParsingSettingsListWidget->height() );
    ui.pParsingSettingsListWidget->setFixedWidth( iMaxWidth );
    
    QListWidget *simplePropsListWidgetPtr = m_simplePropertiesPtr->listWidget();
    simplePropsListWidgetPtr->resize( iMaxWidth, simplePropsListWidgetPtr->height() );
    simplePropsListWidgetPtr->setFixedWidth( iMaxWidth );
}

QString EWASiteEditorDlg::actTooltip( const QString& icon, const QString& text ) const
{
    QString strTemplate = "<br>&nbsp;<img src=\"%1\" width=\"16\" height=\"16\"/>&nbsp;&nbsp;%2";
    return strTemplate.arg( icon ).arg( text );
}

void EWASiteEditorDlg::updateActionsTooltips()
{
    m_pAccessSettingsAction->setToolTip(
    tr( "<b>%1</b>:<ul>%2%3%4%5" )
    .arg( m_pAccessSettingsAction->text() )
    .arg( actTooltip( ":/images/period.png", ui.pAccessSettingsListWidget->item(0)->text() ) )
    .arg( actTooltip( ":/images/changes.png", ui.pAccessSettingsListWidget->item(1)->text() ) )
    .arg( actTooltip( ":/images/mode.png", ui.pAccessSettingsListWidget->item(2)->text() ) )
    .arg( actTooltip( ":/images/user-agent.png", ui.pAccessSettingsListWidget->item(3)->text() ) )
     );
     
    m_pParsingSetingsAction->setToolTip(
    tr( "<b>%1</b>:<ul>%2%3%4" )
    .arg( m_pParsingSetingsAction->text() )
    .arg( actTooltip( ":/images/source.png", ui.pParsingSettingsListWidget->item(0)->text() ) )
    .arg( actTooltip( ":/images/rules.png", ui.pParsingSettingsListWidget->item(1)->text() ) )
    .arg( actTooltip( ":/images/result.png", ui.pParsingSettingsListWidget->item(2)->text() ) )
     );

    updateMessageSettingsActionTooltip();
}

void EWASiteEditorDlg::on_matchPopupGroupBox_toggled( bool on )
{
    if( m_editedSitePtr )
    {
        EWAShowPolicy* pShowPolicy = m_editedSitePtr->getShowPolicy();
        pShowPolicy->setUsed( on );
    }
}

void EWASiteEditorDlg::slotSelectTargetButtonClicked()
{
    if( m_editedSitePtr && !m_pHtmlSelect )
    {
        disconnect( m_editedSitePtr, SIGNAL( signalDownloaded(const EWASitePtr*) ),
            this, SLOT( slotSelectTargetButtonClicked() ) );
                        
        if( m_editedSitePtr->getWebView()->isBlankPage() )
        {
            if( QMessageBox::question( this, tr( "Changes monitor" ), 
                tr( "There is no loaded content. Download it now?" ),
                tr( "Download" ), tr( "Cancel" ) ) == 0 )
                {
                    connect( m_editedSitePtr, SIGNAL( signalDownloaded(const EWASitePtr*) ),
                        this, SLOT( slotSelectTargetButtonClicked() ) );
                    m_editedSitePtr->slotDownload();
                }
                this->setEnabled( true );
                return;
        }
        m_pHtmlSelect = new EWAHTMLSelector( m_editedSitePtr->getWebView() );
        m_pHtmlSelect->setBase( m_editedSitePtr->getWidget() );
        connect( m_pHtmlSelect, SIGNAL( signalAccepted() ),
            this, SLOT( slotWebElementSelectingComplete() ) );
        connect( m_pHtmlSelect, SIGNAL( signalReject() ),
            this, SLOT( slotWebElementSelectingCanceled() ) );
        m_pHtmlSelect->initElementFromPath( m_editedSitePtr->getShowPolicy()->getPath2Element(),
            m_editedSitePtr->getShowPolicy()->getPath2ElementsFrame() );
        m_pHtmlSelect->show();
        m_editedSitePtr->getWidget()->update();
        
        this->setEnabled( false );
    }
}

void EWASiteEditorDlg::on_pMarkerColorButton_clicked()
{
    if( !m_editedSitePtr )
    {
        return;
    }
    
    QColor prevColor = m_editedSitePtr->getChangesMarkerColor();
    
    QColorDialog *pColorDlg = new QColorDialog( m_editedSitePtr->getChangesMarkerColor(), this );
    pColorDlg->setWindowTitle( tr( "Select color for changes highlighting" ) );
    connect( pColorDlg, SIGNAL( currentColorChanged(const QColor&) ),
        m_editedSitePtr, SLOT( setChangesMarkerColor(const QColor&) ) );
    
    bool bRestoreColor = true;
    if( pColorDlg->exec() == QDialog::Accepted )
    {
        QColor newColor = pColorDlg->currentColor();
        if( newColor.isValid() )
        {
            m_editedSitePtr->setChangesMarkerColor( newColor );
            bRestoreColor = false;
        }
    }
    
    if( bRestoreColor )
    {
        m_editedSitePtr->setChangesMarkerColor( prevColor );
    }
    delete pColorDlg;
}

void EWASiteEditorDlg::slotChangesMarkerColorChangedOnFly( const QColor& color )
{
    if( m_editedSitePtr )
    {
        m_editedSitePtr->setChangesMarkerColor( color );
    }
}

void EWASiteEditorDlg::slotWebElementSelectingComplete()
{
    if( !m_pHtmlSelect || !m_editedSitePtr )
    {
        return;
    }
    m_editedSitePtr->getShowPolicy()->setPath2Element( m_pHtmlSelect->getElementsPath() );
    m_editedSitePtr->getShowPolicy()->setPath2ElementsFrame( m_pHtmlSelect->getElementsFramePath() );
    
    QString strPattern = m_editedSitePtr->getShowPolicy()->getRegExp().pattern();
    if( strPattern.isEmpty() )
    {
        strPattern = "(.*)";
        m_editedSitePtr->getShowPolicy()->setRegExp( QRegExp( strPattern ) );
    }
    ui.patternLineEdit->setText( strPattern );
    
    m_editedSitePtr->setPrevMatched( m_editedSitePtr->getShowPolicy()->lookInHtml( m_pHtmlSelect->getSelectedElementSource() ) );
        
    m_editedSitePtr->markupMonitoredElement();//setChangesMarkerColor( m_editedSitePtr->getChangesMarkerColor() );

    
    m_pHtmlSelect->close();
    delete m_pHtmlSelect;
    m_pHtmlSelect = 0;
    
    this->setEnabled( true );
}

void EWASiteEditorDlg::slotWebElementSelectingCanceled()
{
    if( !m_pHtmlSelect )
    {
        return;
    }

    m_pHtmlSelect->close();
    delete m_pHtmlSelect;
    m_pHtmlSelect = 0;
    
    this->setEnabled( true );
    
    //-- TODO: if element for monitoring is not selected 
    //-- and now it was first try for setting it - 
    //-- "monitor changes" checkbox have to be unchecked
    if( m_editedSitePtr->getShowPolicy()->getPath2Element().isEmpty() )
    {
        ui.matchPopupGroupBox->setChecked( false );
    }
}

void EWASiteEditorDlg::on_matchedComboBox_activated( int i )
{
    if( m_editedSitePtr )
    {
        m_editedSitePtr->getShowPolicy()->setShowIfChanged( (bool)i );
    }
}

void EWASiteEditorDlg::connectStackWidgets()
{
    connect(ui.ewaXMsgSettings->stackedWidget(), SIGNAL(currentChanged(int)), this, SLOT(slotShowTabSexy(int)));
    connect(ui.pExtendedStackedWidget, SIGNAL(currentChanged(int)), this, SLOT(slotShowTabSexy(int)));
    connect(ui.stackedWidget, SIGNAL(currentChanged(int)), this, SLOT(slotShowTabSexy(int)));
    connect(ui.pAccessStackedWidget, SIGNAL(currentChanged(int)), this, SLOT(slotShowTabSexy(int)));
    connect(ui.stackedWidget_2, SIGNAL(currentChanged(int)), this, SLOT(slotShowTabSexy(int)));
}


void EWASiteEditorDlg::slotShowTabSexy( int tab )
{
    if( !EWAApplication::splashScreen()->isVisible() )
    {
        QStackedWidget *pStackedWidget = qobject_cast<QStackedWidget*>( sender() );
        if( pStackedWidget )
        {
            if (m_pFaderWidget)
                m_pFaderWidget->close();
            m_pFaderWidget = new EWAFaderWidget(pStackedWidget->widget(tab));
            m_pFaderWidget->start();
        }
    }
}
