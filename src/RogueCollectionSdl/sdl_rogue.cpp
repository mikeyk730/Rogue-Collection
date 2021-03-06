#include <sstream>
#include <fstream>
#include <ctime>
#include <SDL.h>
#include "sdl_rogue.h"
#include "sdl_utility.h"
#include "environment.h"
#include "sdl_display.h"
#include "sdl_input.h"
#include "pipe_input.h"
#include "pipe_output.h"
#include "utility.h"
#include "args.h"

const char* SdlRogue::kWindowTitle = "Rogue Collection 1.3.0";
const unsigned char SdlRogue::kSaveVersion = 2;

SdlRogue::SdlRogue(SDL_Window* window, SDL_Renderer* renderer, std::shared_ptr<Environment> current_env, const std::string& file) :
    m_current_env(current_env)
{
    RestoreGame(file);
    m_display.reset(
        new SdlDisplay(
            window,
            renderer,
            m_current_env.get(),
            m_game_env.get(),
            m_options,
            m_input.get(),
            nullptr));
}

SdlRogue::SdlRogue(SDL_Window* window, SDL_Renderer* renderer, std::shared_ptr<Environment> env, const GameConfig& game, const Args& args) :
    m_current_env(env),
    m_game_env(env)
{
    std::ostringstream ss;
    if (args.seed != "") {
        ss << args.seed;
    }
    else {
        ss << (int)time(0);
    }
    m_game_env->Set("seed", ss.str());

    SetGame(game);

    if (args.rogomatic)
    {
        //todo:mdk turn into decorator that can be cancelled with ESC
        m_input.reset(new PipeInput(m_current_env.get(), m_game_env.get(), m_options, args.GetDescriptorToRogue()));
    }
    else
    {
        m_input.reset(new SdlInput(m_current_env.get(), m_game_env.get(), m_options));
    }

    int frogue = args.rogomatic ? args.GetDescriptorFromRogue() : 0;
    std::unique_ptr<PipeOutput> pipe_output(frogue ? new PipeOutput(frogue) : nullptr);
    m_display.reset(
        new SdlDisplay(
            window,
            renderer,
            m_current_env.get(),
            m_game_env.get(),
            m_options,
            nullptr,
            std::move(pipe_output)));
}

SdlRogue::~SdlRogue()
{
}

SdlDisplay* SdlRogue::Display() const
{
    return m_display.get();
}

InputInterface * SdlRogue::Input() const
{
    return m_input.get();
}

Environment * SdlRogue::GameEnv() const
{
    return m_game_env.get();
}

GameConfig SdlRogue::Options() const
{
    return m_options;
}

void SdlRogue::Run()
{
    auto sdl_input = dynamic_cast<SdlInput*>(m_input.get());

    SdlDisplay::RegisterEvents();

    SDL_Event e;
    while (SDL_WaitEvent(&e)) {
        if (m_display->HandleEvent(e))
            continue;

        if (e.type == SDL_QUIT) {
            std::string value;
            if (m_current_env->Get("autosave", &value) && value == "true") {
                std::string name = "autosave-" + GetTimestamp() + ".sav";
                SaveGame(name, false);
            }
            return;
        }
        else if (e.type == SDL_KEYDOWN) {
            if (e.key.keysym.sym == 's' && (e.key.keysym.mod & KMOD_CTRL)) {
                if (SupportsSave()) {
                    SaveGame("", true);
                    continue;
                }
            }
        }

        if (sdl_input) {
            sdl_input->HandleEvent(e);
        }
    }
}

void SdlRogue::PostQuit()
{
    SDL_Event e;
    SDL_zero(e);
    e.type = SDL_QUIT;
    SDL_PushEvent(&e);
}

void SdlRogue::SetGame(const std::string& name)
{
    int i = GetGameIndex(name);
    if (i < 0)
    {
        throw_error("Save file specified unknown game: " + name);
    }

    auto config = GetGameConfig(i);
    if (!config.supports_save) {
        throw_error(config.name + " doesn't support saving.");
    }

    SetGame(config);
}

void SdlRogue::SetGame(const GameConfig& game)
{
    m_options = game;

    if (m_options.name == "PC Rogue 1.1") {
        m_game_env->Set("emulate_version", "1.1");
    }

    if (!m_game_env->WriteToOs(m_options.is_unix))
        throw_error("Couldn't write environment");
}

bool SdlRogue::SupportsSave() const
{
    return m_options.supports_save;
}

void SdlRogue::SaveGame(std::string path, bool notify)
{
    if (!SupportsSave()) {
        if (notify) {
            DisplayMessage(SDL_MESSAGEBOX_ERROR, "Save Game", m_options.name + " doesn't support saving.");
        }

        return;
    }

    if (path.empty()) {
        if (!m_current_env->Get("savefile", &path) || path.empty()) {
            if (!m_display->GetSavePath(path)) {
                return;
            }
        }
    }

    std::ofstream file(path, std::ios::binary | std::ios::out);
    if (!file) {
        DisplayMessage(SDL_MESSAGEBOX_ERROR, "Save Game", "Couldn't open save file: " + path);
        return;
    }

    Write(file, SdlRogue::kSaveVersion);
    Write(file, m_restore_count);
    WriteShortString(file, m_options.name);
    m_game_env->Serialize(file);
    m_input->SaveGame(file);

    if (!file) {
        DisplayMessage(SDL_MESSAGEBOX_ERROR, "Save Game", "Error writing to file: " + path);
        return;
    }
    if (notify) {
        DisplayMessage(SDL_MESSAGEBOX_INFORMATION, "Save Game", "Your game was saved successfully.  Come back soon!");
    }

    std::string value;
    if (m_current_env->Get("exit_on_save", &value) && value != "false")
        PostQuit();
}

void SdlRogue::RestoreGame(const std::string& path)
{
    std::ifstream file(path, std::ios::binary | std::ios::in);
    if (!file) {
        throw_error("Couldn't open save file: " + path);
    }

    unsigned char version;
    Read(file, &version);
    if (version > SdlRogue::kSaveVersion)
        throw_error("This file is not recognized.  It may have been saved with a newer version of Rogue Collection.  Please download the latest version and try again.");

    Read(file, &m_restore_count);
    ++m_restore_count;

    std::string name;
    ReadShortString(file, &name);

    // set up game environment
    m_game_env.reset(new Environment());
    m_game_env->Deserialize(file);
    m_game_env->Set("in_replay", "true");
    std::string value;
    if (m_current_env->Get("logfile", &value)) {
        m_game_env->Set("logfile", value);
    }
    if (version == 1) {
        m_game_env->Set("trap_bugfix", "false");
        m_game_env->Set("room_bugfix", "false");
        m_game_env->Set("confused_bugfix", "false");
    }

    SetGame(name);

    m_input.reset(new SdlInput(m_current_env.get(), m_game_env.get(), m_options));
    m_input->RestoreGame(file);

    if (m_current_env->Get("delete_on_restore", &value) && value == "true") {
        file.close();
        std::remove(path.c_str());
    }
}
