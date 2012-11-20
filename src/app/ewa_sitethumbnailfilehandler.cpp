#include "ewa_sitethumbnailfilehandler.h"
#include "ewa_sitethumbnailfilesystem.h"

#include <QByteArray>

#include <QtDebug>

/*static*/ int EWASiteThumbnailFileHandler::iCounter = 0;
EWASiteThumbnailFileHandler::EWASiteThumbnailFileHandler( const QByteArray *pBa )
{
    m_pData = pBa;
    iCounter++;
}
EWASiteThumbnailFileHandler::~EWASiteThumbnailFileHandler()
{
    iCounter--;
}

bool EWASiteThumbnailFileHandler::open ( QIODevice::OpenMode mode )
{
    if(mode & QIODevice::ReadOnly )
    {
        m_pos = 0;
        return true;
    }

    return false;
}

qint64 EWASiteThumbnailFileHandler::pos () const
{
    return m_pos;
}

qint64 EWASiteThumbnailFileHandler::read ( char * ddata, qint64 maxlen )
{
    if( m_pData )
    {
        maxlen = qBound( qint64(0), m_pData->size()-m_pos, maxlen );
        memcpy( ddata, m_pData->constData()+m_pos, maxlen );
        m_pos+=maxlen;
    }
    return maxlen;
}
bool EWASiteThumbnailFileHandler::seek ( qint64 offset )
{
    if( !m_pData ) return false;

    if( offset<0 || offset>=m_pData->size() ) return false;
    m_pos = offset;
    return true;
}

qint64 EWASiteThumbnailFileHandler::size () const {
    return m_pData ? m_pData->size() : -1;
}

bool EWASiteThumbnailFileHandler::close () {
    return true;
}
bool EWASiteThumbnailFileHandler::isSequential () const {
    return false;
}

bool EWASiteThumbnailFileHandler::supportsExtension ( Extension extension ) const{
    return QAbstractFileEngine::supportsExtension(extension);
}
