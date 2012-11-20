#ifndef EWA_SCROLLAREAVIEWPORT_H
#define EWA_SCROLLAREAVIEWPORT_H

#include <QWidget>

class EWAScrollAreaViewport : public QWidget
{
    Q_OBJECT
    public:
        EWAScrollAreaViewport( QWidget *pParent = 0 );
        virtual ~EWAScrollAreaViewport();
    
    protected:
        //virtual void paintEvent( QPaintEvent *event );
        //virtual void resizeEvent( QResizeEvent *event );
};

#endif //-- EWA_SCROLLAREAVIEWPORT_H