/*******************************************************************************
**
** file: ewa_useractionseditordlg.h
**
** class: EWAUserActionsEditorDialog
**
** description:
** Dialog for editing saved user's activity
**
** 25.10.2009
**
** sendevent@gmail.com
**
*******************************************************************************/

#ifndef EWA_USERACTIONSEDITORDLG_H
#define EWA_USERACTIONSEDITORDLG_H

#include "ui_ewa_useractionseditordlg.h"
#include <QPixmap>

class EWASiteHandle;
class EWAUserActionsCollection;
class EWAUserAction;
class QTableView;
class QStandardItemModel;
class EWAUserActionsEditorDialog : public QDialog
{
	Q_OBJECT

    public:
	    EWAUserActionsEditorDialog( QWidget *pParent = 0 );
	    virtual ~EWAUserActionsEditorDialog();

        void setSitePtr( EWASiteHandle* pSite );

    protected:
        Ui::EWAUserActionsEditorDialogUi ui;
        
        EWASiteHandle *m_sitePtr;
        QTableView *m_tableViewPtr;
        QStandardItemModel *m_pModel;
        QVector<EWAUserActionsCollection*>* m_pStoredPages;

        QWidget *m_pPrevMsgParent;
        Qt::WindowFlags prevFlags;
        
        QSize m_szIcons;
        QPixmap m_pixKeyUp
            ,m_pixKeyDown
            ,m_pixMouseUp
            ,m_pixMouseDown;
        
        bool m_bActionExecutionStarted;

        virtual void changeEvent( QEvent * event )
        {
            if( event->type() == QEvent::LanguageChange )
            {
                ui.retranslateUi( this );
            }

            return QWidget::changeEvent( event );
        }
        void createUserActivitysCopy( const QVector<EWAUserActionsCollection*> *pSrc, QVector<EWAUserActionsCollection*> *pDst ) const;
        void clearUserActivitys( QVector<EWAUserActionsCollection*> *pCollectionsVector ) const;
	    void invalidateDlg();
    	
	    void enableControlls();
        void disableControlls();
        void setControllsEnabled( bool bEnabled );
        
        void initModel();
        void connectModel();
        void disconnectModel();
        
        void inserActionIntoModel( int iPos, const EWAUserAction *pAction, int iPageNum, int iActInPageNum );

        void updateWidth();

        bool eventFilter( QObject *obj, QEvent *event );
        
        void execute();
        
        QString getKeyValue( const QEvent *pEvent ) const;
        QPoint getClickCoords( const QEvent *pEvent ) const;

        void updateCoordinatesInfo();
        void normalizeStringsLengths( QString& strX, QString& strY ) const;
        
        void enbleItem( int iNum );
        
        EWAUserAction* getCurrentAction();
        
    public slots:
        void accept();
        void reject();
        void show();
        int exec();

    protected slots:
        void slotLoadFinished(bool);
        void slotLoadStarted();
        
        void on_pExecButton_clicked();
        void on_pNormalizeDelaysButton_clicked();
        void on_pDeleteButton_clicked();
        void on_pChangeCoordsButton_clicked();
};
#endif //--EWA_USERACTIONSEDITORDLG_H
