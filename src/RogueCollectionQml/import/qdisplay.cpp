#include <map>
#include <sstream>
#include <QColor>
#include <QRectF>
#include <QKeyEvent>
#include <pc_gfx_charmap.h>
#include "qdisplay.h"
#include "qt_input.h"
#include "dos_to_unicode.h"
#include "qrogue.h"
#include "environment.h"

namespace Colors
{
    QColor black()    { return QColor(   0,   0,   0, 255 ); }
    QColor white()    { return QColor( 255, 255, 255, 255 ); }
    QColor grey()     { return QColor( 170, 170, 170, 255 ); }
    QColor d_grey()   { return QColor(  65,  65,  65, 255 ); }
    QColor l_grey()   { return QColor( 205, 205, 205, 255 ); }
    QColor red()      { return QColor( 170,   0,   0, 255 ); }
    QColor l_red()    { return QColor( 255,  85,  85, 255 ); }
    QColor green()    { return QColor(   0, 170,   0, 255 ); }
    QColor l_green()  { return QColor(  85,  255, 85, 255 ); }
    QColor blue()     { return QColor(   0,   0, 170, 255 ); }
    QColor l_blue()   { return QColor(  85,  85, 255, 255 ); }
    QColor cyan()     { return QColor(   0, 170, 170, 255 ); }
    QColor l_cyan()   { return QColor(  25, 255, 255, 255 ); }
    QColor magenta()  { return QColor( 170,   0, 170, 255 ); }
    QColor l_magenta(){ return QColor( 255,  25, 255, 255 ); }
    QColor yellow()   { return QColor( 255, 255,  25, 255 ); }
    QColor brown()    { return QColor( 170,  85,   0, 255 ); }
    QColor orange()   { return QColor( 234, 118,   2, 255 ); }
}

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

    QColor colors[] = {
        Colors::black(),
        Colors::blue(),
        Colors::green(),
        Colors::cyan(),
        Colors::red(),
        Colors::magenta(),
        Colors::brown(),
        Colors::grey(),
        Colors::d_grey(),
        Colors::l_blue(),
        Colors::l_green(),
        Colors::l_cyan(),
        Colors::l_red(),
        Colors::l_magenta(),
        Colors::yellow(),
        Colors::white()
    };

    QColor GetColor(int color)
    {
        return colors[color];
    }

    QColor GetFg(int color)
    {
        return GetColor(color & 0x0f);
    }

    QColor GetBg(int color)
    {
        return GetColor(color >> 4);
    }
}



QRogueDisplay::QRogueDisplay(QRogue* parent, Coord screen_size)
    : parent_(parent)
{
    screen_size_ = QSize(screen_size.x, screen_size.y);

    auto font = QFont("Px437 IBM VGA8");
    font.setPixelSize(32);

    SetFont(font);
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
    return QSize(screen_size_.width() * font_size_.width(), screen_size_.height() * font_size_.height());
}

QRect QRogueDisplay::ScreenRect() const
{
    return QRect(QPoint(0,0), ScreenPixelSize());
}

QFont QRogueDisplay::Font() const
{
    return font_;
}

void QRogueDisplay::SetFont(const QFont &font)
{
    font_ = font;
    font_.setStyleStrategy(QFont::NoAntialias);

    QFontMetrics font_metrics(font);
    font_size_.setWidth(font_metrics.width("W"));
    font_size_.setHeight(font_metrics.height());

    PostRenderEvent(true);
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
    PostRenderEvent(true);
}

QSize QRogueDisplay::FontSize() const
{
    return font_size_;
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

    std::string counter;
    if (parent_->Input()->GetRenderText(&counter))
        RenderCounterOverlay(&screen_painter, counter, 0);

    painter->drawPixmap(0, 0, *screen_buffer_);

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

    auto w = font_size_.width();
    auto h = font_size_.height();
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

unsigned int GetColor(int ch, int color)
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
    auto w = font_size_.width();
    auto h = font_size_.height();
    QRectF r(w*x, h*y, w, h);

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
    // technically don't need to do anything here, but it doesn't hurt to call
    // GetColor.
    if (!is_text) {
        color = GetColor(ch, color);
    }

    ch = TranslateChar(ch, is_text);
    color = TranslateColor(color, is_text);

    painter->fillRect(r, GetBg(color));
    painter->setFont(font_);
    painter->setRenderHint(QPainter::TextAntialiasing, false);
    painter->setPen(GetFg(color));
    painter->drawText(r, 0, QChar(ch));
}

int QRogueDisplay::TranslateChar(int ch, bool is_text) const
{
    if (Gfx().animate && frame_%2 == 1 && BlinkChar(ch)){
        ch = ' ';
    }

    if (!is_text && Gfx().use_unix_gfx){
        auto i = unix_chars.find(ch);
        if (i != unix_chars.end()) {
            ch = i->second;
        }
    }

    return DosToUnicode(ch);
}

int QRogueDisplay::TranslateColor(int color, bool is_text) const
{    
    if (!color)
        color = 0x07;
    if (!Gfx().use_colors) {
        if (Gfx().use_standout && color > 0x0F)
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
