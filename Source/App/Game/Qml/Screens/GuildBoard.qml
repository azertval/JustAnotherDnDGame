import QtQuick
import Jadg.Ui
import Jadg.Runtime

/*!
    Tableau de la Guilde -- CABLAGE, cote developpeur (LOT-86).

    Alimente par le lot de la boucle de progression de la Guilde.

    Chaque liaison porte une CLE D'ATTRIBUTION nommee, qui aboutit a l'ancre `PendingData`. Le
    jour ou le lot fonctionnel arrive, il remplace ici `PendingData` par sa vraie vue-modele :
    le formulaire ne bouge pas, et la mise en page decidee aujourd'hui est conservee telle quelle.

    `python scripts/list_pending_bindings.py` releve ces cles depuis le QML : l'inventaire de ce
    qu'il reste a brancher est DERIVE du code, donc toujours exact.
*/
GuildBoardForm {
    pending: true

    contracts: PendingData.rows("guild_board.contracts", 6)
    contract: PendingData.value("guild_board.contract")
    giver: PendingData.value("guild_board.contract.giver")
    rank: PendingData.value("guild_board.contract.rank")
    reward: PendingData.value("guild_board.contract.reward")
}
