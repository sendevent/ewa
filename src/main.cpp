#include "ewa_application.h"
#include "ewa_mainwindow.h"
#include "ewa_splashscreen.h"

int main( int argc, char *argv[] )
{
    EWAApplication app( argc, argv );
    
    EWAApplication::splashScreen()->show();
    EWAApplication::processEvents();

    QDir dir = QDir( app.applicationDirPath()+"/staff/" );
    app.setLibraryPaths( QStringList() << dir.absolutePath () << app.libraryPaths() );

    qsrand( QTime::currentTime().msec() );

    app.setQuitOnLastWindowClosed( false );
    app.prepareSysTrayUsage();

    bool bIsFirstRun = !QFileInfo( EWAApplication::settings()->getIniFile() ).exists();
    
    EWAMainWindow *window = app.getMainWindow();

    if( !window )
    {
        qWarning( "Can't create main window. exiting." );
        return 2;
    }
    
    window->loadSettings();
    window->showStartup();
    
    app.slotShowSplashMessage( QObject::tr( "Thinking of goods..." ) );
    EWAApplication::splashScreen()->closeSexy( window );
    EWAApplication::splashScreen()->finish( window );
    
    if( bIsFirstRun )
    {
        app.firstRun();
    }
    
    return app.exec();
}
