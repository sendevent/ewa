/*******************************************************************************
**
** file: ewa_siteeditordlg.h
**
** class: EWASiteEditorDlg
**
** description:
** Dialog with sites editing functionality.
**
** 09.02.2009
**
** ewauthor@indatray.com
**
*******************************************************************************/

#ifndef EWA_SITEEDTORDLG_H
#define EWA_SITEEDTORDLG_H

#include <QDialog>
#include <QPointer>

#include "ui_ewa_siteeditordlg.h"

class EWASiteHandle;
class EWASitePtr;
class EWAParsingRule;
class EWAHTMLSelector;
class EWAFaderWidget;
class EWAXMessageBox;
class EWASimpleSiteProperties;

class QSettings;
class QTableView;
class QToolBar;
class QStandardItemModel;
class QStandardItem;
class EWASiteEditorDlg: public QDialog
{
    Q_OBJECT
    
    public:
        EWASiteEditorDlg( QWidget *parent = 0 );
        virtual ~EWASiteEditorDlg();

        void setSite( EWASiteHandle *pSite, bool bNewSite = false );
        const EWASiteHandle* getSite() const { return m_editedSitePtr; }

	    /**
        ** Work with data files:
        */
	    bool updateModelView();

        QTableView *m_rulesTableViewPtr;
        QStandardItemModel *model;
        void initModel();
        bool moveRow( int delat = 1 );

        void connectModel();
        void disconnectModel();

        bool isSiteRulesCorrect();

        void invalidateDlg();

    public slots:
        void slotAddRule();
        void slotDeleteRule();
        
        void accept();
        void reject();
        void show();

        /**
        ** Move current ( selected ) row.
        */
        void slotMoveUpRule();
        /**
        ** Move current ( selected ) row.
        */
        void slotMoveDownRule();

        bool slotSelectRule( int num );

    protected slots:
        /**
        ** Resize edited item to inputed text width.
        */
        void slotDataChanged( const QModelIndex & topLeft, const QModelIndex & bottomRight );

        void slotItemDataChanged( QStandardItem *item );

        void on_msgSysRadioButton_toggled( bool on );
        void on_msgSysExRadioButton_toggled( bool on );
        void on_durationSpinBox_valueChanged( int val );

        void on_typeComboBox_activated( int num );
        void on_msgLengthGroupBox_toggled( bool on );
        void on_charCountSpinBox_valueChanged( int val );
        void on_spacesCheckBox_toggled( bool on );
        void on_replayStyleIsHttpButton_toggled( bool on );
        void on_replayStyleIsNotHttpButton_toggled( bool on );
        
        void on_addTrackingRegexpButton_clicked();

    signals:
        void sugnalRuleChanged( int ruleNumber );

    protected:
        Ui::EWASiteEditorDlgUi ui;
        EWASiteHandle *m_editedSitePtr;
        EWAHTMLSelector *m_pHtmlSelect;
        EWAXMessageBox *m_pErrorMessageDialog;
        
        bool m_bExtendedOptions;
        EWASimpleSiteProperties *m_simplePropertiesPtr;
        QToolBar *m_pToolBar;
        QAction *m_pMainSettingsAction
            ,*m_pMessageSettingsAction
            ,*m_pAccessSettingsAction
            ,*m_pParsingSetingsAction;
        QString m_qstrActionsTooltipTemplate;
        
        QSettings *m_pSrcSettings;
        bool m_bOriginalMsgVisible;
        int m_iDnldCount;
        
        QPointer<EWAFaderWidget> m_pFaderWidget;
        
        void updateUi();
        bool isTrackChangesRxOk();
        
        void transferHtml();
        
        void normalizeListviewsWidths();
        void updateActionsTooltips();
        QString actTooltip( const QString& icon, const QString& text ) const;
        
        virtual void changeEvent( QEvent * event );
        void translateActions();
        void updateMessageSettingsActionTooltip();
        
        void initToolBar();
        
        void initSimplePropertiesWidget();
        void showExtendedOptions();
        void hideExtendedOptions();
        
        void updateSitePeriodTime( int periodUnits );
        
        void updateDownloadPeriodSpinboxesTooltip( int forUnites );
        
        void updateModelItem( const int iRow, const EWAParsingRule *pRule );
        
        void disableBaloons();
        void connectStackWidgets();

    protected slots:
        void on_timeUnitsComboBox_activated( int id );
        void on_timeUnitsComboBox2_activated( int id );
        void on_timeValueSpinBox_valueChanged( int val = -1 );
        void on_downloadButton_clicked();

        void slotSrcChanged();
        bool slotUpdate( int currentRuleNum );

        void slotPageSourceRecived( const EWASitePtr *pSite );

        void slotCurrentRowChanged( const QModelIndex&, const QModelIndex& );
        void on_runButton_clicked();

        void on_patternLineEdit_textChanged( const QString& );

        void on_userAgentComboBox_signalUserAgentChanged( const QString& newUA );
        
        void on_pChangeDlgButton_clicked();
        
        void slotMsgTitleChanged(const QString& title );

        void on_pEditGesturesButton_clicked();
        
        void slotChangeTab(QAction*);
        
        void on_pExtendedSettingsListWidget_currentRowChanged(int);
        
        void slotEditRule( const QModelIndex& index );
        
        void on_matchPopupGroupBox_toggled( bool on );
        
        void slotSelectTargetButtonClicked();
        void on_pMarkerColorButton_clicked();

        void slotWebElementSelectingComplete();
        void slotWebElementSelectingCanceled();
        
        void slotChangesMarkerColorChangedOnFly( const QColor& color );
        
        void on_matchedComboBox_activated( int i );
        
        void slotShowTabSexy( int tab );
};

#endif //-- EWA_SITEEDTORDLG_H
