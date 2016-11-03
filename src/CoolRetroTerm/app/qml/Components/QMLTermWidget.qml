import QtQuick 2.4
import Rogue 1.0

Rectangle {
    id: root

    signal imagePainted
    signal simulateMousePress
    signal simulateMouseRelease
    signal simulateMouseDoubleClick

    property string colorScheme: "cool-retro-term"
    property bool enableBold: false
    property bool fullCursorHeight: false
    property bool antialiasText: false
    property int lineSpacing

    property alias font: rogue.font
    property alias terminalSize: rogue.screenSize
    property alias fontMetrics: rogue.fontSize

    onSimulateMousePress: {
        console.log(root.width, root.height, rogue.width, rogue.height);
    }

    color: 'black'
    anchors.centerIn: parent

    RogueWindow {
        id: rogue

        onRendered: root.imagePainted()

        property int padding: 10
        x: padding
        y: padding

        //font.family: 'Consolas'
        font.pixelSize: 16
        smooth: false

        //anchors.centerIn: parent
        transform: Scale {
            xScale: Math.floor(10*((root.width-rogue.padding*2)/rogue.width))/10
            yScale: Math.floor(10*((root.height-rogue.padding*2)/rogue.height))/10
        }
    }
}
