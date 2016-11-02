#include <QQmlEngine>
#include <QQmlContext>
#include "plugin.h"
#include "qrogue.h"
#include "game_config.h"

void RoguePlugin::registerTypes(const char *uri)
{
    qmlRegisterType<QRogue>(uri, 1, 0, "QmlRogue");
}

void RoguePlugin::initializeEngine(QQmlEngine *engine, const char *uri)
{
    QStringList titles;

    for (int i = 0; i < s_options.size(); ++i)
        titles.append(s_options[i].name.c_str());

    auto context = engine->rootContext();
    context->setContextProperty("gameTitles", QVariant::fromValue(titles));
}
