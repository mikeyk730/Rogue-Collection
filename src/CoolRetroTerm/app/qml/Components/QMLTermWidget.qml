import QtQuick 2.4

Rectangle {
    signal imagePainted

    property string colorScheme: "cool-retro-term"

    property bool enableBold: false
    property bool fullCursorHeight: true

    property size terminalSize: { Qt.size(80, 25) }
    property size fontMetrics: { Qt.size(8, 16) }

    width: 640
    height: 400

    color: '#222222'
    Image {
        source: '../images/Codepage-437.png'
        anchors.fill: parent
    }
    Text {
        id: test
        text: qsTr("Welcome")
        font.pixelSize: 64
        color: '#aa3300'
    }
}
