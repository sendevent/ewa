#ifndef EWA_SITEEDITORDRAFT_H
#define EWA_SITEEDITORDRAFT_H

#include <QDialog>
#include <QPointer>

#include "ui_ewa_siteeditordraft.h"

class QStateMachine;
class QState;

class EWASiteEditorDraft : public QDialog
{
    Q_OBJECT
    public:
        EWASiteEditorDraft( QWidget *pParent = 0);
        ~EWASiteEditorDraft();
    
    protected:
        Ui::EWASiteEditorDraftUi ui;
        QPointer<QStateMachine> *m_pStateMachine;
        QPointer<QState> *m_pBaloonState
            ,*m_pWidgetState
            ,*m_pSimpleState
            ,*m_pExpertState;
};

#endif //-- EWA_SITEEDITORDRAFT_H
