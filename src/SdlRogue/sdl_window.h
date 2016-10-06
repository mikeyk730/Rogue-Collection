#pragma once
#include <memory>
#include "RogueCore\display_interface.h"

struct _CHAR_INFO;
struct _SMALL_RECT;
struct Coord;

struct SdlWindow : public DisplayInterface
{
    SdlWindow();
    ~SdlWindow();

    void Run();
    void ShutDown();

    virtual void SetDimensions(Coord dimensions);
    virtual void Draw(_CHAR_INFO* info);
    virtual void Draw(_CHAR_INFO* info, _SMALL_RECT rect);
    virtual void MoveCursor(Coord pos);
    virtual void SetCursor(bool enable);

    struct Impl;
    std::unique_ptr<Impl> m_impl;
};
