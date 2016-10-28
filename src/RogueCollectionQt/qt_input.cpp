#include <QEvent>
#include "qt_input.h"
#include "qt_rogue.h"

QtRogueInput::QtRogueInput(QtRogue *p) :
    ReplayableInput(p->CurrentEnv(), p->GameEnv(), p->Config())
{
    parent_ = p;
}

bool QtRogueInput::HandleEvent(QKeyEvent *event)
{
    QueueInput(event->text().toStdString());
    return true;
}
