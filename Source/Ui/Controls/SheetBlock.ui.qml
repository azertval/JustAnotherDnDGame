import QtQuick
import QtQuick.Layouts
import Jadg.Ui

/*!
    Un bloc titré d'un écran du RPG (LOT-86).

    C'est l'unité dont tous les écrans sont faits : un titre en accent, un filet, et ce qu'on y
    place. L'écrire une fois est ce qui donne au châssis son unité (EX-IHM-090) — sans elle, le
    huitième écran finit toujours avec un titre d'une graisse différente et une marge de travers.

    Le contenu se déclare simplement à l'intérieur du bloc : il vient se ranger sous le titre.
*/
ColumnLayout {
    id: root

    property string title: "Titre"

    // Le contenu déclaré par l'appelant s'ajoute APRÈS le titre et le filet, dans l'ordre où il
    // est écrit. C'est ce qui permet d'écrire un bloc comme on le lit.
    default property alias content: root.data

    spacing: Tokens.spaceSmall

    Text {
        Layout.fillWidth: true
        text: root.title
        color: Tokens.accent
        font.family: Tokens.bodyFamily
        font.pixelSize: Tokens.body
        // Un bloc sans titre ne laisse pas de trou : certains blocs de la maquette prolongent
        // celui du dessus et n'en portent pas.
        visible: root.title.length > 0
    }

    Rectangle {
        Layout.fillWidth: true
        Layout.preferredHeight: Tokens.frameThickness
        color: Tokens.frameOrnament
        opacity: 0.5
        visible: root.title.length > 0
    }
}
