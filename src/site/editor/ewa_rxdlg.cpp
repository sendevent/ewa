/*******************************************************************************
**
** file: ewa_regexpdialog.cpp
**
** class: EWARegExpDialog
**
** description:
** Dialog for creating regexps.
**
** 09.02.2009
**
** ewauthor@indatray.com
**
*******************************************************************************/

#include "ewa_rxdlg.h"
#include "ewa_parsingrule.h"
#include "ewa_application.h"
#include "ewa_textsearch.h"

#include <QFile>
#include <QMessageBox>

#include <QDebug>

EWARegExpDialog::EWARegExpDialog( 
    const QString& srcText
    ,bool isMinimal
    ,bool isCaseSensitive
    ,const QString& pattern
    ,const QString& dst
    ,QWidget *parent
    ,bool bHideReplaceWithFields ): QDialog( parent )
{
    ui.setupUi( this );
    setWindowFlags( Qt::Dialog|Qt::CustomizeWindowHint|Qt::WindowCloseButtonHint );
    ui.sourceBrowser->setPlainText( srcText );
    ui.minimalCheckBox->setChecked( isMinimal );
    ui.CSCheckBox->setChecked( isCaseSensitive );

    ui.patternLineEdit->setText( pattern );
    
    if( bHideReplaceWithFields )
    {
        ui.replaceLineEdit->setText( "%" );
        ui.replaceLineEdit->hide();
        ui.label_2->hide();
    }
    else
    {
        ui.replaceLineEdit->setText( dst );
    }

    connect( ui.minimalCheckBox, SIGNAL( toggled(bool) ), this, SLOT( testRegExp() ) );
    connect( ui.CSCheckBox, SIGNAL( toggled(bool) ), this, SLOT( testRegExp() ) );
    connect( ui.patternLineEdit, SIGNAL( textChanged(const QString&) ),
        this, SLOT( testRegExp() ) );
    connect( ui.replaceLineEdit, SIGNAL( textChanged(const QString&) ),
        this, SLOT( testRegExp() ) );
    connect( ui.buttonBox, SIGNAL( clicked(QAbstractButton*) ),
        this, SLOT( slotBtnClicked(QAbstractButton*) ) );

    connect( ui.pTextSearcher, SIGNAL( signalNeedSearch(const QString&,QTextDocument::FindFlags, bool&) ),
        this, SLOT( slotSearchInSource(const QString&, QTextDocument::FindFlags,bool&) ) );
    
    testRegExp();
}

EWARegExpDialog::~EWARegExpDialog()
{
}

EWAParsingRule EWARegExpDialog::getRule() const
{
    return EWAParsingRule(
        ui.patternLineEdit->text(),
        ui.replaceLineEdit->text(),
        ui.minimalCheckBox->isChecked(),
        ui.CSCheckBox->isChecked() );
}

bool EWARegExpDialog::testRegExp()
{
    EWAParsingRule rule = getRule();
    QRegExp rx = rule.makeRegExp();

    QPalette palette = ui.patternLineEdit->palette();
    palette.setColor( QPalette::Text, Qt::red );

    if( rx.isValid() )
    {
        QString text = ui.sourceBrowser->toPlainText();
        if( rule.processString( text ) )
        {
            palette.setColor( QPalette::Text, Qt::black );
            ui.resultBrowser->setPlainText( text );
        }
    }
    ui.patternLineEdit->setPalette( palette );

    update();

    return rx.isValid();
}

void EWARegExpDialog::slotBtnClicked( QAbstractButton *btn )
{
    QPushButton *button = ( QPushButton* )btn;
    if( ui.buttonBox->button( QDialogButtonBox::Help ) == button )
    {
        showHelp();
    }
}

void EWARegExpDialog::showHelp()
{
    QLatin1String notFoundMsg( ":/html/help_regexp.html" );
    QString html;
    QFile file( notFoundMsg );
    bool isOpened = file.open( QIODevice::ReadOnly );
    if( isOpened )
    {
        html = QString( QLatin1String( file.readAll() ) );
    }
    else
    {
        html = tr( "EWARegExpDialog::showHelp: can't open file %1 (%2)" )
            .arg( notFoundMsg ).arg( file.errorString() );
    }

    QMessageBox::information( this, tr( "Help not implemeted yet, sorry" ), html );
}

void EWARegExpDialog::slotSearchInSource( const QString& text,
QTextDocument::FindFlags options, bool& res )
{
    QTextCursor cursor = ui.sourceBrowser->textCursor();
    cursor.clearSelection();
    ui.sourceBrowser->setTextCursor( cursor );
    res = ui.sourceBrowser->find( text, options );

    ui.sourceBrowser->update();
}

void EWARegExpDialog::accept()
{
    if( testRegExp() )
    {
        QDialog::accept();
    }
    else
    {
        QMessageBox::warning( this, tr( "Warning!" ),
        tr( "Syntax error in RegEx pattern:\n"
        "%1\n"
        "Please correct it to continue." ).arg( getRule().makeRegExp().pattern() ) );
    }
}

void EWARegExpDialog::show()
{
    EWAApplication::makeWidgetCentered( this );
    QDialog::show();
}
