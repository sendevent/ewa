#ifndef EWA_SITETHUMBNAILFILEHANDLER_H
#define EWA_SITETHUMBNAILFILEHANDLER_H

#include <QAbstractFileEngine>

class QByteArray;
class EWASiteThumbnailFileHandler : public QAbstractFileEngine
{
public:
    EWASiteThumbnailFileHandler( const QByteArray *pBa = 0 );
    ~EWASiteThumbnailFileHandler();
    
    bool open( QIODevice::OpenMode mode );
    qint64 pos() const;
    qint64 read( char * data, qint64 maxlen );
    bool seek( qint64 offset );
    qint64 size() const;
    bool close();
    bool isSequential() const;
    bool supportsExtension( Extension extension ) const;
    
private:
    const QByteArray *m_pData;
    qint64 m_pos;
    
    static int iCounter;
    
};

#endif //-- EWA_SITETHUMBNAILFILEHANDLER_H
