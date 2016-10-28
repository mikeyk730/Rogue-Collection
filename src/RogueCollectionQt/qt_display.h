#ifndef QT_DISPLAY_H
#define QT_DISPLAY_H

#include <memory>
#include <mutex>
#include <vector>
#include <coord.h>
#include <QGraphicsScene>
#include <QGraphicsView>
#include <QGraphicsItem>
#include <display_interface.h>

namespace Ui {
class Widget;
}
class QtRogue;

class QtRogueDisplay : public DisplayInterface
{
public:
    QtRogueDisplay(QtRogue* p);

    bool HandleEvent(QEvent* event);
    void Render(bool force);
    void RenderRegion(uint32_t* data, Region rect);
    void PostRenderEvent();

    virtual void SetDimensions(Coord dimensions) override;
    virtual void UpdateRegion(uint32_t* buf) override;
    virtual void UpdateRegion(uint32_t* buf, Region rect) override;
    virtual void MoveCursor(Coord pos) override;
    virtual void SetCursor(bool enable) override;

    Region FullRegion() const;
    int TotalChars() const;

private:
    QGraphicsScene *scene;
    std::unique_ptr<Ui::Widget> ui;
    QtRogue* parent_;
    QFont font_;

    Coord dimensions_ = { 0, 0 };

    struct ThreadData
    {
        std::unique_ptr<uint32_t[]> data = 0;
        bool show_cursor = false;
        Coord cursor_pos = { 0, 0 };
        std::vector<Region> render_regions;
    };
    ThreadData shared_;
    std::mutex mutex_;
};

#endif // QT_DISPLAY_H
