/*******************************************************************************
**
** file: ewa_uacombobox.h
**
** class: EWAUAComboBox
**
** description:
** Class of ComboBox that allow to setup Uer-Agent
**
** 24.09.2009
**
** ewauthor@indatray.com
**
*******************************************************************************/

#ifndef EWA_UACOMBOBOX_H
#define EWA_UACOMBOBOX_H

#include <QComboBox>
#include <QMap>

class EWAUAComboBox : public QComboBox
{
    Q_OBJECT
    Q_PROPERTY(QString m_strUserAgent READ userAgent WRITE setUserAgent)

    public:
        EWAUAComboBox( QWidget *pParent = 0 );
        virtual ~EWAUAComboBox();
        
        QString userAgent() const;

    protected:
        QString m_strLastSelected
            ,m_strUserAgent
            ,m_strCustom
            ,m_strEwa;
            
        int m_IEWAId
            ,m_iCustomId;
        
        QMap<QString, QString> m_userAgentsList;
        
        void connectMe();
        void disconnectMe();

        virtual void changeEvent( QEvent * event );

    public slots:
        void slotItemActivated( const QString& text );
        void slotSetUserAgent( const QString& text );
        void setUserAgent( const QString& ua );

    signals:
        void signalUserAgentChanged( const QString& newUserAgent );
};

#endif //-- EWA_UACOMBOBOX_H
