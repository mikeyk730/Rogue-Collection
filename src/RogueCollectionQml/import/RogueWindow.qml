import QtQuick 2.0
import Rogue 1.0

Item {
    property alias font: rogue.font
    property alias screenSize: rogue.screenSize
    property alias fontSize: rogue.fontSize

    id: root

    width: gameSelect.width
    height: gameSelect.height

    GameSelect {
        id: gameSelect

        z: 2
        width: 640
        height: 400
        anchors.centerIn: parent

        onSelected: {
            console.log(game, savefile);
            if (game){
                rogue.game = game;
            }

            if (game === "PC Rogue 1.48"){
                root.width = titleScreen.width
                root.height = titleScreen.height
                titleScreen.visible = true;
                titleScreen.focus = true;
            }
            else{
                root.width = rogue.width
                root.height = rogue.height
                rogue.visible = true;
                rogue.focus = true;
            }
            gameSelect.visible = false;
        }
    }

    RogueTitle {
        id: titleScreen
        visible: false

        z: 1
        anchors.centerIn: parent

        onDismissed: {
            root.width = rogue.width
            root.height = rogue.height
            rogue.visible = true;
            rogue.focus = true;
            titleScreen.visible = false;
        }
    }

    QmlRogue {
        id: rogue
        visible: false

        z: 0
        width: screenSize.width * fontSize.width
        height: screenSize.height * fontSize.height
        anchors.centerIn: parent

        //font.family: 'Consolas'
        font.pixelSize: 16
        smooth: false
    }
}
