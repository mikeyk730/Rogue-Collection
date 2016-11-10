import QtQuick 2.4
import QtQuick.Controls 1.3
import QtQuick.Window 2.2
import QtQuick.Dialogs 1.0
import RoguePlugin 1.0

ApplicationWindow
{
    id: window
    title: rogue.title
    color: 'black'

    property int windowScale: 2

    function setDimensions(force){
        if (force === true || visibility === Window.Windowed){
            width = rogue.width * windowScale
            height = rogue.height * windowScale + window.__topBottomMargins
        }
    }

    Component.onCompleted: setDimensions(true)
    onWindowScaleChanged: setDimensions(false)


    RogueWindow{
        id: rogue
        anchors.centerIn: parent

        property bool maintainAspect: true

        function getXScale() { return parent.width / rogue.width; }
        function getYScale() { return parent.height / rogue.height; }
        function getScale() { return Math.min(getXScale(), getYScale()); }

        transform: Scale {
            xScale: rogue.maintainAspect ? rogue.getScale() : rogue.getXScale()
            yScale: rogue.maintainAspect ? rogue.getScale() : rogue.getYScale()
            origin.x: rogue.width/2
            origin.y: rogue.height/2
        }

        onWidthChanged: window.setDimensions(false)
        onHeightChanged: window.setDimensions(false)

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
            }
        }
    }

    AboutDialog{
        id: aboutDialog
        visible: false
    }

    Action {
        id: fullscreenAction
        text: qsTr("Fullscreen\t")
        shortcut: "Alt+Return"
        onTriggered: {
            var to_fullscreen = window.visibility !== Window.FullScreen;
            menu.visible = !to_fullscreen;
            window.visibility = Window.Windowed;
            if (to_fullscreen){
                window.visibility = Window.FullScreen;
            }
        }
    }
    Action {
        id: aspectAction
        text: qsTr("Maintain Aspect Ratio\t")
        shortcut: "Alt+A"
        onTriggered: rogue.maintainAspect = !rogue.maintainAspect
        checkable: true
        checked: rogue.maintainAspect

    }
    Action {
        id: changeGraphicsAction
        text: qsTr("Change Graphics\t")
        shortcut: "`"
        onTriggered: rogue.nextGraphicsMode()
        enabled: rogue.isGameRunning()
    }
    Action {
        id: saveAction
        text: qsTr("Save Game\t")
        shortcut: "Ctrl+S"
        onTriggered: rogue.saveGame()
        enabled: rogue.isGameRunning()
    }
    Action {
        id: quitAction
        text: qsTr("Quit\t")
        shortcut: "Alt+F4"
        onTriggered: Qt.quit();
    }
    Action{
        id: resetZoom
        text: qsTr("Reset Scale\t")
        shortcut: "Ctrl+0"
        onTriggered: {
            window.visibility = Window.Windowed;
            window.windowScale = 2;
            window.setDimensions();
        }
    }
    Action{
        id: zoomIn
        text: qsTr("Increase Scale\t")
        shortcut: "Ctrl++"
        onTriggered: {
            window.visibility = Window.Windowed;
            window.windowScale += 1;
        }
    }
    Action{
        id: zoomOut
        text: qsTr("Decrease Scale\t")
        shortcut: "Ctrl+-"
        onTriggered: {
            window.visibility = Window.Windowed;
            if (window.windowScale > 1)
                window.windowScale -= 1 ;
        }
    }
    Action{
        id: showAboutAction
        text: qsTr("About\t")
        onTriggered: {
            aboutDialog.show();
            aboutDialog.requestActivate();
            aboutDialog.raise();
        }
    }

    menuBar: MenuBar {
        id: menu
        property bool visible: true
        Menu {
            title: qsTr("File")
            visible: menu.visible
            MenuItem {action: saveAction}
            MenuItem {action: quitAction}
        }
        Menu{
            title: qsTr("View")
            visible: menu.visible
            MenuItem {action: fullscreenAction}
            MenuSeparator{}
            MenuItem {action: zoomIn}
            MenuItem {action: zoomOut}
            MenuItem {action: resetZoom}
            MenuSeparator{}
            MenuItem {action: aspectAction}
            MenuSeparator{}
            MenuItem {action: changeGraphicsAction}
        }
        Menu{
            title: qsTr("Help")
            visible: menu.visible
            MenuItem {action: showAboutAction}
        }
    }
}
