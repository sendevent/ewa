#include "ewa_colordialog.h"
#include "ewa_application.h"

#include <QColorDialog>
#include <QColor>

EWAColorDialog::EWAColorDialog( QWidget *parent )
:QDialog( parent )
{
    ui.setupUi( this );

    resortChildWidgets();

    connect( ui.widget, SIGNAL( signalColorChanged(const QRgb&) ),
    this, SLOT( slotColorChanged(const QRgb&) ) );
}

EWAColorDialog::~EWAColorDialog()
{
}

void EWAColorDialog::setColor( const QColor& color )
{
    if( color.isValid() )
    {
        m_pColorDlg->setCurrentColor( color );
    }
}

QColor EWAColorDialog::getColor() const
{
    return m_pColorDlg->currentColor();
}

void EWAColorDialog::resortChildWidgets()
{
    m_pColorDlg = new QColorDialog();
    m_pColorDlg->setOption( QColorDialog::NoButtons );

    QLayoutItem* label = ui.gridLayout_3->itemAt( 0 );
    ui.gridLayout_3->removeItem( label );

    ui.gridLayout_3->addWidget( m_pColorDlg, 0, 0 );
    ui.gridLayout_3->addItem( label, 1, 0 );

    this->adjustSize();
}


void EWAColorDialog::slotColorChanged( const QRgb& rgb )
{
    QColor color( rgb );
    m_pColorDlg->setCurrentColor( color );
}

void EWAColorDialog::show()
{
    EWAApplication::makeWidgetCentered( this );

    QDialog::show();
}
