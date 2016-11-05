#include <map>
#include <sstream>
#include <QColor>
#include <QRectF>
#include <QKeyEvent>
#include <pc_gfx_charmap.h>
#include "qrogue_display.h"
#include "qrogue_input.h"
#include "dos_to_unicode.h"
#include "qrogue.h"
#include "environment.h"
#include "tile_provider.h"

namespace
{
    const int kMaxQueueSize = 10;

    uint32_t CharText(uint32_t ch)
    {
        return ch & 0x0000ffff;
    }

    uint32_t CharColor(uint32_t ch)
    {
        return (ch >> 24) & 0xff;
    }

    bool IsText(uint32_t ch)
    {
        return (ch & 0x010000) == 0;
    }

    bool BlinkChar(uint32_t ch)
    {
        return CharText(ch) == STAIRS;
    }

    std::map<int, int> unix_chars = {
        { PASSAGE,   '#' },
        { DOOR,      '+' },
        { FLOOR,     '.' },
        { PLAYER,    '@' },
        { TRAP,      '^' },
        { STAIRS,    '%' },
        { GOLD,      '*' },
        { POTION,    '!' },
        { SCROLL,    '?' },
        { FOOD,      ':' },
        { STICK,     '/' },
        { ARMOR,     ']' },
        { AMULET,    ',' },
        { RING,      '=' },
        { WEAPON,    ')' },
        { VWALL,     '|' },
        { HWALL,     '-' },
        { ULWALL,    '-' },
        { URWALL,    '-' },
        { LLWALL,    '-' },
        { LRWALL,    '-' },
        { 204,       '|' },
        { 185,       '|' },
    };
}

QRogueDisplay::QRogueDisplay(QRogue* parent, Coord screen_size)
    : parent_(parent)
{
    screen_size_ = QSize(screen_size.x, screen_size.y);

    auto font = QFont("Px437 IBM VGA8");
    font.setPixelSize(16);
    font_provider_.reset(new FontProvider(font));
}

void QRogueDisplay::SetDimensions(Coord dimensions)
{

}

void QRogueDisplay::UpdateRegion(uint32_t *buf)
{
    UpdateRegion(buf, FullRegion());
}

void QRogueDisplay::UpdateRegion(uint32_t *buf, Region rect)
{
    std::lock_guard<std::mutex> lock(mutex_);

    //If we're behind on rendering, clear the queue and do a single full render.
    if ((int)shared_.render_regions.size() > kMaxQueueSize)
    {
        shared_.render_regions.clear();
        rect = FullRegion();
    }

    shared_.render_regions.push_back(rect);
    if (shared_.render_regions.size() == 1){
        PostRenderEvent(false);
    }

    if(!shared_.data) {
        shared_.data.reset(new uint32_t[TotalChars()]);
        shared_.dimensions = { screen_size_.width(), screen_size_.height() };
    }
    memcpy(shared_.data.get(), buf, TotalChars() * sizeof(int32_t));
}

void QRogueDisplay::MoveCursor(Coord pos)
{
    std::lock_guard<std::mutex> lock(mutex_);
    shared_.cursor_pos = pos;
}

void QRogueDisplay::SetCursor(bool enable)
{
    std::lock_guard<std::mutex> lock(mutex_);
    shared_.show_cursor = enable;
}

QSize QRogueDisplay::ScreenSize() const
{
    return screen_size_;
}

QSize QRogueDisplay::ScreenPixelSize() const
{
    return QSize(screen_size_.width() * TileSize().width(), screen_size_.height() * TileSize().height());
}

QRect QRogueDisplay::ScreenRect() const
{
    return QRect(QPoint(0,0), ScreenPixelSize());
}

QFont QRogueDisplay::Font() const
{
    return font_provider_->Font();
}

void QRogueDisplay::SetFont(const QFont &font)
{
    font_provider_.reset(new FontProvider(font));
    parent_->tileSizeChanged();
    PostRenderEvent(true);
}

bool QRogueDisplay::Monochrome() const
{
    return monochrome_;
}

void QRogueDisplay::SetMonochrome(bool enable)
{
    monochrome_ = enable;
}

void QRogueDisplay::SetScreenSize(Coord screen_size)
{
    screen_size_ = QSize(screen_size.x, screen_size.y);
}

void QRogueDisplay::SetGameConfig(const GameConfig &config, Environment* env)
{
    config_ = config;

    std::string gfx;
    if (env->Get("gfx", &gfx)) {
        for (size_t i = 0; i < config_.gfx_options.size(); ++i)
        {
            if (config_.gfx_options[i].name == gfx) {
                gfx_index_ = i;
                break;
            }
        }
    }
    LoadAssets();
}

const GraphicsConfig& QRogueDisplay::Gfx() const
{
    return config_.gfx_options[gfx_index_];
}

bool QRogueDisplay::HandleKeyEvent(QKeyEvent *event)
{
    if (event->text() == "`") {
        NextGfxMode();
        return true;
    }
    return false;
}

void QRogueDisplay::NextGfxMode()
{
    gfx_index_ = (gfx_index_ + 1) % config_.gfx_options.size();
    LoadAssets();
    PostRenderEvent(true);
}

QSize QRogueDisplay::TileSize() const
{
    return TilePainter()->TileSize();
}

void QRogueDisplay::Render(QPainter *painter)
{ 
    std::unique_lock<std::mutex> lock(mutex_);

    if (!shared_.data){
        painter->fillRect(ScreenRect(), QColor("black"));
        return;
    }

    ThreadData copy(shared_);
    shared_.render_regions.clear();

    lock.unlock();

    if (!screen_buffer_) {
        screen_buffer_.reset(new QPixmap(ScreenPixelSize()));
        copy.render_regions.clear();
        copy.render_regions.push_back(FullRegion());
    }

    QPainter screen_painter(screen_buffer_.get());
    for (auto i = copy.render_regions.begin(); i != copy.render_regions.end(); ++i)
    {
        RenderRegion(&screen_painter, copy.data.get(), *i);
    }

    painter->drawPixmap(0, 0, *screen_buffer_);

    std::string counter;
    if (parent_->Input()->GetRenderText(&counter))
        RenderCounterOverlay(painter, counter, 0);

    if (copy.show_cursor) {
        RenderCursor(painter, copy.cursor_pos);
    }
}

void QRogueDisplay::RenderRegion(QPainter *painter, uint32_t *data, Region rect)
{
    for (int y = rect.Top; y <= rect.Bottom; ++y) {
        for (int x = rect.Left; x <= rect.Right; ++x) {
            uint32_t info = data[Index(x,y)];
            PaintChar(painter, x, y, CharText(info), CharColor(info), IsText(info));
        }
    }
}

void QRogueDisplay::RenderCursor(QPainter *painter, Coord cursor_pos)
{
    if (frame_ % 2)
        return;

    auto w = TileSize().width();
    auto h = TileSize().height();
    QRectF r(w*cursor_pos.x, h*cursor_pos.y + 4*h/5, w, h/5);
    painter->fillRect(r, Colors::grey());
}

void QRogueDisplay::RenderCounterOverlay(QPainter* painter, const std::string& label, int n)
{
    std::ostringstream ss;
    ss << label;
    if (n > 0)
        ss << n;
    std::string s(ss.str());

    size_t len = s.size();
    for (size_t i = 0; i < len; ++i) {
        int x = screen_size_.width() - (len - i) - 1;
        int y = screen_size_.height() - 1;
        PaintChar(painter, x, y, s[i], 0x70, true);
    }
}

unsigned int GetTileColor(int ch, int color)
{
    //if it is inside a room
    if (color == 0x07 || color == 0)
        switch (ch)
        {
        case DOOR:
        case VWALL:
        case HWALL:
        case ULWALL:
        case URWALL:
        case LLWALL:
        case LRWALL:
            return 0x06; //brown
        case FLOOR:
            return 0x0a; //light green
        case STAIRS:
            return 0x20; //black on light green
        case TRAP:
            return 0x05; //magenta
        case GOLD:
        case PLAYER:
            return 0x0e; //yellow
        case POTION:
        case SCROLL:
        case STICK:
        case ARMOR:
        case AMULET:
        case RING:
        case WEAPON:
            return 0x09; //light blue
        case FOOD:
            return 0x04; //red
        }

    //if inside a passage or a maze
    else if (color == 0x70)
        switch (ch)
        {
        case FOOD:
            return 0x74; //red on grey
        case GOLD:
        case PLAYER:
            return 0x7e; //yellow on grey
        case POTION:
        case SCROLL:
        case STICK:
        case ARMOR:
        case AMULET:
        case RING:
        case WEAPON:
            return 0x71; //blue on grey
        }

    return color;
}

void QRogueDisplay::PaintChar(QPainter *painter, int x, int y, int ch, int color, bool is_text)
{
    // Hack for consistent standout in msg lines.  Unix versions use '-'.
    // PC uses ' ' with background color.  We want consistent behavior.
    if (y == 0 && color == 0x70) {
        if (Gfx().use_standout && ch == '-')
            ch = ' ';
        else if (!Gfx().use_standout && ch == ' ')
            ch = '-';
    }

    // Tiles from Unix versions come in with either color=0x00 (for regular state)
    // or color=0x70 (for standout).  We need to translate these into more diverse
    // colors.  Tiles from PC versions already have the correct color, so we
    // technically don't need to do anything here, but it doesn't hurt to set
    // the colors again.
    if (!is_text) {
        color = GetTileColor(ch, color);
    }
    color = TranslateColor(color, is_text);

    ch = TranslateChar(ch, is_text);

    auto w = TileSize().width();
    auto h = TileSize().height();
    QRect r(w*x, h*y, w, h);
    if (is_text)
        TextPainter()->PaintTile(painter, r, ch, color);
    else
        TilePainter()->PaintTile(painter, r, ch, color);
}

int QRogueDisplay::TranslateChar(int ch, bool is_text) const
{
    if (!is_text && tile_provider_)
        return ch;

    if (Gfx().animate && frame_%2 == 1 && BlinkChar(ch)){
        ch = ' ';
    }

    if (!is_text && Gfx().use_unix_gfx){
        auto i = unix_chars.find(ch);
        if (i != unix_chars.end()) {
            ch = i->second;
        }
    }

    if (text_provider_)
        return ch;

    return DosToUnicode(ch);
}

int QRogueDisplay::TranslateColor(int color, bool is_text) const
{    
    if (!color)
        color = 0x07;
    if (!Gfx().use_colors || monochrome_) {
        if (Gfx().use_standout && ((color>>4) == 0x07 || (Gfx().use_colors && color > 0x0f)))
            color = 0x70;
        else
            color = 0x07;
    }
    return color;
}

int QRogueDisplay::Index(int x, int y) const
{
    return y*screen_size_.width() + x;
}

ITileProvider *QRogueDisplay::TilePainter() const
{
    if (tile_provider_)
        return tile_provider_.get();
    else if (text_provider_)
        return text_provider_.get();
    return font_provider_.get();
}

ITileProvider *QRogueDisplay::TextPainter() const
{
    if (text_provider_)
        return text_provider_.get();
    return font_provider_.get();
}

void QRogueDisplay::Animate()
{
    ++frame_;
    bool update = false;
    if (Gfx().animate && screen_buffer_) {

        std::unique_lock<std::mutex> lock(mutex_);
        ThreadData copy(shared_);
        lock.unlock();

        for (int i = 0; i < TotalChars(); ++i) {
            uint32_t info = copy.data[i];
            if (!BlinkChar(info))
                continue;

            int x = i % copy.dimensions.x;
            int y = i / copy.dimensions.x;

            QPainter screen_painter(screen_buffer_.get());
            PaintChar(&screen_painter, x, y, CharText(info), CharColor(info), IsText(info));
            update = true;
        }
    }

    std::unique_lock<std::mutex> lock(mutex_);
    bool show_cursor = shared_.show_cursor;
    lock.unlock();

    if (show_cursor) {
        update = true;
    }

    if (update)
        PostRenderEvent(false);
}

void QRogueDisplay::PostRenderEvent(bool rerender)
{
    if (rerender)
        screen_buffer_.reset();

    parent_->postRender();
}

Region QRogueDisplay::FullRegion() const
{
    Region r;
    r.Left = 0;
    r.Top = 0;
    r.Right = short(screen_size_.width() - 1);
    r.Bottom = short(screen_size_.height() - 1);
    return r;
}

int QRogueDisplay::TotalChars() const
{
    return screen_size_.width() * screen_size_.height();
}

void QRogueDisplay::LoadAssets()
{
    tile_provider_.reset();
    if (Gfx().tiles) {
        tile_provider_.reset(new TileProvider(*Gfx().tiles));
    }

    text_provider_.reset();
    if (Gfx().text) {
        text_provider_.reset(new TextProvider(*Gfx().text));
    }

    parent_->tileSizeChanged();
}

QRogueDisplay::ThreadData::ThreadData(QRogueDisplay::ThreadData &other)
{
    dimensions = other.dimensions;

    int total = dimensions.x * dimensions.y;
    data.reset(new uint32_t[total]);
    memcpy(data.get(), other.data.get(), total * sizeof(uint32_t));

    show_cursor = other.show_cursor;
    cursor_pos = other.cursor_pos;
    render_regions = other.render_regions;
}
