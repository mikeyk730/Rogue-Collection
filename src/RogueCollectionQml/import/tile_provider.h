#pragma once
#include <vector>
#include <map>
#include <QSize>
#include <QFont>
#include <QPainter>
#include <QBitmap>
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
public:
    TileProvider(const TileConfig& config);
    virtual QSize TileSize() const override;
    virtual void PaintTile(QPainter* painter, QRect r, int ch, int color) override;

private:
    int TitleIndex(unsigned int ch, unsigned int color);
    bool UseInverse(unsigned int color);
    QRect GetTileRect(int i, bool use_inverse);

    TileConfig config_;
    QImage tilemap_;
    QSize tile_size_;
};

class TextProvider : public ITileProvider
{
public:
    TextProvider(const TextConfig& config);
    virtual QSize TileSize() const override;
    virtual void PaintTile(QPainter* painter, QRect r, int ch, int color) override;
private:
    QRect GetTextRect(unsigned int ch);

    TextConfig config_;
    QBitmap mask_;
    QSize tile_size_;
};
