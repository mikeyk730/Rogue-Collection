#pragma once
#include <vector>
#include <map>
#include <QSize>
#include <QFont>
#include <QPainter>
#include "game_config.h"

struct ITileProvider
{
    virtual ~ITileProvider();
    virtual QSize TileSize() const = 0;
    virtual void PaintTile(QPainter* painter, QRect r, int ch, int color) = 0;
};

class FontProvider : public ITileProvider
{
public:
    FontProvider(const QFont& font);
    virtual QSize TileSize() const override;
    virtual void PaintTile(QPainter* painter, QRect r, int ch, int color) override;

    QFont Font() const;
    void SetFont(const QFont& font);

private:
    QFont font_;
    QSize font_size_;
};

class TileProvider : public ITileProvider
{
    TileProvider(const TileConfig& config);
    virtual QSize TileSize() const override;
    virtual void PaintTile(QPainter* painter, QRect r, int ch, int color) override;
};

class TextProvider : public ITileProvider
{
    TextProvider(const TextConfig& config);
    virtual QSize TileSize() const override;
    virtual void PaintTile(QPainter* painter, QRect r, int ch, int color) override;
};
