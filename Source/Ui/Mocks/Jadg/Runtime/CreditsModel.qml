import QtQuick

/*!
    Doublure de `hmi::CreditsModel` pour Qt Design Studio (LOT-87, T3.3).

    Mêmes propriétés que le type C++ (`Source/HMI/Runtime/CreditsModel.h`) : deux sections
    d'exemple par colonne, aux mêmes champs (`sectionId`, `title`, `iconKey`, `lines` de `role` et
    `names`). Les noms sont des exemples de conception, jamais ceux du jeu.
*/
QtObject {
    property int column: 0
    property string language: "fr"
    readonly property var sections: [
        { sectionId: "development", title: "Section", iconKey: "ui/icon/credits-section/development",
          lines: [ { role: "Rôle", names: "Nom\nNom" }, { role: "Rôle", names: "Nom" } ] },
        { sectionId: "other", title: "Section", iconKey: "ui/icon/credits-section/other",
          lines: [ { role: "Rôle", names: "Nom" } ] }
    ]
}