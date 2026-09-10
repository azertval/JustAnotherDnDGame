// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: GPL-3.0-or-later

#include "HMI/Presentation/InventoryModel.h"

#include <QVector>
#include <array>
#include <string>
#include <utility>

#include "Core/Rpg/Inventory.h"
#include "HMI/HmiLog.h"
#include "HMI/Platform/ExecutableDirectory.h"
#include "HMI/Presentation/DemonstrationCharacter.h"
#include "HMI/Presentation/RuleLabels.h"

namespace hmi {
namespace {

constexpr const char* EMPTY_MARK = "—";

/// Les seize emplacements, dans l'ordre où une feuille de personnage les grave : de la tête aux
/// pieds, puis ce qui se porte à part.
constexpr std::array<core::EquipmentSlot, core::EQUIPMENT_SLOT_COUNT> SLOTS{
    core::EquipmentSlot::Head,       core::EquipmentSlot::Neck,     core::EquipmentSlot::Cloak,
    core::EquipmentSlot::Torso,      core::EquipmentSlot::Belt,     core::EquipmentSlot::Hands,
    core::EquipmentSlot::Bracers,    core::EquipmentSlot::RingLeft, core::EquipmentSlot::RingRight,
    core::EquipmentSlot::MainHand,   core::EquipmentSlot::OffHand,  core::EquipmentSlot::Ranged,
    core::EquipmentSlot::Ammunition, core::EquipmentSlot::Feet,     core::EquipmentSlot::Pouch,
    core::EquipmentSlot::Trinket,
};

[[nodiscard]] QString toQt(const std::string& text) {
    return QString::fromStdString(text);
}

/**
 * @brief Traduit un nom d'emplacement du modèle en clé de traduction.
 *
 * `core::equipmentSlotName` rend un nom en tirets (`ring-left`) — c'est ce qui sert d'identifiant
 * de valeur. Le catalogue de traduction, lui, indexe en soulignés (`rpg.slot.ring_left`). La
 * conversion vit ici, à la frontière, plutôt que d'imposer une des deux conventions à l'autre
 * côté : le modèle a la sienne, le catalogue la sienne, et aucun des deux n'a tort.
 */
[[nodiscard]] std::string translationKey(std::string_view slotName) {
    std::string key(slotName);
    for (char& letter : key) {
        if (letter == '-') {
            letter = '_';
        }
    }
    return "rpg.slot." + key;
}

}  // namespace

InventoryModel::InventoryModel(QObject* parent) : QObject(parent), _slots(this) {}

QString InventoryModel::value(const char* key) const {
    const auto found = _values.find(key);
    if (found == _values.end() || found->second.empty()) {
        return QString::fromUtf8(EMPTY_MARK);
    }
    return toQt(found->second);
}

void InventoryModel::loadDemonstrationCharacter() {
    const DemonstrationCharacter loaded = loadDemonstrationValues();
    _values = loaded.inventory;

    const std::string language = activeLanguage();

    QVector<SheetRow> rows;
    rows.reserve(static_cast<qsizetype>(SLOTS.size()));
    for (const core::EquipmentSlot slot : SLOTS) {
        const std::string name(core::equipmentSlotName(slot));
        const auto found = _values.find("inventory.slot." + name);
        rows.append(SheetRow{
            .id = toQt(name),
            .label = toQt(ruleLabel(translationKey(name), language)),
            .value = found == _values.end() ? QString::fromUtf8(EMPTY_MARK) : toQt(found->second)});
    }
    _slots.setRows(std::move(rows));

    emit changed();
}

}  // namespace hmi
