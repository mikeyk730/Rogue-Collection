import QtQuick 2.4
import RoguePlugin 1.0

Rectangle {
    id: root

    signal imagePainted
    signal simulateMousePress
    signal simulateMouseRelease
    signal simulateMouseDoubleClick
    signal simulateWheel
    signal simulateMouseMove

    property string colorScheme
    property bool enableBold
    property bool fullCursorHeight
    property bool antialiasText
    property int lineSpacing
    property bool terminalUsesMouse: true

    property alias monochrome: rogue.monochrome
    property alias title: rogue.title
    property alias font: rogue.font
    property alias terminalSize: rogue.screenSize
    property alias fontMetrics: rogue.fontSize

    property int naturalWidth: rogue.width + 2*rogue.padding
    property int naturalHeight: rogue.height + 2*rogue.padding

    function setGraphics(graphics){
        rogue.graphics = graphics;
    }

    onSimulateMousePress: {
        scope.focus = true;
    }

    color: 'black'

    FocusScope {
        id: scope
        anchors.fill: parent
        focus: true

        RogueWindow {
            id: rogue

            property int padding: 1
            x: padding
            y: padding
            anchors.centerIn: parent

            smooth: false
            onRendered: root.imagePainted()
            Component.onCompleted: appSettings.rogue = rogue

            onGraphicsChanged: appSettings.graphics = graphics
        }
    }
}
