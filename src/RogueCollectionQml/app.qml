import QtQuick 2.0
import QtQuick.Controls 1.3
import QtQuick.Window 2.2
import Rogue 1.0


ApplicationWindow
{
    id: window
    title: rogue.title

    property int windowScale: 2
    width: rogue.width * windowScale
    height: rogue.height * windowScale

    Rectangle {
        id: root

        Keys.onPressed: {
            if (event.modifiers & Qt.AltModifier)
            {
                if (event.key >= Qt.Key_1 && event.key <= Qt.Key_9)
                    window.windowScale = event.key - Qt.Key_0;
                else if (event.key === Qt.Key_Return){
                    window.visibility = (window.visibility === Window.FullScreen) ? Window.Windowed : Window.FullScreen
                }
            }
        }

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
