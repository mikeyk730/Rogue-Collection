#include <map>
#include <QEvent>
#include "qt_input.h"
#include "qt_rogue.h"

#define CTRL(ch)   (ch&0x1f)
#define ESCAPE     0x1b

std::map<int, unsigned char> keymap = {
    { Qt::Key_Return,   '\r' },
    { Qt::Key_Backspace,'\b' },
    { Qt::Key_Escape,    ESCAPE },
    { Qt::Key_Home,     'y' },
    { Qt::Key_Up,       'k' },
    { Qt::Key_PageUp,   'u' },
    { Qt::Key_Left,     'h' },
    { Qt::Key_Right,    'l' },
    { Qt::Key_End,      'b' },
    { Qt::Key_Down,     'j' },
    { Qt::Key_PageDown, 'n' },
    { Qt::Key_Insert,   '>' },
    { Qt::Key_Delete,   's' },
    { Qt::Key_F1,       '?' },
    { Qt::Key_F2,       '/' },
    { Qt::Key_F3,       'a' },
    { Qt::Key_F4,       CTRL('R') },
    { Qt::Key_F5,       'c' },
    { Qt::Key_F6,       'D' },
    { Qt::Key_F7,       'i' },
    { Qt::Key_F8,       '^' },
    { Qt::Key_F9,       CTRL('F') },
    { Qt::Key_F10,      '!' },
};

QtRogueInput::QtRogueInput(QtRogue *p) :
    ReplayableInput(p->CurrentEnv(), p->GameEnv(), p->Config())
{
    parent_ = p;
}

bool QtRogueInput::HandleEvent(QKeyEvent *event)
{
    std::string input;

    auto i = keymap.find(event->key());
    if (i != keymap.end()) {
        input.push_back(i->second);
    }
    else{
        input = event->text().toStdString();
    }

    QueueInput(input);
    return true;
}
