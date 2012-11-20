#ifndef EWA_BUZZER_H
#define EWA_BUZZER_H

#include <QObject>
#include <QNetworkRequest>

class QNetworkReply;
class QNetworkAccessManager;
class EwaBuzzer : public QObject
{
    Q_OBJECT

    public:
        EwaBuzzer( QObject *parent = 0 );
        virtual ~EwaBuzzer();

        void sayHelloHomeServer();
        void sayGoodbayHomeServer();

    protected:
        QNetworkAccessManager *m_pNetwork;
        QByteArray m_encodedID;
        QNetworkRequest m_reqOnStart, m_reqOnQuit;

        QByteArray m_EncodableChars, m_NonEncodableChars;
        
        void initRequests();

        void processHomeServerResponse( const QString& text, bool isOnStart ) const;

    protected slots:
        void slotRequestFinished( QNetworkReply *pReply );
};


#endif //- EWA_BUZZER_H