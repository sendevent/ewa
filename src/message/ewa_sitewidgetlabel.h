/*******************************************************************************
**
** file: ewa_sitewidgetlabel.h
**
** class: EWASiteWidgetLabel
**
** description:
** Info control for EWASiteWidget. Showing in message title area time/download
** progress and text description, like as in EWASitesModelView.
**
** 29.05.2009
**
** sendevent@gmail.com
**
*******************************************************************************/

#ifndef EWA_MSGTITLELABEL_H
#define EWA_MSGTITLELABEL_H

#include "ewa_textlabel.h"

class EWASiteHandle;
class EWASiteWidgetLabel : public EWATextLabel
{
    Q_OBJECT
    
    public:
        EWASiteWidgetLabel( QWidget *pParent = 0 );
        virtual ~EWASiteWidgetLabel();

        void setSite( EWASiteHandle *pSite );
        EWASiteHandle* getSite() const { return m_sitePtr; }
        
    protected:
        EWASiteHandle *m_sitePtr;
        long m_lTime
            ,m_lProgress;
        
        virtual void paintEvent( QPaintEvent *e );
        virtual void resizeEvent( QResizeEvent *e );
        
        void connectSite();
        void disconnectSite();
        void drawBackground();
        void updateTooltip();
        
        void setColor( const QColor& color );
        
    protected slots:
        void slotUpdateSiteInfo();
        void slotUpdateSiteInfo( bool );
        void slotUpdateSiteInfo( int );
        void slotUpdateSiteInfo( const QString& );
        
    signals:
        void signalTooltipChanged( const QString& tooltip );
};

#endif //-- EWA_MSGTITLELABEL_H
