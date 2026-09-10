import QtQuick
import Jadg.Ui

/*!
    Carte du monde -- CABLAGE, cote developpeur (LOT-86).

    Les regions existent (atlas du LOT-37) ; leur presentation a l'ecran, non.

    Chaque liaison porte une CLE D'ATTRIBUTION nommee, qui aboutit a l'ancre `PendingData`. Le
    jour ou le lot fonctionnel arrive, il remplace ici `PendingData` par sa vraie vue-modele :
    le formulaire ne bouge pas, et la mise en page decidee aujourd'hui est conservee telle quelle.

    `python scripts/list_pending_bindings.py` releve ces cles depuis le QML : l'inventaire de ce
    qu'il reste a brancher est DERIVE du code, donc toujours exact.
*/
WorldMapForm {
    pending: true

    regions: PendingData.rows("world_map.regions", 6)
    region: PendingData.value("world_map.place.region")
    placeType: PendingData.value("world_map.place.type")
    danger: PendingData.value("world_map.place.danger")
    mapSource: PendingData.image("world_map.image")
}
