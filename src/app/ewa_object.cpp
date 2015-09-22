/*******************************************************************************
**
** file: ewa_object.cpp
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

#include "ewa_object.h"

EWAObject::EWAObject()
{
}

 EWAObject::~EWAObject()
{
}

 void EWAObject::load( QSettings *pSettings )
{
    Q_UNUSED( pSettings );
}
 void EWAObject::save( QSettings *pSettings )
{
    Q_UNUSED( pSettings );
}
