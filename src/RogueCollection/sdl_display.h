#pragma once
#include <mutex>
#include <SDL.h>
#include <display_interface.h>
#include "sdl_rogue.h"
#include "window_sizer.h"

struct Environment;
struct ITextProvider;
struct TileProvider;

struct SdlDisplay : public DisplayInterface
{
    const unsigned int kMaxQueueSize = 1;

    SdlDisplay(SDL_Window* window, SDL_Renderer* renderer, Environment* current_env, Environment* game_env, const GameConfig& options, SdlInput* input);

    //display interface
    virtual void SetDimensions(Coord dimensions) override;
    virtual void UpdateRegion(uint32_t* buf) override;
    virtual void UpdateRegion(uint32_t* info, Region rect) override;
    virtual void MoveCursor(Coord pos) override;
    virtual void SetCursor(bool enable) override;

    void SetTitle(const std::string& title);
    void NextGfxMode();
    bool GetSavePath(std::string& path);

    bool HandleEvent(const SDL_Event& e);

    static void RegisterEvents();
    static void PostRenderMsg(int force);

private:
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

    const GraphicsConfig& current_gfx() const;

    Coord ScreenPosition(Coord buffer_pos);
    SDL_Rect ScreenRegion(Coord buffer_pos);

    Region FullRegion() const;
    int TotalChars() const;

private:
    SDL_Window* m_window = 0;
    SDL_Renderer* m_renderer = 0;
    Environment* m_current_env = 0;
    Environment* m_game_env = 0;
    SdlInput* m_input = 0;
    GameConfig m_options;

    Coord m_dimensions = { 0, 0 };

    Coord m_block_size = { 0, 0 };
    int m_gfx_mode = 0;
    WindowSizer m_sizer;
    std::unique_ptr<ITextProvider> m_text_provider;
    std::unique_ptr<TileProvider> m_tile_provider;
    int m_frame_number = 0;

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
