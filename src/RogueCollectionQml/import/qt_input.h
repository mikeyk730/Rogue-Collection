#ifndef QT_INPUT_H
#define QT_INPUT_H

#include <QKeyEvent>
#include "replayable_input.h"

class QtRogueInput : public ReplayableInput
{
public:
    QtRogueInput(Environment* current_env, Environment* game_env, const GameConfig& options);

    bool HandleEvent(QKeyEvent *event);
};

#endif // QT_INPUT_H
