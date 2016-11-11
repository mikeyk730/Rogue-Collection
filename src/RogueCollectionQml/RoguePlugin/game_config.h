#pragma once
#include <string>
#include <vector>
#include <QObject>
#include <QSize>
#include <QString>
#include <coord.h>

struct Environment;

struct FontConfig
{
    std::string fontfile;
    int size;
};

class TextConfig
{
public:
    TextConfig();
    TextConfig(const QString& imagefile, QSize layout);

    QString imagefile() const;
    QSize layout() const;

private:
    QString imagefile_;
    QSize layout_;
};

struct TileConfig
{
    std::string filename;
    int count;
    int states;
};

struct GraphicsConfig
{
    std::string name;
    TextConfig* text;
    TileConfig* tiles;
    bool use_unix_gfx;
    bool use_colors;
    bool animate;
};

struct GameConfig
{
    std::string name;
    std::string dll_name;
    Coord screen;
    Coord small_screen;
    bool emulate_ctrl_controls;
    bool is_unix;
    std::vector<GraphicsConfig> gfx_options;
};

extern std::vector<GameConfig> s_options;
GameConfig GetGameConfig(int i);
int GetGameIndex(const std::string& name);

