/*******************************************************************************
**
** file: ewa_object.h
**
** class: EWAObject
**
** description:
** Common EWA's class with interface for loading/saving items with QSettings.
**
** 22.10.2009
**
** sendevent@gmail.com
**
*******************************************************************************/

#ifndef EWA_OBJECT_H
#define EWA_OBJECT_H

#include <QSettings>

class EWAObject 
{
    public:
        EWAObject();
        virtual ~EWAObject();

        virtual void load( QSettings *pSettings );
        virtual void save( QSettings *pSettings );
};
#endif //-- EWA_OBJECT_H
