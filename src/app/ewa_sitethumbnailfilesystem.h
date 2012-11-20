#ifndef EWA_SITETHUMBNAILFILESYSTEM_H
#define EWA_SITETHUMBNAILFILESYSTEM_H

#include <QAbstractFileEngineHandler>
#include <QPixmap>
#include <QHash>
#include <QString>

class EWASiteThumbnailFileSystem : public QAbstractFileEngineHandler
{
public:
    EWASiteThumbnailFileSystem();
    ~EWASiteThumbnailFileSystem();
    QAbstractFileEngine * create ( const QString & fileName ) const;
    bool addFile(const QString &filePath, const QByteArray *imgData );
    void removeFile(const QString &filePath, const QByteArray *imgData );
    
    static QString getFilesPrefix();
    
    int totalFiles() const { return m_pFiles ? m_pFiles->count() : -1; }
    
protected:
    QHash<QString,const QByteArray *> *m_pFiles;
    static QString m_strFilesPrefix;
};

#endif //-- EWA_SITETHUMBNAILFILESYSTEM_H
