pragma Singleton
import QtQuick

/*!
    Les images produites de la charte v2 qui sont LIVREES (LOT-87, T2.7).

    Une brique ne nomme jamais un fichier : elle nomme une CLE du cahier des assets
    (`ui/frame/panel-dark`, `ui/button/apply/hover`) et lit ici l'entree qui la porte :
    `Artwork.delivered[cle]`, son fichier (relatif a `baseUrl`) et ses marges 9-patch. Tant que la
    piece n'est pas livree, l'entree est `undefined` et la brique dessine l'aplat de jetons que le
    cahier prevoit pour elle (`fallback`). Le jour ou la piece arrive, la meme brique
    la pose -- sans qu'un seul formulaire change.

    Pourquoi une table et non « essayer le fichier » : une image absente ne se decouvre qu'en la
    chargeant, et un `BorderImage` qui echoue ecrit un avertissement QML a chaque instance. Treize
    briques, dix ecrans : le journal de session serait noye sous des pannes qui n'en sont pas, et
    la vraie -- un fichier livre puis perdu -- s'y cacherait.

    La table `delivered` est ENGENDREE depuis `illustrations.json` (les entrees `produced`) par
    `scripts/receive_ui_assets.py`, et `scripts/check_ui_assets.py` echoue si elle ne le suit
    plus. Ne pas l'editer a la main : c'est le manifeste qui fait foi.

    **Une table et une propriete, pas de fonction.** Qt Design Studio refuse tout appel de fonction
    dans un `.ui.qml` (`qmllint` : QtDesignStudio.FunctionsNotSupportedInQmlUi) : les briques lisent
    la table par indice et concatenent `baseUrl`, ce que l'atelier accepte.

    Les chemins sont resolus relativement a CE fichier (`Theme/`), ce qui donne le meme resultat
    dans l'atelier, depuis les sources et depuis la ressource : `../../Elements/Assets/UI/` y
    designe toujours `Source/Elements/Assets/UI/` -- le mecanisme `BASE` de Source/Ui/CMakeLists.txt.
*/
QtObject {
    id: artwork

    // --- DEBUT DE LA TABLE ENGENDREE (receive_ui_assets.py) -- ne pas editer --------------------
    readonly property var delivered: ({
    })
    // --- FIN DE LA TABLE ENGENDREE --------------------------------------------------------------

    /// Le dossier des images, `Source/Elements/Assets/UI/`, resolu une fois ; se termine par « / ».
    readonly property string baseUrl: Qt.resolvedUrl("../../Elements/Assets/UI/").toString()
}
