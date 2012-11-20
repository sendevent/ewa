#include <iostream> 
#include <cstdlib> 

#include <QProcess> 
#include <QtGlobal> 
#include <QStringList> 
#include <QFile> 
#include <QTextStream> 
#include <QFileInfo> 
#include <QDateTime> 
#include <QDebug>
#include <QTemporaryFile> 
#include <QSysInfo>

using namespace std;

static QString getProcessesOutput( const QString& strUtility0, const QStringList argsList0,
                                   const QString& strUtility1, const QStringList argsList1 )
{
    QProcess process0, process1;

    process0.setStandardOutputProcess( &process1 );

    process0.start( strUtility0, argsList0 );
    process1.start( strUtility1, argsList1 );

    QString strRes;
    if( process0.waitForStarted() )
    {
        if( process1.waitForStarted() && process1.waitForReadyRead() )
        {
            strRes = QString( process1.readAllStandardOutput().trimmed() );
            process1.waitForFinished();
        }
        process0.waitForFinished();
    }

//    qDebug( "getProcessesOutput: 0: %d", process0.exitCode() );
//    qDebug( "getProcessesOutput: 1: %d", process1.exitCode() );
//    qDebug( "getProcessesOutput: res: [%s]", qPrintable( strRes ) );

    return strRes;
}
static bool getRevisionNumber( QString& strNum, QString& strHash )
{ 
    QString strPATH2GIT="";
    QString strGitbin( "git" );
    QString strWCBin( "wc" );
    QString strAwkBin( "awk" );

#if defined(Q_OS_WIN32)
    QString strExeSuffix( ".exe" );
	if( QSysInfo::WV_WINDOWS7 == QSysInfo::windowsVersion() )
	{
		strPATH2GIT = "C:\\msysgit\\bin\\";
	}
	else
		strPATH2GIT = "C:\\Program Files\\GIT\\BIN\\";
    strGitbin.append( strExeSuffix );
    strWCBin.append( strExeSuffix );
    strAwkBin.prepend( "g" );

#elif defined(Q_OS_LINUX)
    //-- strPATH2GIT = "";
#elif defined(Q_OS_MAC)
    //-- strPATH2GIT = "";
#endif //-- Q_OS_MAC

    strNum = getProcessesOutput( strPATH2GIT+strGitbin,
                                 QStringList() << "log" << "--pretty=format:\"commit \"",
                                 strPATH2GIT+strWCBin,
                                 QStringList() << "-l" );

    qDebug("commits count: %s", qPrintable( strNum ) );


    strHash = QString( "%1" )
             .arg( getProcessesOutput( strPATH2GIT+strGitbin,
                                       QStringList() << "log" << "-1" << "--pretty=%h",
                                       strPATH2GIT+strAwkBin,
                                       QStringList() << "{ print $1 }" )
            )/*.arg( getProcessesOutput( strPATH2GIT+strGitbin,
                                       QStringList() << "log" << "-1" << "--pretty=%ai %h",
                                       strPATH2GIT+strAwkBin,
                                       QStringList() << "{ print $4 }" ) )*/;

    qDebug("commit's hash: %s", qPrintable( strHash ) );
    
    return ( !strNum.isEmpty() && !strHash.isEmpty() );
} 
 
static QByteArray readFile(const QString& fileName) 
{ 
  QFile file(fileName); 
  if (!file.open(QIODevice::ReadOnly)) 
  { 
    return QByteArray(); 
  } 
  return file.readAll(); 
} 
 
static int writeFile(const QString& fileName, const int major, const int minor,
                     const QString& revNum, const QString& revHash )
{ 
  // Create a temp file containing the version info and 
  // only replace the existing one if they are different 
  QTemporaryFile tempFile; 
  if (tempFile.open()) 
  { 
    QTextStream out(&tempFile); 

    out << "#ifndef EWA_VERSIONDEF_H";
    endl( out );
    out << "#define EWA_VERSIONDEF_H";
    endl( out );
    endl( out );

    out << "#define EWA_VERSION_MAJOR " << major;
    endl( out );

    out << "#define EWA_VERSION_MINOR " << minor;
    endl( out );

    out << "#define EWA_VERSION_REVISION " << revNum;
    endl( out );

    out << "#define EWA_VERSION_COMMIT_HASH \"" << revHash << '"';
    endl( out );
    
    out << "#define EWA_VERSION_BLDTIME \"" << QDateTime::currentDateTime().toString( "yyyy-MM-dd, hh:mm:ss" ) << '"';
    endl( out );

    out << "#define EWA_VERSION_DEF( m0, m1, m2 ) m0.m1.m2";
    endl( out );

    out << "#define EWA_VERSION EWA_VERSION_DEF( EWA_VERSION_MAJOR, EWA_VERSION_MINOR, EWA_VERSION_REVISION )";
    endl( out );

    out << "#define _EWA_VERSION_STRING_DEF( x ) #x";
    endl( out );

    out << "#define EWA_VERSION_STRING_DEF( x ) _EWA_VERSION_STRING_DEF( x )";
    endl( out );

    out << "#define EWA_VERSION_STRING EWA_VERSION_STRING_DEF( EWA_VERSION  )";
    endl( out );
    endl( out );

    out << "#endif //-- EWA_VERSIONDEF_H";
    endl( out );

 
    const QString tempFileName = tempFile.fileName(); 
    tempFile.close(); 
 
    if (!QFile::exists(fileName) || readFile(fileName) != readFile(tempFileName)) 
    { 
      QFile::remove(fileName); 
      QFile::copy(tempFileName, fileName); 
    } 
 
    return 0; 
  } 
  else 
  { 
    cout << "Error creating temporary file!" << endl; 
    return 1; 
  } 
} 

QString getExistingHash( const QString& file )
{
    QFile prevFile( file );
    if( prevFile.open( QIODevice::ReadOnly ) )
    {
        QString source = QObject::tr( prevFile.readAll() );
        QRegExp rx( "#define EWA_VERSION_COMMIT_HASH\\s\"(.*)\"" );
        rx.setMinimal( true );

        int pos = 0;
        if( ( pos = rx.indexIn( source, pos ) ) != -1 )
        {
            return rx.cap( 1 );
        }
    }

    return QString();
}
 
int main(int argc, char *argv[]) 
{

    if (argc != 4)
    {
        cout << "Usage: version major minor filename" << endl;
        return 1;
    }

    QString strNum, strHash;
    if( !getRevisionNumber( strNum, strHash ) )
    {
        cout << "Can't get revision number/hash." << endl;
        return 2;
    }

    QString strPrev = getExistingHash( argv[3] );
    qDebug( "%s", qPrintable( strPrev ));
    if( !strPrev.isEmpty() && !strHash.compare( strPrev ) )
    {
        qDebug( "version not changed" );
        return 0;
    }

    bool bConverted = false;
    const int major = QString( argv[1] ).toInt( &bConverted, 10 );
    if( !bConverted )
    {
        cout << "Can't covert major to int (" << argv[1] << ")" << endl;
        return 3;
    }

    const int minor = QString( argv[2] ).toInt( &bConverted, 10 );
    if( !bConverted )
    {
        cout << "Can't covert minor to int (" << argv[2] << ")" << endl;
        return 4;
    }

    qDebug( "version: %s", qPrintable(
            QString( "%1.%2.%3 (%4)" )
            .arg( major )
            .arg( minor )
            .arg( strNum )
            .arg( strHash ) ) );





    return writeFile(argv[3], major, minor, strNum, strHash );
} 
