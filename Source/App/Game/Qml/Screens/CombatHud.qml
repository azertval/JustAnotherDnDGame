import QtQuick
import Jadg.Ui
import Jadg.Runtime

/*!
    ATH de combat -- CABLAGE, cote developpeur (LOT-86).

    Alimente par le lot du combat tactique : initiative, cible, actions du tour.

    Chaque liaison porte une CLE D'ATTRIBUTION nommee, qui aboutit a l'ancre `PendingData`. Le
    jour ou le lot fonctionnel arrive, il remplace ici `PendingData` par sa vraie vue-modele :
    le formulaire ne bouge pas, et la mise en page decidee aujourd'hui est conservee telle quelle.

    `python scripts/list_pending_bindings.py` releve ces cles depuis le QML : l'inventaire de ce
    qu'il reste a brancher est DERIVE du code, donc toujours exact.
*/
CombatHudForm {
    pending: true

    initiative: PendingData.rows("combat_hud.initiative", 6)
    targetName: PendingData.value("combat_hud.target.name")
    targetHitPoints: PendingData.value("combat_hud.target.hit_points")
    targetArmorClass: PendingData.value("combat_hud.target.armor_class")
    targetConditions: PendingData.value("combat_hud.target.conditions")
    actions: PendingData.rows("combat_hud.actions", 6)
}
