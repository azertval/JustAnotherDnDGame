import QtQuick
import QtQuick.Controls
import "."

Column {
    spacing: 8
    IconButton { iconSource: "../../assets/icons/icon_zoom_plus.svg"; onClicked: zoomIn() }
    IconButton { iconSource: "../../assets/icons/icon_zoom_minus.svg"; onClicked: zoomOut() }
    signal zoomIn()
    signal zoomOut()
}
