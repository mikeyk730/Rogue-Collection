#ifndef QT_INPUT_H
#define QT_INPUT_H

#include <QKeyEvent>
#include "replayable_input.h"

class QtRogue;

class QtRogueInput : public ReplayableInput
{
public:
    QtRogueInput(QtRogue* p);

    bool HandleEvent(QKeyEvent *event);

private:
    QtRogue* parent_;
};

#endif // QT_INPUT_H
