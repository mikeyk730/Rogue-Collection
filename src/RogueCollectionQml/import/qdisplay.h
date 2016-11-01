#ifndef QT_DISPLAY_H
#define QT_DISPLAY_H

#include <memory>
#include <mutex>
#include <vector>
#include <QSize>
#include <QFont>
#include <QPainter>
#include <coord.h>
#include <display_interface.h>

class QRogue;

class QRogueDisplay : public DisplayInterface
{
public:
    QRogueDisplay(QRogue* parent, Coord screen_size);

    QFont Font() const;
    void SetFont(const QFont& font);

    QSize FontSize() const;
    QSize ScreenSize() const;
    QSize ScreenPixelSize() const;

    void Render(QPainter *painter);
    void RenderRegion(QPainter *painter, uint32_t* data, Region rect);
    void RenderCursor(QPainter *painter, Coord cursor_pos);
    void Animate();
    void PostRenderEvent();

    virtual void SetDimensions(Coord dimensions) override;
    virtual void UpdateRegion(uint32_t* buf) override;
    virtual void UpdateRegion(uint32_t* buf, Region rect) override;
    virtual void MoveCursor(Coord pos) override;
    virtual void SetCursor(bool enable) override;

    Region FullRegion() const;
    int TotalChars() const;

private:
    void PaintChar(QPainter *painter, int x, int y, int ch, int color, bool is_text);
    int TranslateChar(int ch, bool is_text) const;
    int TranslateColor(int color) const;
    int Index(int x, int y) const;

    QPainter& ScreenPainter();

    QFont font_;
    QSize font_size_;
    QSize screen_size_;
    QRogue* parent_;
    std::unique_ptr<QPixmap> screen_buffer_;
    int frame_ = 0;

    struct ThreadData
    {
        ThreadData() = default;
        ThreadData(ThreadData& other);

        Coord dimensions = { 0, 0 };
        std::unique_ptr<uint32_t[]> data = 0;
        std::vector<Region> render_regions;

        bool show_cursor = false;
        Coord cursor_pos = { 0, 0 };
    };
    ThreadData shared_;
    std::mutex mutex_;
};

#endif
