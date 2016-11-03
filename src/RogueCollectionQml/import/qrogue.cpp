/****************************************************************************
**
** Copyright (C) 2015 The Qt Company Ltd.
** Contact: http://www.qt.io/licensing/
**
** This file is part of the documentation of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:BSD$
** You may use this file under the terms of the BSD license as follows:
**
** "Redistribution and use in source and binary forms, with or without
** modification, are permitted provided that the following conditions are
** met:
**   * Redistributions of source code must retain the above copyright
**     notice, this list of conditions and the following disclaimer.
**   * Redistributions in binary form must reproduce the above copyright
**     notice, this list of conditions and the following disclaimer in
**     the documentation and/or other materials provided with the
**     distribution.
**   * Neither the name of The Qt Company Ltd nor the names of its
**     contributors may be used to endorse or promote products derived
**     from this software without specific prior written permission.
**
**
** THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
** "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
** LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
** A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
** OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
** SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
** LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
** DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
** THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
** (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
** OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE."
**
** $QT_END_LICENSE$
**
****************************************************************************/
#include "qrogue.h"
#include "qt_input.h"
#include "qdisplay.h"
#include <QPainter>
#include <QTimer>
#include <sstream>
#include "args.h"
#include "environment.h"
#include "run_game.h"
#include "game_config.h"

QRogue::QRogue(QQuickItem *parent)
    : QQuickPaintedItem(parent),
      config_()
{
    connect(this, SIGNAL(render()), this, SLOT(update()), Qt::QueuedConnection);

    int argc = 0;
    char* argv[] = {0};
    Args args = LoadArgs(argc, argv);

    env_.reset(new Environment(args));
    InitGameConfig(env_.get());

    display_.reset(new QRogueDisplay(this, {80,25}));
}

QRogue::~QRogue()
{
}

QSize QRogue::screenSize() const
{
    return display_->ScreenSize();
}

QString QRogue::game() const
{
    return config_.name.c_str();
}

void QRogue::setGame(const QString &game)
{
    config_ = GetGameConfig(game.toStdString());
    game_env_ = env_;

    int seed = (int)time(0);
    std::ostringstream ss;
    ss << seed;
    game_env_->Set("seed", ss.str());

    LaunchGame();
}

void QRogue::LaunchGame()
{
    if (config_.name == "PC Rogue 1.1") {
        game_env_->Set("emulate_version", "1.1");
    }

    display_->SetGameConfig(config_, game_env_.get());

    if (!game_env_->WriteToOs(config_.is_unix))
        throw_error("Couldn't write environment");

    std::string screen;
    if (game_env_->Get("small_screen", &screen) && screen == "true")
    {
        display_->SetScreenSize(config_.small_screen);
        screenSizeChanged(config_.small_screen.x, config_.small_screen.y);
    }

    input_.reset(new QtRogueInput(env_.get(), game_env_.get(), config_));

    QTimer *timer = new QTimer(this);
    connect(timer, SIGNAL(timeout()), this, SLOT(onTimer()));
    timer->start(250);

    //start rogue engine on a background thread
    char* argv[] = {0};
    std::thread rogue(RunGame<QRogue>, config_.dll_name, 0, argv, this);
    rogue.detach(); //todo: how do we want threading to work?
}

QString QRogue::savefile() const
{
    return "TODO";
}

void QRogue::setSavefile(const QString &savefile)
{

}

QFont QRogue::font() const
{
    return display_->Font();
}

void QRogue::setFont(const QFont &font)
{
    display_->SetFont(font);
    emit fontSizeChanged(fontSize().width(), fontSize().height());
}

QSize QRogue::fontSize() const
{
    return display_->FontSize();
}

void QRogue::paint(QPainter *painter)
{
    display_->Render(painter);
}

void QRogue::onTimer()
{
    display_->Animate();
}

void QRogue::postRender()
{
    emit render();
}

Environment *QRogue::GameEnv() const
{
    return game_env_.get();
}

QtRogueInput *QRogue::Input() const
{
    return input_.get();
}

QRogueDisplay *QRogue::Display() const
{
    return display_.get();
}

void QRogue::keyPressEvent(QKeyEvent *event)
{
    if (display_->HandleKeyEvent(event))
        return;
    else if (!input_->HandleKeyEvent(event))
        QQuickPaintedItem::keyPressEvent(event);
}

InputInterface::~InputInterface(){}
DisplayInterface::~DisplayInterface(){}
