/*******************************************************************************
**
** file: ewa_aboutdialog.cpp
**
** class: EWAAboutDialog
**
** description:
** "About..." dialog implementation
**
** 23.03.2009
**
** ewauthor@indatray.com
**
*******************************************************************************/

#include "ewa_aboutdialog.h"
#include "ewa_application.h"
#include "ewa_feedbackdlg.h"
#include "ewa_timer.h"

#include <QTime>
#include <QClipboard>
#include <QMenu>
#include <QMessageBox>
#include <QtGlobal>

EWAAboutDialog::EWAAboutDialog( QWidget *parent )
    :QDialog( parent )
{
    ui.setupUi( this );
    setWindowFlags( Qt::WindowType_Mask|Qt::WindowStaysOnTopHint );

    int width = calcWidth();
    resize( QSize( width, 230 ) );

    ui.pushButton->setBackgroundRole( QPalette::Base );
    
    m_strUserName = EWAApplication::getUserName();
    m_strMachineName = EWAApplication::getMachineName();
    m_strUsedText = getAboutDefaultText();
    m_strSrcText = QString("");

    minTicks = 10;

    maxTicks =
#ifdef Q_OS_WIN
	250;
#else
    50;
#endif

    m_pCharTimer = new EWATimer( this );

    connect( m_pCharTimer, SIGNAL( signalTimeOut() ), this, SLOT( slotTick() ) );
    connect( ui.textEdit, SIGNAL( customContextMenuRequested(const QPoint&) ),
        this, SLOT( slotShowContextMenu(const QPoint&) ) );

    connect( ui.textEdit, SIGNAL( signalAltF1Pressed() ),
        this, SLOT( slotShowFeedbackForm() ) );
    m_uiPrintCounter = 0;
    
    setWindowTitle( tr( "About Enhanced Web Assistant" ) );
}

EWAAboutDialog::~EWAAboutDialog()
{
}

long EWAAboutDialog::calcWidth() const
{
    QString aboutStr = getAboutDefaultText();
    QString creditsStr = getAboutCreditsText();
    QString debitsStr = getAboutDebitsText();
    QString separator = "%";

    int delta = fontMetrics().width( "W" )+48;

    int res = qMax
        (
            qMax(
                getMaxLineWidth( aboutStr, separator ),
                getMaxLineWidth( creditsStr, separator )
                ),

            getMaxLineWidth( debitsStr, separator )
        );
        
    return res + delta;
}

long EWAAboutDialog::getMaxLineWidth( const QString& m_strUsedText, const QString& linesSeparator ) const
{
    QStringList lines = m_strUsedText.split( linesSeparator  );

    if( !lines.count() )
    {
        return 0;
    }
    
    if( lines.count() == 1 )
    {
        return getLineWidth( lines.at( 0 ) );
    }
    
    int prevWidth = 0;
    for( int i = 0; i < lines.count()-1; i++ )
    {
        QString currentStr = lines.at( i );
        int currentStrWidth = getLineWidth( currentStr );
        QString nextStr = lines.at( ++i );
        int nextStrWidth = getLineWidth( nextStr );
        prevWidth = qMax( prevWidth, qMax( currentStrWidth, nextStrWidth ) );
    }

    return prevWidth;
}

long EWAAboutDialog::getLineWidth( const QString& m_strUsedText ) const
{
    return fontMetrics().width( m_strUsedText );
}

void EWAAboutDialog::show()
{
    ui.textEdit->clear();

    if( m_strSrcText.isEmpty() )
    {
        m_strSrcText = getAboutDefaultText();
    }
    
    m_strUsedText = m_strSrcText;
    
    QDialog::show();
    
    ui.textEdit->insertPlainText( getUserAtMachine() );
    ui.textEdit->ensureCursorVisible();

    ui.textEdit->setFocus();
    
    m_pCharTimer->start( minTicks );
}

void EWAAboutDialog::close()
{
    m_pCharTimer->stop();
    
    ui.creditsButton->setChecked( false );
    ui.debitsButton->setChecked( false );
    
    QDialog::close();
}

void EWAAboutDialog::printNext()
{
    QString str( m_strUsedText[0] );
    if( !str.compare( "%" ) )
    {
        str = "\n";
    }
    echo( str );
    m_strUsedText.remove( 0, 1 );

    if( m_strUsedText.isEmpty() )
    {
        m_pCharTimer->stop();
        
        ui.textEdit->insertPlainText( "\n" );
        ui.textEdit->insertPlainText( getUserAtMachine() );
        ui.textEdit->ensureCursorVisible();
        
        return;
    }

    str = m_strUsedText[0];
    if( !str.compare( "%" ) )
    {
        m_uiPrintCounter = ( unsigned )ui.textEdit->getDelayPeriod( 1, 4 );
        echo( "\n" );
        m_strUsedText.remove( 0, 1 );
    }
    else if( !str.compare( " " ) )
    {
        m_uiPrintCounter = ( unsigned )ui.textEdit->getDelayPeriod( 1, 2 );
        echo( str );
    }

    if( !ui.textEdit->getDelayPeriod( 1, 5 ) )
    {
        m_uiPrintCounter = ( unsigned )ui.textEdit->getDelayPeriod( 1, 3 );
    }
    else
    {
        m_uiPrintCounter /= 2;
    }
}

void EWAAboutDialog::slotTick()
{
    qsrand( QTime::currentTime().msec() );
    
    if( ui.textEdit->fastPrint() )
        while( ui.textEdit->fastPrint() && m_strUsedText.length() > 1 )
        {
            printNext();
            qApp->processEvents();
        }

    if( m_uiPrintCounter > 0 )
    {
        --m_uiPrintCounter;
        return;
    }
    
    int val = ( unsigned )ui.textEdit->getDelayPeriod( minTicks, maxTicks );
    m_pCharTimer->setInterval( val );

    printNext();
}

void EWAAboutDialog::on_creditsButton_toggled( bool on )
{
    if( on )
    {
        if( ui.debitsButton->isChecked() )
            ui.debitsButton->setChecked( false );

        QPalette myPalette = ui.creditsButton->palette();
        myPalette.setColor( ui.creditsButton->backgroundRole(), Qt::black );
        ui.creditsButton->setPalette( myPalette );

        m_strSrcText = getAboutCreditsText();
    }
    else
    {
        m_strSrcText = getAboutDefaultText();
    }
    
    ui.textEdit->setFocus( Qt::MouseFocusReason );
    show();
}
void EWAAboutDialog::on_debitsButton_toggled( bool on )
{
    if( on )
    {
        if( ui.creditsButton->isChecked() )
            ui.creditsButton->setChecked( false );

        QPalette myPalette = ui.debitsButton->palette();
        myPalette.setColor( ui.debitsButton->backgroundRole(), Qt::black );
        ui.debitsButton->setPalette( myPalette );

        m_strSrcText = getAboutDebitsText();
    }
    else
    {
        m_strSrcText = getAboutDefaultText();
    }

    ui.textEdit->setFocus( Qt::MouseFocusReason );
    show();
}

const QString EWAAboutDialog::getUserAtMachine() const
{
    QString strGreeringEndMark = m_strUserName.compare( "root" ) ? "$" : "#";

    return tr( "%1@%2%3 " )
        .arg( m_strUserName )
        .arg( m_strMachineName )
        .arg( strGreeringEndMark );
}

const QString EWAAboutDialog::getCommandMarkText( int mode ) const
{
    QString strCommand( "whois EWA" );
    if( mode == 1)
    {
        strCommand = "EWA >> gratuity";
    }

    if( mode == 2 )
    {
        strCommand = "gratuity | EWA";
    }
    
    QString strHint = tr( "%hint: hold the \"down arrow\" key to force output.%" );

    return strCommand.append( strHint );
}

const QString EWAAboutDialog::getAboutDefaultText() const
{
    QString strQtVer( qVersion() );
    QString strOriginal = tr(
            "%1"
            "%Enhanced Web Assistant v.%2"
            "%(builded at %3; Qt %5)"
            "%"
            "%helps to:"
            "%    automize sites loading;"
            "%    fast detect content's changes;"
            "%    view only the interesting data through"
            "%      desktop widget/system baloon;"
            "%    hinder access to your information for observers."
            "%"
            "%(c) ewauthor, Saint-Petersburg, 2009-2010."
            "%mailto: contact@indatray.com"
            )
            .arg( getCommandMarkText( 0 ) )
            .arg( EWAApplication::getCurrentVersionWithHash() )
            .arg( EWAApplication::getCurrentVersionBuildTime() )
            .arg( strQtVer );
    return EWAApplication::translate( this, strOriginal.toUtf8().data() );
}

const QString EWAAboutDialog::getAboutCreditsText() const
{
     QString strOriginal = tr( 
        "%1"
        "%Thanks to %2 for using EWA."
        "%Thanks to designer@indatray.com for nice icon set."
        "%Thanks to Trolltech (Nokia) for Qt."
        )
        .arg( getCommandMarkText( 1 ) )
        .arg( m_strUserName );
     return EWAApplication::translate( this, 
        strOriginal.toUtf8().data() );
}

const QString EWAAboutDialog::getAboutDebitsText() const
{
    QString strOriginal = tr(
        "%1"
        "%if you wish to post:"
        "% * feature request;"
        "% * bug report;"
        "% * your opinion;"
        "% * etc."
        "%- press Esc now!"
        )
        .arg( getCommandMarkText( 2 ) );
    return EWAApplication::translate( this, 
        strOriginal.toUtf8().data() );
}

void EWAAboutDialog::slotShowContextMenu( const QPoint& pt )
{
    if( ui.debitsButton->isChecked() )
    {
        return;
    }
    
    QMenu *context = new QMenu( this );
    QAction *title = context->addAction( tr( "Copy:" ) );
    title->setEnabled( false );
    context->addSeparator();

    if( ui.creditsButton->isChecked() )
    {
        context->addAction( tr( "designer@indatray.com" ) );
        context->addAction( tr( "http://qt.nokia.com/" ) );
    }
    else
    {
        context->addAction( QString( "EWA %1" ).arg( EWAApplication::getCurrentVersionWithHash() ) );
        context->addAction( tr( "http://qt.nokia.com/" ) );
        context->addAction( tr( "contact@indatray.com" ) );
    }

    QAction *selectedAction = context->exec( ui.textEdit->mapToGlobal( pt ) );
    if( selectedAction )
    {
        EWAApplication::add2Clipboard( selectedAction->text() );
    }

    delete context;
}

void EWAAboutDialog::echo( const QString& m_strUsedText )
{
    ui.textEdit->insertPlainText( m_strUsedText );
    ui.textEdit->ensureCursorVisible();
}

void EWAAboutDialog::slotShowFeedbackForm()
{
    close();

    EWAFeedbackDialog *pDlg = new EWAFeedbackDialog();
    pDlg->setAttribute( Qt::WA_DeleteOnClose );
    pDlg->show();
}

void EWAAboutDialog::changeEvent( QEvent * event )
{
    if( QEvent::LanguageChange == event->type() )
    {
        ui.retranslateUi( this );
        int width = calcWidth();
        resize( QSize( width, 230 ) );
        m_strSrcText = getAboutDefaultText();
        setWindowTitle( EWAApplication::translate( this, "About EWA" ) );
    }

    return QWidget::changeEvent( event );
}
