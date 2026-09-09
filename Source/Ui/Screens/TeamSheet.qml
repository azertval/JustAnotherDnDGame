import QtQuick
import Jadg.Ui

/*!
    Feuille d'equipe -- CABLAGE, cote developpeur (LOT-86).

    Alimente par le lot de la Guilde et celui du groupe.

    Chaque liaison porte une CLE D'ATTRIBUTION nommee, qui aboutit a l'ancre `PendingData`. Le
    jour ou le lot fonctionnel arrive, il remplace ici `PendingData` par sa vraie vue-modele :
    le formulaire ne bouge pas, et la mise en page decidee aujourd'hui est conservee telle quelle.

    `python scripts/list_pending_bindings.py` releve ces cles depuis le QML : l'inventaire de ce
    qu'il reste a brancher est DERIVE du code, donc toujours exact.
*/
TeamSheetForm {
    pending: true

    teamName: PendingData.value("team_sheet.name")
    careerPoints: PendingData.value("team_sheet.career_points")
    fame: PendingData.value("team_sheet.fame")
    prestige: PendingData.value("team_sheet.prestige")
    beneficiary: PendingData.value("team_sheet.beneficiary")
    style: PendingData.value("team_sheet.style")
    specialization: PendingData.value("team_sheet.specialization")
    members: PendingData.rows("team_sheet.members", 4)
    relations: PendingData.value("team_sheet.relations")
    coatOfArmsSource: PendingData.image("team_sheet.coat_of_arms")
    dream: PendingData.value("team_sheet.dream")
    hiddenAgenda: PendingData.value("team_sheet.hidden_agenda")
    legendaryRewards: PendingData.value("team_sheet.legendary_rewards")
    headquarters: PendingData.rows("team_sheet.headquarters", 8)
}
