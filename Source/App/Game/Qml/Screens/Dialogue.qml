import QtQuick
import Jadg.Ui
import Jadg.Runtime

/*!
    Dialogue -- CABLAGE, cote developpeur (LOT-86, alimente au LOT-15).

    Chaque propriete du formulaire se lit de `DialogueModel`, chaque reponse cliquee devient
    `DialogueModel.choose`. Le modele ne decide rien de la conversation : c'est
    `core::DialogueRunner` qui la joue, et ce qui se voit ici est relu apres chaque geste.

    **Le dialogue ouvert est celui du heraut du Colisee**, dialogue de demonstration provisoire :
    aucune carte ne sait encore ouvrir la conversation d'un PNJ (l'interaction du LOT-10 n'est pas
    cablee dans la session de jeu). Le jour ou elle le sera, c'est ce seul identifiant qui viendra
    du PNJ au lieu d'etre ecrit ici.

    `Echap` quitte la conversation ; `1` a `9` choisissent la reponse de ce rang. La conversation
    terminee, l'ecran se referme de lui-meme.
*/
DialogueForm {
    id: root

    focus: true

    readonly property DialogueModel conversation: DialogueModel {
        dialogueId: "heraut-colisee"
    }

    speakerName: conversation.speakerName
    attitude: conversation.attitude
    line: conversation.line
    checkOutcome: conversation.checkOutcome
    replies: conversation.replies

    onReplyChosen: (rowId) => conversation.choose(rowId)

    Connections {
        target: root.conversation

        function onChanged() {
            if (root.conversation.finished) {
                ScreenRouter.closeRpgScreen();
            }
        }
    }

    Keys.onEscapePressed: ScreenRouter.closeRpgScreen()
    Keys.onPressed: (event) => {
        if (event.key >= Qt.Key_1 && event.key <= Qt.Key_9) {
            root.conversation.chooseAt(event.key - Qt.Key_1);
            event.accepted = true;
        }
    }
}
