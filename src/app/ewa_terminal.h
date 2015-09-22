/*******************************************************************************
**
** file: ewa_terminal.h
**
** class: EWATerminal
**
** description:
** Unix-like "console" with autotyping ( for "about" dialog )
**
** 23.03.2009
**
** sendevent@gmail.com
**
*******************************************************************************/

#ifndef EWA_TERMINAL_H
#define EWA_TERMINAL_H

#include <QTextEdit>


class QPixmap;
class EWATerminal : public QTextEdit
{
    Q_OBJECT

    public:
        EWATerminal( QWidget *parent = 0 );
        ~EWATerminal();
        
        double getDelayPeriod( double min, double max ) const;
        
        bool fastPrint() const {return m_bForcedOutput;}

    protected:
        QPixmap *m_pPixmap;
        bool m_bForcedOutput;
        
        virtual void paintEvent( QPaintEvent *event );
        virtual void keyPressEvent ( QKeyEvent *event );
        virtual void keyReleaseEvent( QKeyEvent *event );
        virtual void mousePressEvent( QMouseEvent *event );
        virtual void mouseReleaseEvent( QMouseEvent *event );
        virtual void mouseDoubleClickEvent( QMouseEvent *event );
        
    signals:
        void signalAltF1Pressed();
};


#endif //-- EWA_TERMINAL_H
