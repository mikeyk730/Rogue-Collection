#include <QPainter>
#include <QTimer>
#include <QCoreApplication>
#include <QDir>
#include <sstream>
#include <fstream>
#include <thread>
#include <cstdio>
#include "qrogue.h"
#include "qrogue_input.h"
#include "qrogue_display.h"
#include "args.h"
#include "environment.h"
#include "run_game.h"
#include "game_config.h"
#include "tile_provider.h"
#include "pipe_input.h"
#include "utility_qml.h"
#include "utility.h"

namespace
{
    QString convertUrlToNativeFilePath(const QUrl& url)
    {
        return QDir::toNativeSeparators(url.toLocalFile());
    }

    Args GetArgs()
    {
        QStringList q_args = QCoreApplication::arguments();
        std::vector<std::string> v;
        for (int i = 0; i < q_args.size(); ++i) {
            v.push_back(q_args[i].toStdString());
        }

        return Args(v);
    }
}

const unsigned char QRogue::kSaveVersion = 2;

QRogue::QRogue(QQuickItem *parent)
    : QQuickPaintedItem(parent),
      args_(GetArgs()),
      config_(),
      thread_exited_(false)
{
    connect(this, SIGNAL(render()), this, SLOT(update()), Qt::QueuedConnection);
    connect(this, SIGNAL(soundEvent(const QString&)), this, SLOT(playSound(const QString&)), Qt::QueuedConnection);

    env_.reset(new Environment(args_));

    std::string graphics;
    env_->Get("gfx", &graphics);
    int frogue = args_.rogomatic ? args_.GetDescriptorFromRogue() : 0;
    display_.reset(new QRogueDisplay(this, {80,25}, graphics, frogue));

    std::string value;
    bool sound(!env_->Get("sound", &value) || value != "false");
    display_->SetSound(sound);

    std::unique_ptr<GameConfig> game_config;
    if (args_.rogomatic_player) {
        game_config.reset(new GameConfig(GetRogomaticGameConfig()));
    }

    if (!game_config){
        std::string game;
        if (env_->Get("game", &game) && !game.empty()){
            int i = GetGameIndex(game.c_str());
            if (i == -1  && game.size() == 1 && (game[0] >= 'a' && game[0] < 'a' + static_cast<int>(s_options.size()))){
                i = game[0] - 'a';
            }

            if (i != -1)
            {
                game_config.reset(new GameConfig(GetGameConfig(i)));
            }
            else
            {
                RestoreGame(game.c_str());
            }
        }
    }

    if (game_config)
    {
        setGame(*game_config);
    }
}

QRogue::~QRogue()
{
    autosave();
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
    int i = GetGameIndex(game.toStdString());
    setGame(GetGameConfig(i));
}

void QRogue::setGame(const GameConfig& game)
{
    if (args_.rogomatic)
    {
        if (!game.supports_rogomatic)
        {
            DisplayMessage("Error", "Rogomatic", "Rogomatic doesn't support " + game.name);
            args_.rogomatic = false;
        }
    }

    if (args_.rogomatic)
    {
        env_->SetRogomaticValues();
    }

    config_ = game;
    emit gameChanged(config_.name.c_str());
    game_env_ = env_;

    int seed = static_cast<int>(time(nullptr));
    std::ostringstream ss;
    ss << seed;
    game_env_->Set("seed", ss.str());

    if (args_.rogomatic)
    {
        input_.reset(new PipeInput(env_.get(), game_env_.get(), config_, args_.GetDescriptorToRogue()));
    }
    else
    {
        input_.reset(new QtRogueInput(this, env_.get(), game_env_.get(), config_));
    }

    LaunchGame();
}

bool QRogue::showTitleScreen()
{
    return config_.name == "PC Rogue 1.48" && restore_count_ == 0;
}

void QRogue::restoreGame(const QUrl& url)
{
    auto filename = convertUrlToNativeFilePath(url);
    RestoreGame(filename.toStdString());
}

void QRogue::RestoreGame(const std::string& path)
{
    std::ifstream file(path, std::ios::binary | std::ios::in);
    if (!file) {
        throw_error("Couldn't open save file: " + path);
    }

    unsigned char version;
    Read(file, &version);
    if (version > QRogue::kSaveVersion)
        throw_error("This file is not recognized.  It may have been saved with a newer version of Rogue Collection.  Please download the latest version and try again.");

    Read(file, &restore_count_);
    ++restore_count_;

    std::string name;
    ReadShortString(file, &name);
    int i = GetGameIndex(name);
    config_ = GetGameConfig(i);
    emit gameChanged(config_.name.c_str());

    // set up game environment
    game_env_.reset(new Environment());
    game_env_->Deserialize(file);
    game_env_->Set("in_replay", "true");
    std::string value;
    if (env_->Get("logfile", &value)) {
        game_env_->Set("logfile", value);
    }
    if (version == 1){
        game_env_->Set("trap_bugfix", "false");
        game_env_->Set("room_bugfix", "false");
        game_env_->Set("confused_bugfix", "false");
    }

    input_.reset(new QtRogueInput(this, env_.get(), game_env_.get(), config_));
    input_->RestoreGame(file);

    if (env_->Get("delete_on_restore", &value) && value == "true") {
        file.close();
        std::remove(path.c_str());
    }

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
    else //todo:mdk fixed size for rogomatic
    {
        display_->SetScreenSize(config_.screen);
        screenSizeChanged(config_.screen.x, config_.screen.y);
    }

    QTimer *timer = new QTimer(this);
    connect(timer, SIGNAL(timeout()), this, SLOT(onTimer()));
    timer->start(250);

    //start rogue engine on a background thread
    std::thread rogue([=] {
        RunGame(config_.dll_name, Display(), Input(), Lines(), Columns(), args_);
        thread_exited_ = true;
        QuitApplication();
    });
    rogue.detach(); //todo: how do we want threading to work?
}

void QRogue::saveGame(const QUrl& url)
{
    auto filename = convertUrlToNativeFilePath(url);
    SaveGame(filename.toStdString(), true);
}

void QRogue::nextGraphicsMode()
{
    display_->NextGfxMode();
}

void QRogue::autosave()
{
    std::string value;
    if (input_ && env_->Get("autosave", &value)){
        if ((value == "true" && !thread_exited_) || value == "force"){
            std::string name = "autosave-" + GetTimestamp() + ".sav";
            SaveGame(name, false);
        }
    }
}

QString QRogue::graphics() const
{
    return display_->Graphics();
}

void QRogue::setGraphics(const QString &gfx)
{
    display_->SetGraphics(gfx);
}

void QRogue::SaveGame(std::string path, bool notify)
{
    std::ofstream file(path, std::ios::binary | std::ios::out);
    if (!file) {
        DisplayMessage("Error", "Save Game", "Couldn't open save file: " + path);
        return;
    }

    Write(file, QRogue::kSaveVersion);
    Write(file, restore_count_);
    WriteShortString(file, config_.name);
    game_env_->Serialize(file);
    input_->SaveGame(file);

    if (!file) {
        DisplayMessage("Error", "Save Game", "Error writing to file: " + path);
        return;
    }

    if (notify) {
        std::string value;
        bool quit(env_->Get("exit_on_save", &value) && value == "true");

        std::string msg = "Your game was saved successfully.";
        if (quit){
            msg += "  Come back soon!";
        }
        DisplayMessage("Info", "Save Game", msg);

        if (quit)
            QuitApplication();
    }
}

QFont QRogue::font() const
{
    return display_->Font();
}

void QRogue::setFont(const QFont &font)
{
    display_->SetFont(font);
}

bool QRogue::sound() const
{
    return display_->Sound();
}

void QRogue::setSound(bool enable)
{
    display_->SetSound(enable);
}

bool QRogue::monochrome() const
{
    return display_->Monochrome();
}

void QRogue::setMonochrome(bool enable)
{
    display_->SetMonochrome(enable);
}

QSize QRogue::fontSize() const
{
    return display_->TileSize();
}

void QRogue::paint(QPainter *painter)
{
    display_->Render(painter);
    emit rendered();
}

void QRogue::onTimer()
{
    display_->Animate();
}

void QRogue::postRender()
{
    emit render();
}

void QRogue::tileSizeChanged()
{
    emit fontSizeChanged(fontSize().width(), fontSize().height());
}

void QRogue::playSound(const QString &id)
{
    display_->PlaySoundMainThread(id);
}

Environment *QRogue::GameEnv() const
{
    return game_env_.get();
}

ReplayableInput *QRogue::Input() const
{
    return input_.get();
}

QRogueDisplay *QRogue::Display() const
{
    return display_.get();
}

int QRogue::Lines() const
{
    return screenSize().height();
}

int QRogue::Columns() const
{
    return screenSize().width();
}

void QRogue::keyPressEvent(QKeyEvent *event)
{
    if (display_->HandleKeyEvent(event))
        return;

    auto qt_input = dynamic_cast<QtRogueInput*>(input_.get());
    if (qt_input && qt_input->HandleKeyEvent(event))
        return;

    QQuickPaintedItem::keyPressEvent(event);
}

InputInterface::~InputInterface(){}
DisplayInterface::~DisplayInterface(){}
