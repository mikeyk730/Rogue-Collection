#ifndef QROGUE_H
#define QROGUE_H

#include <memory>
#include <mutex>
#include <QtQuick/QQuickPaintedItem>
#include <coord.h>
#include "game_config.h"

struct Environment;
class QtRogueInput;
class QRogueDisplay;

class QRogue : public QQuickPaintedItem
{
    Q_OBJECT
    Q_PROPERTY(QFont font READ font WRITE setFont)
    Q_PROPERTY(QSize fontSize READ fontSize NOTIFY fontSizeChanged)
    Q_PROPERTY(QSize screenSize READ screenSize)

public:
    QRogue(QQuickItem *parent = 0);
    ~QRogue();

    QFont font() const;
    void setFont(const QFont& font);

    QSize fontSize() const;
    QSize screenSize() const;
    virtual void paint(QPainter *painter) override;
    void postRender();

public slots:
    void onTimer();

signals:
    void render();
    void fontSizeChanged(int height, int width);    

public:
    Environment* GameEnv() const;
    Environment* CurrentEnv() const;
    QtRogueInput* Input() const;
    QRogueDisplay* Display() const;
    GameConfig Config() const;

protected:
    virtual void keyPressEvent(QKeyEvent *event) override;

private:
    Environment* env_;
    GameConfig config_;
    std::unique_ptr<QtRogueInput> input_;
    std::unique_ptr<QRogueDisplay> display_;
};

#endif

