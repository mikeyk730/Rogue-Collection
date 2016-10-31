import QtQuick 2.0
import Rogue 1.0

Rectangle {
    id: root

    width: rogue.width
    height: rogue.height
    color: '#888888'

    MouseArea{
        anchors.fill: parent
        onClicked : {
            console.log("size", rogue.fontSize.width, rogue.fontSize.height)
        }
    }

    QmlRogue {
        id: rogue

        width: screenSize.width * fontSize.width
        height: screenSize.height * fontSize.height

        //font.family: 'Consolas'
        font.pixelSize: 16 * 2
    }
}

