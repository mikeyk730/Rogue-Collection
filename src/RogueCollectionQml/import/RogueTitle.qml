import QtQuick 2.0

Image {
    anchors.centerIn: parent
    source: "assets/title.png"
    smooth: false

    signal dismissed()

    Keys.onPressed: {
        if (event.key !== Qt.Key_Alt && event.key !== Qt.Key_Shift && event.key !== Qt.Key_Control &&
                (event.modifiers & Qt.AltModifier) === 0){
            event.accepted = true;
            dismissed();
        }
    }
}
