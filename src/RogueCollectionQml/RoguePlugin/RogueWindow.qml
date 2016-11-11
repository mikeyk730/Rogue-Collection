import QtQuick 2.4
import QtQuick.Dialogs 1.0
import RoguePlugin 1.0

Item {
    id: root

    property alias graphics: rogue.graphics
    property alias monochrome: rogue.monochrome
    property alias font: rogue.font
    property alias screenSize: rogue.screenSize
    property alias fontSize: rogue.fontSize
    property alias title: rogue.title
    property Item activeItem: gameSelectContainer

    function isGameRunning()
    {
        return activeItem == rogue;
    }

    function saveGame()
    {
        if (isGameRunning())
            saveDialog.open();
    }

    function nextGraphicsMode()
    {
        if (isGameRunning())
            rogue.nextGraphicsMode();
    }

    function setTextConfig(imagefile, layout) {
        rogue.setTextConfig(imagefile, layout);
    }

    function setTileConfig(filename, count, states) {
        rogue.setTileConfig(filename, count, states);
    }

    signal rendered

    width: activeItem.width
    height: activeItem.height

    Item {
        id: gameSelectContainer

        width: rogue.width
        height: rogue.height
        z: 2

        GameSelect {
            id: gameContainer
            width: 640
            height: 400

            transform: Scale {
                xScale: Math.min(rogue.width/gameContainer.width, rogue.height/gameContainer.height)
                yScale: Math.min(rogue.width/gameContainer.width, rogue.height/gameContainer.height)
                origin.x: gameContainer.width/2
                origin.y: gameContainer.height/2
            }

            anchors.centerIn: parent

            onRestore: {
                console.log(savefile);
                rogue.restoreGame(savefile);

                activeItem = rogue;
                activeItem.visible = true;
                activeItem.focus = true;
                gameSelectContainer.visible = false;
            }

            function onGameSet()
            {
                if (rogue.showTitleScreen()){
                    activeItem = titleScreen;
                }
                else{
                    activeItem = rogue;
                }
                activeItem.visible = true;
                activeItem.focus = true;
                gameSelectContainer.visible = false;
            }

            onSelected: {
                rogue.game = game;
                onGameSet();
            }

            Component.onCompleted: {
                if (rogue.game)
                    onGameSet();
            }
        }
    }

    RogueTitle {
        id: titleScreen
        visible: false

        z: 1
        width: rogue.width
        height: rogue.height
        anchors.centerIn: parent

        onDismissed: {
            activeItem = rogue;
            activeItem.visible = true;
            activeItem.focus = true;
            titleScreen.visible = false;
        }
    }

    QmlRogue {
        id: rogue
        visible: false

        onRendered: root.rendered()

        property string title: "Rogue Collection v2.0" + (rogue.game ? ' - ' + rogue.game : '')

        z: 0
        width: screenSize.width * fontSize.width
        height: screenSize.height * fontSize.height
        anchors.centerIn: parent

        smooth: false

        Keys.onPressed: {
            if ((event.modifiers & Qt.ControlModifier) && (event.key === Qt.Key_S))
            {
                saveDialog.open();
                event.accepted = true;
            }
        }

        FileDialog {
            id: saveDialog
            title: "Please choose a file to save"
            nameFilters: [ "Saved games (*.sav)", "All files (*)" ]
            selectMultiple: false
            selectExisting: false

            onAccepted: {
                var path = saveDialog.fileUrl.toString();
                path = path.replace(/^(file:\/{3})/,"");
                rogue.saveGame(path);
            }
        }
    }
}
