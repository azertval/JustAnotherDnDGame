import QtQuick
import QtQuick.Layouts
import Jadg.Ui

/*!
    Le châssis commun aux écrans du RPG (LOT-86, EX-IHM-090).

    Il porte ce que les neuf écrans partagent — le sol de parchemin, le double cadre, le titre, le
    pied de navigation — et **rien** de ce qui les distingue. La disposition du contenu appartient
    à chaque écran, donc à la conception : c'est le déplacement que ce lot opère, l'ancien châssis
    imposant au contraire une ossature en deux colonnes décrite en C++.

    Ce que ce châssis garantit reste ce qu'`EX-IHM-090` demandait : le même cadre, le même titre,
    la même ouverture et le même passage d'un écran à l'autre. Ce qu'il cesse d'imposer, c'est
    l'arrangement interne — qui n'avait aucune raison d'être décidé dans du code.
*/
Rectangle {
    id: root

    property string title: "Titre de l'écran"

    /*!
        Vrai tant qu'aucune donnée réelle n'alimente cet écran (LOT-86).

        Le pied l'affiche alors discrètement. Ce n'est pas de la décoration : un écran dessiné mais
        pas branché ressemble à un écran cassé, et sans cette mention quelqu'un finirait par
        chercher la panne. `python scripts/list_pending_bindings.py` en donne l'inventaire.
    */
    property bool pending: false

    /*!
        Ce que le pied annonce des touches. Par défaut, le passage d'un écran du RPG à l'autre.

        Configurable, et il fallait qu'il le soit : les crédits et les options ne se parcourent pas
        aux gâchettes, et leur laisser cette indication aurait annoncé à l'utilisateur des touches
        qui ne font rien. Une aide de touche fausse est pire qu'une aide absente.
    */
    property string navigationHint: qsTr("Précédent") + " ·  · " + qsTr("Suivant")

    property alias content: body.data

    color: Tokens.background

    // L'encadrement relevé sur le corpus : trait d'encre, réserve de parchemin, filet doré, et un
    // cabochon de grenat à chaque angle. Il remplace le double rectangle qui tenait sa place en
    // attendant le portage des ornements.
    ParchmentFrame {
        anchors.fill: parent
        anchors.margins: Tokens.spaceSmall
    }

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: Tokens.spaceExtraLarge
        spacing: Tokens.spaceLarge

        TitleBanner {
            Layout.fillWidth: true
            Layout.preferredHeight: Tokens.sectionTitle + Tokens.spaceLarge
            title: root.title
        }

        // Ce qui ne tient pas DEFILE, au lieu d'etre rogne.
        //
        // La garantie vit sur le chemin COMMUN, jamais dans un ecran. C'est la lecon que le
        // projet a payee trois fois du cote des widgets : le meme debordement y a ete corrige
        // deux fois ecran par ecran avant qu'on ne comprenne qu'une regle a reappliquer se
        // reperd au premier ecran ajoute. Ici, un ecran trop dense defile par construction --
        // personne n'a a y penser.
        Flickable {
            Layout.fillWidth: true
            Layout.fillHeight: true
            clip: true
            contentWidth: width
            contentHeight: body.implicitHeight
            boundsBehavior: Flickable.StopAtBounds

            ColumnLayout {
                id: body
                width: parent.width
                spacing: Tokens.spaceLarge
            }
        }

        // Pied : la navigation, et l'aveu qu'un écran n'est pas encore alimenté.
        RowLayout {
            Layout.fillWidth: true
            spacing: Tokens.spaceLarge

            Text {
                text: root.navigationHint
                color: Tokens.textMuted
                font.family: Tokens.bodyFamily
                font.pixelSize: Tokens.caption
            }
            Item { Layout.fillWidth: true }
            Text {
                text: qsTr("Écran dessiné, données à brancher")
                color: Tokens.textMuted
                font.family: Tokens.bodyFamily
                font.pixelSize: Tokens.caption
                visible: root.pending
            }
        }
    }
}
