#ifndef EWA_SPLASHSCREEN_H
#define EWA_SPLASHSCREEN_H

#include <QSplashScreen>

class EWASplashScreen : public QSplashScreen
{
    Q_OBJECT
    
    public:
        EWASplashScreen();
        virtual ~EWASplashScreen();
        
    protected:
        QPixmap m_pixmap;
        QString m_lastMessage;
        QColor m_msgColor;
        int m_iAlignment;
        
        
        virtual void paintEvent( QPaintEvent *event );
        void setupMask();
        
    public slots:
        void showMessage(const QString &message, int alignment = Qt::AlignLeft, const QColor &color = Qt::black);
        void show();
        void closeSexy( QWidget *mainWin );
};


#endif //-- EWA_SPLASHSCREEN_H