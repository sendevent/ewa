#ifndef EWA_FADERWIDGET_H
#define EWA_FADERWIDGET_H

#include <QWidget>

class QTimer;
class EWAFaderWidget : public QWidget
{
    Q_OBJECT
    Q_PROPERTY(QColor fadeColor READ fadeColor WRITE setFadeColor)
    Q_PROPERTY(int fadeDuration READ fadeDuration WRITE setFadeDuration)
public:

    EWAFaderWidget(QWidget *parent);

    QColor fadeColor() const { return m_startColor; }
    void setFadeColor( const QColor &newColor ) { m_startColor = newColor; }

    int fadeDuration() const { return m_iDuration; }
    void setFadeDuration( int milliseconds ) { m_iDuration = milliseconds; }

    void start();

protected:
    void paintEvent( QPaintEvent *event );

private:
    QTimer *m_pTimer;
    QColor m_startColor;
    int m_iCurrentAlpha;
    int m_iDuration;
};

#endif //-- EWA_FADERWIDGET_H
