import QtQuick
import Jadg.Ui

/*!
    Une entree de menu (LOT-86).

    Le focus se signale par un CHEVRON et par la couleur, jamais par la seule couleur : un menu se
    parcourt a la manette, sans pointeur pour dire ou l'on en est, et un signal qui ne tient qu'a
    une teinte disparait pour qui les distingue mal (EX-IHM-040).

    L'entree est aussi sensible au POINTEUR. La manette n'est pas le seul peripherique du jeu, et
    un menu qui ne repond qu'aux touches parait casse a qui l'aborde a la souris -- rien a l'ecran
    ne dit qu'il faudrait lacher le pointeur. Le formulaire n'expose que la ZONE : c'est le jumeau
    qui decide ce que survoler et cliquer veulent dire, un formulaire ne pouvant pas contenir de
    code.
*/
Item {
    id: root

    property string label: "Entrée"
    property bool current: false

    /// La zone sensible au pointeur, a brancher par le jumeau (`entered`, `clicked`).
    property alias pointer: entryPointer

    // La HAUTEUR de l'entree deborde son texte : une cible de la taille exacte des lettres se
    // manque a la souris, et le survol clignoterait entre deux entrees en passant de l'une a
    // l'autre. La largeur, elle, reste celle du libelle -- au-dela, on cliquerait sur du decor.
    implicitHeight: entryLabel.implicitHeight + Tokens.spaceSmall * 2
    implicitWidth: entryChevron.implicitWidth + entryLabel.implicitWidth + Tokens.spaceMedium

    FocusFleuron {
        id: entryChevron
        anchors.verticalCenter: parent.verticalCenter
        size: Tokens.sectionTitle
        opacity: root.current ? 1.0 : 0.0
    }

    Text {
        id: entryLabel
        anchors.left: entryChevron.right
        anchors.leftMargin: Tokens.spaceMedium
        anchors.verticalCenter: parent.verticalCenter
        text: root.label
        // Sur le fond d'encre du menu, l'encre sepia du corps de texte ne se lirait pas : les
        // entrees prennent les teintes CLAIRES de la palette. Deux roles, pas deux couleurs.
        color: root.current ? Tokens.surfaceAlt : Tokens.accent
        font.family: Tokens.bodyFamily
        font.pixelSize: Tokens.sectionTitle
    }

    // Posee EN DERNIER, donc au-dessus : elle recouvre le chevron comme le libelle, et l'entree
    // repond donc au pointeur sur toute sa surface -- y compris a la place que le chevron occupe
    // quand elle n'est pas courante.
    MouseArea {
        id: entryPointer
        anchors.fill: parent
        hoverEnabled: true
        cursorShape: Qt.PointingHandCursor
    }
}
