/*******************************************************************************
**
** file: ewa_textlabel.h
**
** class: EWATextLabel
**
** description:
** Transparent text label
**
** 15.02.2009
**
** ewauthor@indatray.com
**
*******************************************************************************/

#ifndef EWA_TEXTLABEL_H
#define EWA_TEXTLABEL_H


#include <QLabel>

class EWATextLabel : public QLabel
{
    Q_OBJECT
    
    public:
        EWATextLabel( QWidget *pParent = 0 );
        virtual ~EWATextLabel();

        void setColor( const QColor& textColor );
        
    protected:
        QLinearGradient m_bckgrndGradient;
        QColor m_color
            ,m_ggColor; //-- gradient's left and right grants
        
        virtual void paintEvent( QPaintEvent *e );
        virtual void resizeEvent( QResizeEvent *e );
        
        void drawBackground();
        void updateBackgroundGradient();
        
        QRect getRectWithText( const QString& str = QString() ) const;
        
        bool isDarckColor( const QColor& color ) const;
        
    public slots:
        void setText( const QString& text );
        void clear();
};

#endif //-- EWA_TEXTLABEL_H