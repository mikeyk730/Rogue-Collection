import QtQuick 2.0
import QtQuick.Controls 1.0
import Rogue 1.0


ApplicationWindow {

    title: rogue.title

    Rectangle {
        id: root

        anchors.fill: parent
        color: '#888888'
        transform: Scale {
            xScale: width/rogue.width
            yScale: height/rogue.height
            origin.x: width/2
            origin.y: height/2

        }

        RogueWindow{
            id: rogue
            anchors.centerIn: parent
        }
    }
}
