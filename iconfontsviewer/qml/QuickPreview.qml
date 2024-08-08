import IconFonts
import QtQuick.Controls

FontIcon {
    Label {
        anchors.centerIn: parent
        text: qsTr("No symbol selected.")
        visible: parent.icon.symbol.isNull
    }
}
