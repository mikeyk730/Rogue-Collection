import QtQuick 2.0
import QtQuick.Controls 1.3
import QtQuick.Window 2.2
import Rogue 1.0


ApplicationWindow
{
    id: window
    title: rogue.title

    property int windowScale: 2
    property bool maintainAspect: true
    width: 640 * 2
    height: 400 * 2

    function setDimensions(){
        if (window.visibility !== Window.FullScreen){
            window.width = rogue.width * window.windowScale
            window.height = rogue.height * window.windowScale
        }
    }

    onVisibilityChanged: setDimensions()
    onWindowScaleChanged: setDimensions()

    Rectangle {
        id: root

        Keys.onPressed: {
            if (event.modifiers & Qt.AltModifier)
            {
                if (event.key >= Qt.Key_1 && event.key <= Qt.Key_9){
                    window.visibility = Window.Windowed;
                    window.windowScale = event.key - Qt.Key_0;
                }
                else if (event.key === Qt.Key_Return){
                    window.visibility = (window.visibility === Window.FullScreen) ? Window.Windowed : Window.FullScreen;
                }
                else if (event.key === Qt.Key_A){
                    window.maintainAspect = !window.maintainAspect
                }
            }
        }

        anchors.fill: parent
        color: 'black'

        function getXScale() { return window.width / rogue.width; }
        function getYScale() { return window.height / rogue.height; }
        function getScale() { return Math.min(getXScale(), getYScale()); }

        transform: Scale {
            xScale: window.maintainAspect ? root.getScale() : root.getXScale()
            yScale: window.maintainAspect ? root.getScale() : root.getYScale()
            origin.x: width/2
            origin.y: height/2
        }

        RogueWindow{
            onWidthChanged: window.setDimensions()
            onHeightChanged: window.setDimensions()
            id: rogue
            anchors.centerIn: parent
        }
    }
}
