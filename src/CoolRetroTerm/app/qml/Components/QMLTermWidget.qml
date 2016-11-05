import QtQuick 2.4
import Rogue 1.0

Rectangle {
    id: root

    signal imagePainted
    signal simulateMousePress
    signal simulateMouseRelease
    signal simulateMouseDoubleClick

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
        rogue.setGraphics(graphics);
    }

    //onSimulateMousePress: {
    //    console.log(root.width, root.height, root.naturalWidth, root.naturalHeight);
    //}

    color: 'black'

    RogueWindow {
        id: rogue

        property int padding: 1
        x: padding
        y: padding
        anchors.centerIn: parent

        smooth: false
        onRendered: root.imagePainted()
    }
}
