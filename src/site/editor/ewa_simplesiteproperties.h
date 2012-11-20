#ifndef EWA_SIMPLESITEPROPERTIES_H
#define EWA_SIMPLESITEPROPERTIES_H

#include <QWidget>
#include "ui_ewa_simplesiteproperties.h"

class EWASiteHandle;
class EWASimpleSiteProperties : public QWidget
{
    Q_OBJECT

    public:
        EWASimpleSiteProperties( QWidget *pParent = 0 );
        ~EWASimpleSiteProperties();
        void setSite( EWASiteHandle *pSite );
        void resetSite();
        
        QLineEdit* titleLineEdit() const { return ui.titleLineEdit; }
        QSpinBox* durationSpinBox() const {return ui.durationSpinBox; }
        QSpinBox* timeValueSpinBox() const {return ui.timeValueSpinBox; }
        QComboBox* timeUnitsComboBox() const {return ui.timeUnitsComboBox; }
        QRadioButton* msgSysRadioButton() const {return ui.msgSysRadioButton; }
        QRadioButton* msgSysExRadioButton() const {return ui.msgSysExRadioButton; }
        QListWidget* listWidget() const { return ui.pSimplePropertiesListWidget; }
        QPushButton* selectTargetButton() const { return ui.selectTargetButton; }
        QCheckBox* changesCheckBox() const {return ui.changesCheckBox;}

    protected:
        Ui::SimpleSiteEditorUi ui;
        virtual void changeEvent( QEvent * event );
};
#endif //-- EWA_SIMPLESITEPROPERTIES_H
