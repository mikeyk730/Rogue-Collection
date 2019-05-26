#pragma once
#include <vector>
#include <memory>
#include <QSize>
#include <QFont>
#include <QPainter>
#include <QBitmap>
#include <QPixmap>
#include "game_config.h"

class ITileProvider
{
public:
    virtual ~ITileProvider();
    virtual QSize TileSize() const = 0;
    virtual void PaintTile(QPainter* painter, QRect r, int ch, int color) = 0;
};

class FontProvider : public ITileProvider
{
public:
    FontProvider(const QFont& font);
    virtual ~FontProvider() override;
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
    virtual ~TileProvider() override;
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
    virtual ~TextProvider() override;
    virtual QSize TileSize() const override;
    virtual void PaintTile(QPainter* painter, QRect r, int ch, int color) override;
private:
    QRect GetTextRect(unsigned int ch);
    QPixmap* GetPixMap(int color);

    TextConfig config_;
    QBitmap mask_;
    std::map<int, std::unique_ptr<QPixmap>> images_;
    QSize image_size_;
    QSize tile_size_;
};
