#ifndef WIDGET_H
#define WIDGET_H

#include <memory>
#include <QWidget>

#include "qt_display.h"
#include "qt_input.h"

struct Environment;

class QtRogue : public QWidget
{
    Q_OBJECT

public:
    explicit QtRogue(QWidget *parent, Environment* env);
    ~QtRogue();

    Environment* GameEnv() const;
    Environment* CurrentEnv() const;
    QtRogueDisplay* Display() const;
    QtRogueInput* Input() const;
    GameConfig Config() const;

protected:
    virtual bool event(QEvent *event) override;
    virtual void keyPressEvent(QKeyEvent *event) override;

private:
    Environment* env_;
    GameConfig config_;
    std::unique_ptr<QtRogueDisplay> display_;
    std::unique_ptr<QtRogueInput> input_;
};

#endif // WIDGET_H
