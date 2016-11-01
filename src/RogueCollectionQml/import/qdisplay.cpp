#include <map>
#include <QColor>
#include <QRectF>
#include <QRect>
#include <pc_gfx_charmap.h>
#include "qdisplay.h"
#include "dos_to_unicode.h"
#include "environment.h"
#include "qrogue.h"

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

    bool use_unix_gfx = false;
    bool use_colors = true;
    bool use_standout = true;
    bool animate = true;

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

QRogueDisplay::QRogueDisplay(QRogue* parent, Coord screen_size)
    : parent_(parent)
{
    screen_size_ = QSize(screen_size.x, screen_size.y);

    auto font = QFont("Px437 IBM VGA8");
    font.setPixelSize(32);

    SetFont(font);
}

QSize QRogueDisplay::ScreenSize() const
{
    return screen_size_;
}

QSize QRogueDisplay::ScreenPixelSize() const
{
    return QSize(screen_size_.width() * font_size_.width(), screen_size_.height() * font_size_.height());
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

    screen_buffer_.reset();
    PostRenderEvent();
}

QSize QRogueDisplay::FontSize() const
{
    return font_size_;
}

void QRogueDisplay::Render(QPainter *painter)
{ 
    std::unique_lock<std::mutex> lock(mutex_);

    if (!shared_.data)
        return;

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

    if (copy.show_cursor) {
        RenderCursor(painter, copy.cursor_pos);
    }

    //std::string counter;
    //if (input_ && input_->GetRenderText(&counter))
    //    RenderCounterOverlay(counter, 0);
}

void QRogueDisplay::PaintChar(QPainter *painter, int x, int y, int ch, int color, bool is_text)
{
    auto w = font_size_.width();
    auto h = font_size_.height();
    QRectF r(w*x, h*y, w, h);

    if (animate && frame_%2 == 1 && BlinkChar(ch)){
        ch = ' ';
    }
    ch = TranslateChar(ch, is_text);
    color = TranslateColor(color);

    painter->fillRect(r, GetBg(color));
    painter->setFont(font_);
    painter->setRenderHint(QPainter::TextAntialiasing, false);
    painter->setPen(GetFg(color));
    painter->drawText(r, 0, QChar(ch));
}

int QRogueDisplay::TranslateChar(int ch, bool is_text) const
{
    if (!is_text && use_unix_gfx){
        auto i = unix_chars.find(ch);
        if (i != unix_chars.end()) {
            ch = i->second;
        }
    }
    return DosToUnicode(ch);
}

int QRogueDisplay::TranslateColor(int color) const
{
    if (!use_colors) {
        if (use_standout && color > 0x0F)
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

void QRogueDisplay::Animate()
{
    ++frame_;
    bool update = false;
    if (animate && screen_buffer_) {

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
        PostRenderEvent();

}

void QRogueDisplay::PostRenderEvent()
{
    parent_->postRender();
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
        PostRenderEvent();
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
