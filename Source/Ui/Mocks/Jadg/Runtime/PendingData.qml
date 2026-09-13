pragma Singleton
import QtQuick

/*!
    Doublure de `hmi::PendingData` pour Qt Design Studio (LOT-87).

    Même contrat que le type C++ (`Source/HMI/Runtime/PendingData.h`) : `value()` rend le tiret
    cadratin des écrans dessinés mais pas encore alimentés, `image()` une source vide, `rows()` un
    modèle de `count` lignes aux rôles `rowId`, `label`, `value` -- ceux de `SheetRowModel`.
*/
QtObject {
    id: root

    function value(key) {
        return "—";
    }

    function image(key) {
        return "";
    }

    function rows(key, count) {
        const model = Qt.createQmlObject("import QtQuick; ListModel {}", root, "PendingData.rows");
        for (let rank = 0; rank < count; ++rank) {
            model.append({ rowId: key, label: "—", value: "" });
        }
        return model;
    }
}
