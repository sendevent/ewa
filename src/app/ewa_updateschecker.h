/*******************************************************************************
**
** file: ewa_updateschecker.h
**
** class: EWAUpdatesChecker
**
** description:
** Updates checker =)
**
** 14.11.2009
**
** ewauthor@indatray.com
**
*******************************************************************************/

#ifndef EWA_UPDATESCHECKER_H
#define EWA_UPDATESCHECKER_H

#include <QObject>

class EWASiteHandle;
class EWANetworkAccessManager;
class EWATimer;
class EWASitePtr;

class EWAUpdatesChecker : public QObject
{
    Q_OBJECT

    public:
        EWAUpdatesChecker( QObject *pParent = 0 );
        virtual ~EWAUpdatesChecker();

        bool isUsed() const {return m_bUsed;}
        void setUsed( bool bUsed );

        void startChecking();
        void stopChecking();
        
        QString getPrevMatchedString() const;
        void updatePrevMatchedString( const QString& str );

        EWASiteHandle *getSite()const {return m_pSite;}

    protected:
        
        EWASiteHandle *m_pSite;
        EWANetworkAccessManager *m_pNetworkManager;
        EWATimer *m_pTimer;

        QString m_strPrevMatchedValue
            ,m_strUpdatesUrl;

        bool m_bUsed;
        
        void createSite();
        void intiSiteWidget();
        
        void initDefaultPrevMetchedValue();
        
        void initChangeRules();
        void resetChangeRules();

    protected slots:
        void slotOneSecondTicked();
        void slotSiteDownloaded(const EWASitePtr*);
};

#endif //-- EWA_UPDATESCHECKER_H