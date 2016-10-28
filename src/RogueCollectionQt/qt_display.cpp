#include "qt_display.h"
#include "qt_rogue.h"
#include "dos_to_unicode.h"
#include "ui_widget.h"
#include "environment.h"

namespace
{
    const unsigned int kMaxQueueSize = 1;

    uint32_t CharText(uint32_t ch)
    {
        return ch & 0x0000ffff;
    }

    uint32_t CharColor(uint32_t ch)
    {
        return (ch >> 24) & 0xff;
    }
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


class SimpleColoredText : public QGraphicsSimpleTextItem
{
    public:
        SimpleColoredText(const QString &text, QColor fg, QColor bg) :
            QGraphicsSimpleTextItem(text),
            bg(bg)
        {
            setBrush(fg);
        }

        void paint( QPainter *painter, const QStyleOptionGraphicsItem *o, QWidget *w) {
            painter->fillRect(boundingRect(), bg);
            QGraphicsSimpleTextItem::paint(painter, o, w);
        }

        QColor bg;
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

void DrawChar(QGraphicsScene* scene, QFont& font, int ch, int color, int x, int y)
{
    QChar unicode = DosToUnicode(ch);
    auto text = new SimpleColoredText(unicode, GetFg(color), GetBg(color));
    text->setFont(font);
    auto r = text->boundingRect();
    text->setPos(x*r.width(),y*r.height());
    scene->addItem(text);
}

QtRogueDisplay::QtRogueDisplay(QtRogue *p) :
    parent_(p),
    ui(new Ui::Widget)
{
    ui->setupUi(parent_);
    ui->graphicsView->scale(2,2);

    dimensions_ = { p->GameEnv()->Columns(), p->GameEnv()->Lines() };

    //auto font = QFont("Consolas");
    font_ = QFont("Px437 IBM VGA8");
    font_.setPixelSize(16);
    font_.setStyleStrategy(QFont::NoAntialias);
}

bool QtRogueDisplay::HandleEvent(QEvent *event)
{
    if (event->type() == QEvent::User)
    {
        Render(false);
        return true;
    }
    return false;
}

void QtRogueDisplay::Render(bool force)
{
    std::vector<Region> regions;
    std::unique_ptr<uint32_t[]> data;
    Coord cursor_pos;
    bool show_cursor;
    {
        std::lock_guard<std::mutex> lock(mutex_);

        if (!shared_.data)
            return;
        if (shared_.render_regions.empty() && !force)
            return;

        uint32_t* temp = new uint32_t[TotalChars()];
        memcpy(temp, shared_.data.get(), TotalChars() * sizeof(uint32_t));
        data.reset(temp);

        regions = shared_.render_regions;
        shared_.render_regions.clear();

        show_cursor = shared_.show_cursor;
        cursor_pos = shared_.cursor_pos;
    }

    scene = new QGraphicsScene(parent_);

    if (force) {
//        scene = new QGraphicsScene(parent_);
        regions.push_back(FullRegion());
    }


    //todo:
    scene->setSceneRect(0, 0, 640, 400);


    for (auto i = regions.begin(); i != regions.end(); ++i)
    {
        RenderRegion(data.get(), *i);
    }

//    if (show_cursor) {
//        RenderCursor(cursor_pos);
//    }

    //std::string counter;
    //if (input_ && input_->GetRenderText(&counter))
    //    RenderCounterOverlay(counter, 0);

    ui->graphicsView->setScene(scene);
}

void QtRogueDisplay::RenderRegion(uint32_t *data, Region rect)
{
    //rect.Bottom = 15;
    //rect.Right = 30;
    for (int y = rect.Top; y <= rect.Bottom; ++y) {
        for (int x = rect.Left; x <= rect.Right; ++x) {
            uint32_t info = data[y*dimensions_.x + x];
            int color = CharColor(info);
            int ch = CharText(info);
            //todo: --more-- standout hack
            DrawChar(scene, font_, ch, color, x, y);
        }
    }
}

void QtRogueDisplay::PostRenderEvent()
{
    QEvent* e = new QEvent(QEvent::User);
    QCoreApplication::postEvent(parent_, e);
}

void QtRogueDisplay::SetDimensions(Coord dimensions)
{

}

void QtRogueDisplay::UpdateRegion(uint32_t *buf)
{
    UpdateRegion(buf, FullRegion());
}

void QtRogueDisplay::UpdateRegion(uint32_t *buf, Region rect)
{
    std::lock_guard<std::mutex> lock(mutex_);

    //If we're behind on rendering, clear the queue and do a single full render.
    if (shared_.render_regions.size() > kMaxQueueSize)
    {
        shared_.render_regions.clear();
        shared_.render_regions.push_back(FullRegion());
    }
    else {
        shared_.render_regions.push_back(rect);
        PostRenderEvent();
    }

    if(!shared_.data)
        shared_.data.reset(new uint32_t[TotalChars()]);
    memcpy(shared_.data.get(), buf, TotalChars() * sizeof(int32_t));
}

void QtRogueDisplay::MoveCursor(Coord pos)
{

}

void QtRogueDisplay::SetCursor(bool enable)
{

}

Region QtRogueDisplay::FullRegion() const
{
    Region r;
    r.Left = 0;
    r.Top = 0;
    r.Right = short(dimensions_.x - 1);
    r.Bottom = short(dimensions_.y - 1);
    return r;
}

int QtRogueDisplay::TotalChars() const
{
    return dimensions_.x * dimensions_.y;
}

