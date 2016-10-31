import QtQuick 2.4
import Rogue 1.0

Rectangle {
    signal imagePainted

    property string colorScheme: "cool-retro-term"
    property bool enableBold: false
    property bool fullCursorHeight: false
    property bool antialiasText: false
    property int lineSpacing

    property alias font: rogue.font
    property alias terminalSize: rogue.screenSize
    property alias fontMetrics: rogue.fontSize

    color: 'black'

    QmlRogue {
        id: rogue

        anchors.centerIn: parent

        width: screenSize.width * fontSize.width + 16
        height: screenSize.height * fontSize.height + 16

        //font.family: 'Consolas'
        font.pixelSize: 16
        smooth: false
        //scale: 2
    }
}

