import QtQuick 2.4

Rectangle {
    signal imagePainted

    property string colorScheme: "cool-retro-term"
    property bool enableBold: false
    property bool fullCursorHeight: false
    property bool antialiasText: false
    //property font font
    property alias font: test.font
    property int lineSpacing

    property size terminalSize: { Qt.size(80, 25) }
    property size fontMetrics: { Qt.size(8, 16) }

    color: 'black'

    Image {
        source: '../images/Codepage-437.png'
        //anchors.fill: parent
        width: 32 * 8 + 40
        height: 8 * 16 + 20
    }

    Text {
        id: test
        anchors.centerIn: parent
        text: qsTr("Welcome")
        font.pixelSize: 16
        color: '#aa3300'
    }
}
