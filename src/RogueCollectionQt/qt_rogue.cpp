#include <QEvent>
#include "qt_rogue.h"

QtRogue::QtRogue(QWidget *parent, Environment* env) :
    QWidget(parent),
    env_(env)
{
    display_.reset(new QtRogueDisplay(this));
    input_.reset(new QtRogueInput(this));
}

QtRogue::~QtRogue()
{
}

Environment *QtRogue::GameEnv() const
{
    return env_;
}

Environment *QtRogue::CurrentEnv() const
{
    return env_;
}

QtRogueDisplay *QtRogue::Display() const
{
    return display_.get();
}

QtRogueInput *QtRogue::Input() const
{
    return input_.get();
}

GameConfig QtRogue::Config() const
{
    return config_;
}

bool QtRogue::event(QEvent *event)
{
    if (display_->HandleEvent(event))
        return true;
    return QWidget::event(event);
}

void QtRogue::keyPressEvent(QKeyEvent *event)
{
    if (!input_->HandleEvent(event))
        QWidget::keyPressEvent(event);
}
