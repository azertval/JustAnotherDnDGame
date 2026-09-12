import QtQuick
import Jadg.Ui
import Jadg.Runtime

/*!
    Dialogue -- CABLAGE, cote developpeur (LOT-86).

    Alimente par le lot des dialogues et du runner qui les joue.

    Chaque liaison porte une CLE D'ATTRIBUTION nommee, qui aboutit a l'ancre `PendingData`. Le
    jour ou le lot fonctionnel arrive, il remplace ici `PendingData` par sa vraie vue-modele :
    le formulaire ne bouge pas, et la mise en page decidee aujourd'hui est conservee telle quelle.

    `python scripts/list_pending_bindings.py` releve ces cles depuis le QML : l'inventaire de ce
    qu'il reste a brancher est DERIVE du code, donc toujours exact.
*/
DialogueForm {
    pending: true

    speakerName: PendingData.value("dialogue.speaker.name")
    attitude: PendingData.value("dialogue.speaker.attitude")
    portraitSource: PendingData.image("dialogue.speaker.portrait")
    line: PendingData.value("dialogue.line")
    replies: PendingData.rows("dialogue.replies", 4)
}
