import QtQuick 2.4
import QtQuick.Dialogs 1.0

Rectangle {
    id: root
    width: 640
    height: 400
    anchors.centerIn: parent
    color: 'black'

    signal selected(string game)
    signal restore(string savefile)

    property string letters: "abcdefghijklmnopqrstuvwxyz"

    FileDialog {
        id: fileDialog

        title: "Please choose a file to restore"
        nameFilters: [ "Saved games (*.sav)", "All files (*)" ]
        selectMultiple: false
        onAccepted: {
            var path = fileDialog.fileUrl.toString();
            path = path.replace(/^(file:\/{3})/,"");
            root.restore(path)
        }
    }

    Image {
        x: 225
        y: 45
        scale: 1.55

        source: "assets/rogue.png"
        smooth: false
    }

    FontLoader { id: titleFont; source: "assets/Px437_IBM_BIOS.ttf" }

    Rectangle {
        id: header
        x: 15
        y: 218
        height: 22
        width: childrenRect.width
        color: 'black'
        Text {
            text: "Choose your Rogue:"
            font.family: titleFont.name
            font.pixelSize: 16
            color: 'white'
            style: Text.Outline
            styleColor: "transparent"
        }
    }
    ListView {
        id: list

        x: header.x + 17
        y: header.y + header.height
        height: 22 * count

        focus: true

        function handleSelection()
        {
            var item = model[list.currentIndex];
            if(item === "Restore Game"){
                fileDialog.open();
            }
            else
            {
                root.selected(item)
            }
        }

        Keys.onPressed: {
            if (event.modifiers & Qt.AltModifier)
                return;

            if (event.key >= Qt.Key_A && event.key < Qt.Key_A + count) {
                event.accepted = true;
                list.currentIndex = event.key - Qt.Key_A;
                handleSelection();
            }
            else if (event.key === Qt.Key_Return){
                event.accepted = true;
                handleSelection();
            }
        }

        model: gameTitles
        delegate: Item {
            id: wrapper
            height: 22
            Text {
                id: itemText
                text: letters[index] + ") " + modelData
                font.family: titleFont.name
                font.pixelSize: 16
                color: 'white'
                style: Text.Outline
                styleColor: "transparent"
            }
            states: State {
                name: "Current"
                when: wrapper.ListView.isCurrentItem
                PropertyChanges { target: itemText; color: '#ff8800' }
            }
        }
    }
}
