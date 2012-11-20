#include "ewa_siteeditordraft.h"

#include <QStateMachine>
#include <QState>

EWASiteEditorDraft::EWASiteEditorDraft( QWidget *pParent )
:QDialog( pParent )
{
    ui.setupUi( this );
    
    /*m_pStateMachine = new QStateMachine( this );
    m_pBaloonState = new QState();
    m_pWidgetState = new QState();
    m_pSimpleState = new QState();
    m_pExpertState = new QState();*/
}

EWASiteEditorDraft::~EWASiteEditorDraft()
{
}
