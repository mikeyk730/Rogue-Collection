import QtQuick 2.0
import QtQuick.Controls 1.3
import QtQuick.Window 2.2
import Rogue 1.0

ApplicationWindow
{
    id: window
    title: rogue.title
    color: 'black'

    property int windowScale: 2
    width: 640 * 2
    height: 400 * 2

    function setDimensions(){
        if (window.visibility === Window.Windowed){
            window.width = rogue.width * window.windowScale
            window.height = rogue.height * window.windowScale
        }
    }

    onVisibilityChanged: setDimensions()
    onWindowScaleChanged: setDimensions()

    RogueWindow{
        id: rogue
        anchors.centerIn: parent

        property bool maintainAspect: true

        function getXScale() { return window.width / rogue.width; }
        function getYScale() { return window.height / rogue.height; }
        function getScale() { return Math.min(getXScale(), getYScale()); }

        transform: Scale {
            xScale: rogue.maintainAspect ? rogue.getScale() : rogue.getXScale()
            yScale: rogue.maintainAspect ? rogue.getScale() : rogue.getYScale()
            origin.x: rogue.width/2
            origin.y: rogue.height/2
        }

        onWidthChanged: window.setDimensions()
        onHeightChanged: window.setDimensions()

        FontLoader { id: pcFont; source: "assets/Px437_IBM_VGA8.ttf" }
        FontLoader { id: unixFont; source: "assets/TerminusTTF-4.38.2.ttf" }

        font.family: graphics === 'unix' ? unixFont.name : pcFont.name
        font.pixelSize: graphics === 'unix' ? 12 : 16

        Keys.onPressed: {
            if (event.modifiers & Qt.AltModifier)
            {
                if (event.key >= Qt.Key_1 && event.key <= Qt.Key_9){
                    window.visibility = Window.Windowed;
                    window.windowScale = event.key - Qt.Key_0;
                }
                else if (event.key === Qt.Key_Return){
                    var to_fullscreen = window.visibility !== Window.FullScreen;
                    window.visibility = Window.Windowed;
                    if (to_fullscreen)
                        window.visibility = Window.FullScreen;
                }
                else if (event.key === Qt.Key_A){
                    maintainAspect = !maintainAspect
                }
            }
        }
    }
}
