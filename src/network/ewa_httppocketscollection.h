/*******************************************************************************
**
** file: ewa_httppocketscollection.cpp
**
** class: EWAHttpPocketsCollection
**
** description:
** Class to store transfered HTTP pockets
**
** 22.10.2009
**
** sendevent@gmail.com
**
*******************************************************************************/

#ifndef EWA_HTTPPOCKETSCOLLECTION_H
#define EWA_HTTPPOCKETSCOLLECTION_H

#include <QObject>
#include <QList>

class EWASiteRoutePoint;
class EWASiteHandle;
class QSettings;
class EWAHttpPocketsCollection : public QObject
{
    Q_OBJECT
    
    public:
        EWAHttpPocketsCollection( QObject *pParent = 0);
        virtual ~EWAHttpPocketsCollection();
        
        void load( QSettings *pSettings );
        void save( QSettings *pSettings );
        
        int packetsCollectionCount() const;
        EWASiteRoutePoint* addPacket( EWASiteRoutePoint* pPacket = 0 );
        
        EWASiteRoutePoint* getPacketAt( int i ) const;
        
        void clear();
        
        void setSite( EWASiteHandle *pSite );
        
        void cloneSettings( const EWAHttpPocketsCollection *pOther );
        
    protected:
        QList<EWASiteRoutePoint*> *m_pRoute;    
        EWASiteHandle *m_sitePtr;
};
#endif //-- EWA_HTTPPOCKETSCOLLECTION_H
