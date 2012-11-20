#include "ewa_xmessagebox.h"

EWAXMessageBox::EWAXMessageBox( QWidget *pParent
,bool bShowAgain 
,const QString& strTitle
,const QString& strMessage
,const QPixmap& pm )
:QDialog( pParent )
{
    setWindowFlags( windowFlags() ^ Qt::WindowContextHelpButtonHint );
    m_bShowAgain = bShowAgain;
    ui.setupUi( this );
    
    strTitle.isEmpty() ? setWindowTitle( QApplication::applicationName() ) : setWindowTitle( strTitle );
    setText( strMessage );
    setPixmap( pm );
}

EWAXMessageBox::~EWAXMessageBox()
{
}

void EWAXMessageBox::on_pCheckBox_stateChanged( int iState )
{
    bool bChecked = !iState;
    
    emit signalShowAgainChanged( bChecked );
}

void EWAXMessageBox::setText( const QString& text )
{
    ui.pLabelText->setText( text );
    adjustSize();
}
void EWAXMessageBox::setPixmap( const QPixmap& pm )
{
    if( !pm.isNull() )
    {
        ui.pLabelPixmap->setPixmap( pm );
        adjustSize();
    }
}

