#include "ewa_settings.h"
#include "ewa_application.h"
#include "ewa_mainwindow.h"
#include "ewa_sitehandle.h"
#include "ewa_siteslistmanager.h"

#ifndef Q_OS_WIN
    #include <QX11Info>
#endif //--  Q_OS_WIN

#include "ewa_updateschecker.h"

#include <QDesktopServices>
#include <QDir>
#include <QFileInfo>
#include <QMessageBox>
#include <QFileDialog>

void EWASettingsGroup::load( QSettings *pSettings, const QString& name )
{
    if( pSettings )
    {
        pSettings->beginGroup( name.isEmpty() ? getName() : name );
            const QMetaObject* metaObject = this->metaObject();
            for(int i = metaObject->propertyOffset(); i < metaObject->propertyCount(); ++i)
            {
                const char *propName = metaObject->property(i).name();
                QVariant propValue = pSettings->value( QString::fromLatin1( propName ) );
                
                setProperty( propName, propValue );
            }
        pSettings->endGroup();
    }
}
void EWASettingsGroup::save( QSettings *pSettings, const QString& name ) const
{
    if( pSettings )
    {
        pSettings->beginGroup( name.isEmpty() ? getName() : name );
            const QMetaObject* metaObject = this->metaObject();
            for(int i = metaObject->propertyOffset(); i < metaObject->propertyCount(); ++i)
            {
                const char *propName = metaObject->property(i).name();
                QVariant propValue = property( propName );
                if( propValue.isValid() )
                {
                    pSettings->setValue( propName, propValue );
                }
            }
        pSettings->endGroup();
    }
}

EWASettings::EWASettings( QObject *pParent )
:QObject( pParent )
{
    m_sSitesListFolder = QString( getUserDataFolder() + "/rules" );
    normolizeSlashesInPath( m_sSitesListFolder );
    m_sIconsCollectionFolder = QString( getUserDataFolder() + "/collection" );
    normolizeSlashesInPath( m_sIconsCollectionFolder );
    m_sTranslationsFolder = QString( getUserDataFolder() + "/localizations" );
    normolizeSlashesInPath( m_sTranslationsFolder );
    m_sFaviconsFolder = QString( getUserDataFolder() + "/favicons" );
    normolizeSlashesInPath( m_sFaviconsFolder );
    m_sCacheFolder = QString( getUserDataFolder() + "/cache" );
    normolizeSlashesInPath( m_sCacheFolder );
    m_sDefaultIconFile = QString( QLatin1String( ":/images/ewa.png" ) );
    normolizeSlashesInPath( m_sDefaultIconFile );

QString strIniName = "ewa.ini";
#ifdef EWA_DBG    
    strIniName = "ewad.ini";
#endif
    m_sIniFile = QString( getUserDataFolder() + "/" + strIniName );
    normolizeSlashesInPath( m_sIniFile );

    m_pGroupSites = new EWASitesTabSettingsGroup( this, "Sites" );
    m_pGroupPreferences = new EWASettingsPreferencesGroup( this, "Preferences" );
    m_pGroupTrayIcon = new EWATrayIconSettingsGroup( this, "SysTray" );
    m_pGroupWebkit = new EWAWebKitSettingsGroup( this, "WebKit" );
    m_pGroupProxy = new EWAProxySettingsGroup( this, "Proxy" );
    m_pGroupCache = new EWACacheSettingsGroup( this, "Cache" );
    m_pGroupApp = new EWAMainWindowSettingsGroup( this, "App" );
#ifndef Q_OS_WIN
    m_pGroupApp->setUseRichWM( QX11Info::isCompositingManagerRunning() );
#else
    m_pGroupApp->setUseRichWM( true );    
#endif    
    
    m_pIniFile = new QSettings( m_sIniFile, QSettings::IniFormat, this );

}

EWASettings::~EWASettings()
{
}


QString EWASettings::getUserDataFolder()
{
    if( m_sUserDataFolder.isEmpty() )
    {
        m_sUserDataFolder = QDesktopServices::storageLocation( QDesktopServices::DataLocation );
    }
    if( !QFileInfo(m_sUserDataFolder).exists() )
    {
        QString locationStr = QDesktopServices::storageLocation(QDesktopServices::DataLocation);
#ifdef Q_WS_MAC
        locationStr.insert(locationStr.count() - QCoreApplication::applicationName().count(),
            QCoreApplication::organizationName() + "/");
#endif

        QDir homeInfo( locationStr );
        m_sUserDataFolder = homeInfo.absolutePath();

        if( !homeInfo.exists() )
        {
            bool bAsk = true;

            if( !m_sUserDataFolder.isEmpty() )
            {
                bAsk = !QDir().mkpath( m_sUserDataFolder );
            }

            if( bAsk )
            {
                processRealAppPath();
            }
        }
    }
    
    normolizeSlashesInPath( m_sUserDataFolder );
    
    return m_sUserDataFolder;
}


QString EWASettings::getSitesListFolder()
{
    QDir targetFolder( m_sSitesListFolder );
    if( !targetFolder.exists() )
    {
        targetFolder.mkdir( m_sSitesListFolder );
        m_sSitesListFolder = targetFolder.absolutePath();
    }
    
    normolizeSlashesInPath( m_sSitesListFolder );
    return m_sSitesListFolder;
}

QString EWASettings::getIconsCollectionFolder()
{
    QDir targetFolder( m_sIconsCollectionFolder );
    if( !targetFolder.exists() )
    {
        targetFolder.mkdir( m_sIconsCollectionFolder );
        m_sIconsCollectionFolder = targetFolder.absolutePath();
    }
    
    normolizeSlashesInPath( m_sIconsCollectionFolder );
    return m_sIconsCollectionFolder;
}

QString EWASettings::getTranslationsFolder()
{
    QDir targetFolder( m_sTranslationsFolder );
    if( !targetFolder.exists() )
    {
        targetFolder.mkdir( m_sTranslationsFolder );
        m_sTranslationsFolder = targetFolder.absolutePath();
    }
    
    normolizeSlashesInPath( m_sTranslationsFolder );
    return m_sTranslationsFolder;
}

QString EWASettings::getFaviconsFolder()
{
    QDir targetFolder( m_sFaviconsFolder );
    if( !targetFolder.exists() )
    {
        targetFolder.mkdir( m_sFaviconsFolder );
        m_sFaviconsFolder = targetFolder.absolutePath();
    }
    
    normolizeSlashesInPath( m_sFaviconsFolder );
    return m_sFaviconsFolder;
}

QString EWASettings::getCacheFolder()
{
    QDir targetFolder( m_sCacheFolder );
    if( !targetFolder.exists() )
    {
        targetFolder.mkdir( m_sCacheFolder );
        m_sCacheFolder = targetFolder.absolutePath();
    }
    
    normolizeSlashesInPath( m_sCacheFolder );
    return m_sCacheFolder;
}

QString EWASettings::getDefaultESLFile()
{
    if( m_sDefaultESLFile.isEmpty() )
    {
        m_sDefaultESLFile = getSitesListFolder() + "/example.esl";
    }
    
    normolizeSlashesInPath( m_sDefaultESLFile );
    return m_sDefaultESLFile;
}

QString EWASettings::getLogFile()
{
    if( m_sLogFile.isEmpty() )
    {
        m_sLogFile = EWASettings::getUserDataFolder() + "/ewa.rec";
    }
    
    normolizeSlashesInPath( m_sLogFile );
    return m_sLogFile;
}

QString EWASettings::getDefaultIconFile()
{
    normolizeSlashesInPath( m_sDefaultIconFile );
    return m_sDefaultIconFile;
}

QString EWASettings::getIniFile()
{   
    normolizeSlashesInPath( m_sIniFile );
    return m_sIniFile;
}

void EWASettings::processRealAppPath()
{
    QMessageBox::information( 0, QObject::tr( "EWA's data storage" ),
        QObject::tr( "Unfortunatly, EWA can't detect path to your home directory."
        "<br>To continue work you need to select it manualy." ) );

    m_sUserDataFolder = QFileDialog::getExistingDirectory( 0,
        QObject::tr( "Choose location for EWA's data storage:" ) );

    if( !QFileInfo( m_sUserDataFolder ).exists() )
    {
        switch( QMessageBox::question(
            0
            ,QObject::tr( "EWA's data storage" )
            ,QObject::tr( "%1 is not valid directory name. If you wish to choose another directory - press \"OK\". Press \"Cancel\" to terminate application." )
            ,QMessageBox::Ok
            ,QMessageBox::Cancel ) )
        {
            case QMessageBox::Ok:
            {
                getUserDataFolder();
            }
            default:
            {
                QApplication::exit(2);
                return;
            }
        }
    }
}

void EWASettings::save()
{
    m_pGroupApp->save( m_pIniFile );
    m_pGroupSites->save( m_pIniFile );
    m_pGroupPreferences->save( m_pIniFile );
    m_pGroupTrayIcon->save( m_pIniFile );
    m_pGroupWebkit->save( m_pIniFile );
    m_pGroupProxy->save( m_pIniFile );
    m_pGroupCache->save( m_pIniFile );
}


void EWASettings::load()
{
    m_pGroupApp->load( m_pIniFile );
    m_pGroupSites->load( m_pIniFile );
    m_pGroupPreferences->load( m_pIniFile );
    m_pGroupTrayIcon->load( m_pIniFile );
    m_pGroupWebkit->load( m_pIniFile );
    m_pGroupProxy->load( m_pIniFile );
    m_pGroupCache->load( m_pIniFile );
}

void EWASettings::normolizeSlashesInPath( QString& path ) const
{
    QString pathDelimSrc( "\\" );
    QString pathDelimDst( "/" );
#ifdef Q_OS_WIN
    qSwap( pathDelimSrc, pathDelimDst );
#endif //-- Q_OS_WIN
    if( path.contains( pathDelimSrc ) )
        path.replace( pathDelimSrc, pathDelimDst );
}