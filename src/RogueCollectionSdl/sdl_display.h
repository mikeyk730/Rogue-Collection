#pragma once
#include <mutex>
#include <SDL.h>
#include <display_interface.h>
#include "sdl_rogue.h"
#include "sdl_utility.h"
#include "window_sizer.h"
#ifdef __linux__ //todo:mdk fix
#include "text_provider.h"
#include "tile_provider.h"
#endif

struct Environment;
struct ITextProvider;
struct TileProvider;
struct ReplayableInput;
struct PipeOutput;

struct SdlDisplay : public DisplayInterface
{
    const unsigned int kMaxQueueSize = 1;

    SdlDisplay(
        SDL_Window* window,
        SDL_Renderer* renderer,
        Environment* current_env,
        Environment* game_env,
        const GameConfig& options,
        ReplayableInput* input,
        int pipe_fd);
    ~SdlDisplay();

    //display interface
    virtual void SetDimensions(Coord dimensions) override;
    virtual void UpdateRegion(uint32_t* info, char* dirty) override;
    virtual void MoveCursor(Coord pos) override;
    virtual void SetCursor(bool enable) override;
    virtual void PlaySound(const std::string& id) override;
    virtual void DisplayMessage(const std::string& message) override;

    void SetTitle(const std::string& title);
    void NextGfxMode();
    bool GetSavePath(std::string& path);
    Coord GetDimensions() const;

    bool HandleEvent(const SDL_Event& e);

    static void RegisterEvents();
    static void PostRenderMsg(int force);

private:
    void UpdateRegion(uint32_t* buf);
    void UpdateRegion(uint32_t* info, Region rect);

    bool HandleWindowEvent(const SDL_Event& e);
    bool HandleRenderEvent(const SDL_Event& e);
    bool HandleTimerEvent(const SDL_Event& e);
    bool HandleEventKeyDown(const SDL_Event& e);
    bool HandleEventText(const SDL_Event& e);

    void LoadAssets();
    void RenderGame(bool force);
    void Animate();
    void RenderRegion(uint32_t* info, Region rect);
    void RenderText(uint32_t info, unsigned char color, SDL_Rect r, bool is_tile);
    void RenderTile(uint32_t info, SDL_Rect r);
    void RenderCursor(Coord pos);
    void RenderCounterOverlay(const std::string& s, int n);
    void UpdateWindow();

    const GraphicsConfig& graphics_cfg() const;

    Coord ScreenPosition(Coord buffer_pos);
    SDL_Rect ScreenRegion(Coord buffer_pos);

    Region FullRegion() const;
    int TotalChars() const;

private:
    SDL_Window* m_window = nullptr;
    SDL_Renderer* m_renderer = nullptr;
    Environment* m_current_env = nullptr;
    Environment* m_game_env = nullptr;
    ReplayableInput* m_input = nullptr;
    SDL::Scoped::Texture m_screen_texture;
    GameConfig m_options;

    Coord m_dimensions = { 0, 0 };

    Coord m_block_size = { 0, 0 };
    int m_gfx_mode = 0;
    WindowSizer m_sizer;
    std::unique_ptr<ITextProvider> m_text_provider;
    std::unique_ptr<TileProvider> m_tile_provider;
    int m_frame_number = 0;
    std::unique_ptr<PipeOutput> m_pipe_output;

    struct ThreadData
    {
        std::unique_ptr<uint32_t[]> data = 0;
        bool show_cursor = false;
        Coord cursor_pos = { 0, 0 };
        std::vector<Region> render_regions;
    };
    ThreadData m_shared;
    std::mutex m_mutex;
};
