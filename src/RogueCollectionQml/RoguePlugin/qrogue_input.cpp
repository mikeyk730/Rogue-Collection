#include <map>
#include <QKeyEvent>
#include "qrogue_input.h"
#include "qrogue.h"
#include "key_utility.h"

#define CTRL(ch)   (ch&0x1f)
#define ESCAPE     0x1b

namespace
{
    std::map<int, int> key_aliases = {
        { Qt::Key_Home,     Qt::Key_Y },
        { Qt::Key_Up,       Qt::Key_K },
        { Qt::Key_PageUp,   Qt::Key_U },
        { Qt::Key_Left,     Qt::Key_H },
        { Qt::Key_Right,    Qt::Key_L },
        { Qt::Key_End,      Qt::Key_B },
        { Qt::Key_Down,     Qt::Key_J },
        { Qt::Key_PageDown, Qt::Key_N },
    };

    std::map<int, unsigned char> key_translation = {
        { Qt::Key_Return,    '\r' },
        { Qt::Key_Backspace, '\b' },
        { Qt::Key_Escape,    ESCAPE },
        { Qt::Key_Insert,    '>' },
        { Qt::Key_Delete,    's' },
        { Qt::Key_F1,        '?' },
        { Qt::Key_F2,        '/' },
        { Qt::Key_F3,        'a' },
        { Qt::Key_F4,        CTRL('R') },
        { Qt::Key_F5,        'c' },
        { Qt::Key_F6,        'D' },
        { Qt::Key_F7,        'i' },
        { Qt::Key_F8,        '^' },
        { Qt::Key_F9,        CTRL('F') },
        { Qt::Key_F10,       '!' },
    };
}

bool IsAltOn(QKeyEvent *event){
    return event->modifiers() & Qt::AltModifier;
}

bool IsShiftOn(QKeyEvent *event){
    return event->modifiers() & Qt::ShiftModifier;
}

bool IsCtrlOn(QKeyEvent *event){
    return event->modifiers() & Qt::ControlModifier;
}

QtRogueInput::QtRogueInput(QRogue* parent, Environment* current_env, Environment* game_env, const GameConfig& options) :
    ReplayableInput(current_env, game_env, options),
    parent_(parent)
{
}

void QtRogueInput::HandleReplayKeyEvent(QKeyEvent *event)
{
    int key = event->key();
    char ch = event->text().size() == 1 ? event->text().at(0).toLatin1() : 0;

    if (key == Qt::Key_Space) {
        PauseReplay();
    }
    else if (key == Qt::Key_Return) {
        ResumeReplay();
    }
    else if (key == Qt::Key_Escape) {
        CancelReplay();
        parent_->postRender();
    }
    else if (ch == '-') {
        DecreaseReplaySpeed();
    }
    else if (ch == '+') {
        IncreaseReplaySpeed();
    }
    else if (ch == '0') {
        SetMaxReplaySpeed();
    }
    else if (ch >= '1' && ch <= '9') {
        SetReplaySpeed(ch - '0');
    }
}

std::string QtRogueInput::GetModifiedText(QKeyEvent *event, int ch)
{
    if (!IsLetterKey(ch)) {
        return "";
    }

    if (IsCtrlOn(event)) {
        ch = CTRL(ch);
    }
    else if (IsCapsLockOn() ^ IsShiftOn(event)) {
        ch = toupper(ch);
    }
    return std::string(1, (char)ch);
}

void QtRogueInput::TranslateKey(QKeyEvent *event, int *key, std::string *input)
{
    auto i = key_aliases.find(event->key());
    if (i != key_aliases.end())
    {
        *key = i->second;
        *input = GetModifiedText(event, tolower(i->second));
    }
    else{
        *key = event->key();
        *input = event->text().toStdString();
    }
}

bool QtRogueInput::HandleKeyEvent(QKeyEvent *event)
{
    if (InReplay()) {
        HandleReplayKeyEvent(event);
        return true;
    }

    int key;
    std::string input;
    TranslateKey(event, &key, &input); //todo: numpad?

    // Translate Alt+F9 to 'F'
    if (IsAltOn(event) && key == Qt::Key_F9) {
        QueueInput('F');
        return true;
    }

    if (IsAltOn(event))
        return false;

    // Translate special keys to appropriate input
    auto i = key_translation.find(key);
    if (i != key_translation.end()) {
        QueueInput(i->second);
        return true;
    }

    // Ctrl+dir is translated to 'f'+dir for most versions
    if (IsCtrlOn(event) && IsDirectionKey(tolower(key)))
    {
        if (Options().emulate_ctrl_controls){
            std::string keybuf;
            keybuf.push_back('f');
            keybuf.push_back(tolower(key));
            QueueInput(keybuf);
            return true;
        }
    }

    if (!input.empty()) {
        QueueInput(input);
    }
    return true;
}
