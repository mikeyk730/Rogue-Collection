import QtQuick 2.0

Image {
    anchors.centerIn: parent
    source: "assets/title.png"
    smooth: false

    id: root
    signal dismissed()

    Keys.onPressed: {
        if (event.key !== Qt.Key_Alt && event.key !== Qt.Key_Shift && event.key !== Qt.Key_Control){
            event.accepted = true;
            root.dismissed();
        }
    }
}
