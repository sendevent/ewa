#include "ewa_textsearch.h"

EWATextSearch::EWATextSearch( QWidget *parent )
:QWidget( parent )
{
    ui.setupUi( this );
    canSearch();
}

EWATextSearch::~EWATextSearch()
{
}

void EWATextSearch::on_lineEdit_textChanged( const QString& text )
{
    Q_UNUSED( text );
    askSearch();
}

void EWATextSearch::on_findPrevButton_clicked()
{
    askSearch( false );
}

void EWATextSearch::on_findNextButton_clicked()
{
    askSearch();
}

void EWATextSearch::on_csCheckBox_toggled( bool on )
{
    Q_UNUSED( on );
    askSearch();
}
void EWATextSearch::on_wwCheckBox_toggled( bool on )
{
    Q_UNUSED( on );
    askSearch();
}

bool EWATextSearch::canSearch()
{
    bool bEnabled = !ui.lineEdit->text().isEmpty();
    ui.findNextButton->setEnabled( bEnabled );
    ui.findPrevButton->setEnabled( bEnabled );

    return bEnabled;
}

void EWATextSearch::askSearch( bool next )
{
    if( !canSearch() )
    {
        return;
    }
    QString exp = ui.lineEdit->text();
    QTextDocument::FindFlags flags;
    if( ui.csCheckBox->isChecked() )
    {
        flags |= QTextDocument::FindCaseSensitively;
    }
    if( !next )
    {
        flags |= QTextDocument::FindBackward;
    }
    if( ui.wwCheckBox->isChecked() )
    {
        flags |= QTextDocument::FindWholeWords;
    }

    bool bFinded = true;
    emit signalNeedSearch( exp, flags, bFinded );

    QPalette palette = ui.lineEdit->palette();
    if( bFinded )
    {
        palette.setColor( QPalette::Text, Qt::black );
    }
    else
    {
        palette.setColor( QPalette::Text, Qt::red );
    }

    ui.lineEdit->setPalette( palette );
    update();
}