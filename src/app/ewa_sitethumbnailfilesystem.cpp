#include "ewa_sitethumbnailfilesystem.h"
#include "ewa_sitethumbnailfilehandler.h"

#include <QtDebug>
#include <QBuffer>
/*static*/ QString EWASiteThumbnailFileSystem::m_strFilesPrefix = QString( "ewa_site_thumbnail_file:" );

EWASiteThumbnailFileSystem::EWASiteThumbnailFileSystem()
:m_pFiles(0)
{
    m_pFiles = new QHash<QString,const QByteArray *>();
}

EWASiteThumbnailFileSystem::~EWASiteThumbnailFileSystem()
{
    delete m_pFiles;
}


QAbstractFileEngine * EWASiteThumbnailFileSystem::create ( const QString & fileName ) const 
{
    if( fileName.startsWith( getFilesPrefix() )  )
    {
        if( m_pFiles )
        {
            return new EWASiteThumbnailFileHandler( m_pFiles->value( fileName ) );
        }
    }

    return 0;
}

bool EWASiteThumbnailFileSystem::addFile( const QString &filePath, const QByteArray *imgData )
{
    if( m_pFiles )
    {
        m_pFiles->insert( filePath, imgData );
        return true;
    }
    
    return false;
}

void EWASiteThumbnailFileSystem::removeFile(const QString &filePath, const QByteArray *imgData )
{
    if( m_pFiles )
    {
        if( m_pFiles->key( imgData ) == filePath )
        {
            m_pFiles->remove( filePath );
        }
    }
}

/*static*/ QString EWASiteThumbnailFileSystem::getFilesPrefix()
{
    return m_strFilesPrefix;
}
