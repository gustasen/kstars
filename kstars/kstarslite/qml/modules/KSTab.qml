import QtQuick 2.6
import QtQuick.Layouts 1.2
import QtQuick.Controls 2.0
//import QtQuick.Controls 1.4
import "../constants" 1.0

Pane {
    id: tab
    property string title: ""
    clip: true
    property Item flickableItem: flickable
    padding: 0

    //contentItem is already used by Pane so be it rootItem
    property Item rootItem

    onRootItemChanged: {
        if(rootItem.parent != flickable.contentItem) rootItem.parent = flickable.contentItem
    }

    Flickable {
        id: flickable
        anchors.fill: parent
        ScrollBar.vertical: ScrollBar { id: scrollBar }
        flickableDirection: Flickable.VerticalFlick
        contentWidth: rootItem != undefined ? rootItem.width : 0
        contentHeight: rootItem != undefined ? rootItem.height : 0
        flickableChildren: rootItem
    }
}
