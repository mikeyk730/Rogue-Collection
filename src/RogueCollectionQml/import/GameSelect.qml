import QtQuick 2.0

Rectangle {
    anchors.centerIn: parent
    color: 'black'
    Image {
        x: 215
        y: 50

        scale: 1.6

        source: "assets/rogue.png"
        smooth: false
    }
    ListView {
        x: 25
        y: 250

        width: 125
        height: 200

        model: gameTitles
        delegate: Rectangle {
            color: 'black'

            height: 25
            width: 200
            Text {
                color: 'white'
                text: modelData
            }
        }
    }
}
