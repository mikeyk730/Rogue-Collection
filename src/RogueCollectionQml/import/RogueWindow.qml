import QtQuick 2.0
import Rogue 1.0

Item {
    width: rogue.width
    height: rogue.height

    GameSelect {
        id: gameSelect

        z: 2
        anchors.fill: parent

        onSelected: {
            console.log(game, savefile);
            if (game){
                rogue.game = game;
            }

            if (game === "PC Rogue 1.48"){
                titleScreen.visible = true;
                titleScreen.focus = true;
            }
            else{
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
        anchors.fill: parent

        onDismissed: {
            rogue.visible = true;
            rogue.focus = true;
            titleScreen.visible = false;
        }
    }

    QmlRogue {
        id: rogue

        visible: false
        z: 0
        anchors.centerIn: parent

        width: screenSize.width * fontSize.width
        height: screenSize.height * fontSize.height

        //font.family: 'Consolas'
        font.pixelSize: 16
        smooth: false
    }
}
