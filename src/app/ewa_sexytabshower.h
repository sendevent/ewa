#ifndef EWA_SEXYTABSHOWER_H
#define EWA_SEXYTABSHOWER_H

#include <QWidget>
#include <QPointer>

class EWAFaderWidget;
class EWASexyTabShower : public QWidget
{
    Q_OBJECT
    public:
        EWASexyTabShower( QWidget *pParent = 0 );
        virtual ~EWASexyTabShower();
    
    
    protected:
        QPointer<EWAFaderWidget> m_pFaderWidget;
        virtual void connectStackWidgets() = 0;
        
    protected slots:
        virtual void slotShowTabSexy( int tab );
};


#endif //-- EWA_SEXYTABSHOWER_H