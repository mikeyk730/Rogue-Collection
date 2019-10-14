#ifndef QROGUE_H
#define QROGUE_H

#include <memory>
#include <atomic>
#include <QQuickPaintedItem>
#include "game_config.h"

struct Environment;
struct ReplayableInput;
class QRogueDisplay;
struct Args;

class QRogue : public QQuickPaintedItem
{
    Q_OBJECT
    Q_PROPERTY(bool monochrome READ monochrome WRITE setMonochrome)
    Q_PROPERTY(QFont font READ font WRITE setFont)
    Q_PROPERTY(QSize fontSize READ fontSize NOTIFY fontSizeChanged)
    Q_PROPERTY(QSize screenSize READ screenSize NOTIFY screenSizeChanged)
    Q_PROPERTY(QString game READ game WRITE setGame NOTIFY gameChanged)
    Q_PROPERTY(QString graphics READ graphics WRITE setGraphics NOTIFY graphicsChanged)
    Q_PROPERTY(bool sound READ sound WRITE setSound)

public:
    QRogue(QQuickItem *parent = nullptr);
    virtual ~QRogue() override;

    QFont font() const;
    void setFont(const QFont& font);

    bool sound() const;
    void setSound(bool enable);

    bool monochrome() const;
    void setMonochrome(bool enable);

    QSize fontSize() const;
    QSize screenSize() const;
    QString game() const;
    void setGame(const QString& game);
    void setGame(int index, Args& args);

    Q_INVOKABLE bool showTitleScreen();
    Q_INVOKABLE void restoreGame(const QUrl& url);
    Q_INVOKABLE void saveGame(const QUrl& url);
    Q_INVOKABLE void nextGraphicsMode();
    Q_INVOKABLE void autosave();

    QString graphics() const;
    void setGraphics(const QString& gfx);

    virtual void paint(QPainter *painter) override;
    void postRender();
    void tileSizeChanged();

Q_SIGNALS:
    void rendered();

public slots:
    void onTimer();
    void playSound(const QString& id);

signals:
    void render();
    void soundEvent(const QString& id);

    void fontSizeChanged(int height, int width);
    void screenSizeChanged(int height, int width);
    void gameChanged(const QString& name);
    void graphicsChanged(const QString& gfx);

public:
    Environment* GameEnv() const;
    Environment* CurrentEnv() const;
    ReplayableInput* Input() const;
    QRogueDisplay* Display() const;
    GameConfig Config() const;
    int Lines() const;
    int Columns() const;

protected:
    virtual void keyPressEvent(QKeyEvent *event) override;

private:
    void LaunchGame(Args& args);
    void SaveGame(std::string path, bool notify);
    void RestoreGame(const std::string& path);
    bool GetSavePath(std::string& filename);

    static const unsigned char kSaveVersion;

    GameConfig config_;
    std::shared_ptr<Environment> env_;
    std::shared_ptr<Environment> game_env_;
    std::unique_ptr<ReplayableInput> input_;
    std::unique_ptr<QRogueDisplay> display_;
    uint16_t restore_count_ = 0;
    std::atomic<bool> thread_exited_;
};

#endif

