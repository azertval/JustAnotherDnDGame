// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <map>
#include <string>

class QWidget;

/**
 * @file HMI/Interface/RpgScreenSurface.h
 * @brief Ce qu'un écran du RPG doit savoir faire, quel que soit son rendu (`LOT-38`).
 *
 * Deux rendus coexistent depuis que la fiche a sa planche : l'ossature générique
 * (`hmi::RpgScreenFrame`) et la planche décrite en Qt Designer
 * (`hmi::RpgCharacterSheetPlate`). `hmi::RpgScreenHost` n'a pas à savoir lequel il tient — il les
 * héberge, les traduit et les remplit de la même façon.
 *
 * Sans cette interface, l'hôte brancherait sur le genre de rendu en **quatre** endroits — la
 * traduction, les valeurs, le focus, l'ajout à la pile — et le cinquième serait oublié le jour où
 * un second écran recevrait sa planche. Le branchement se fait donc **une** fois, à la
 * construction.
 *
 * Ce n'est délibérément pas un `QObject` : hériter d'une interface pure à côté de `QWidget` évite
 * la double héritance de `QObject`, que Qt n'admet pas. `widget()` rend le `QWidget` que la pile
 * doit héberger.
 */

namespace hmi {

class Localization;

class RpgScreenSurface {
public:
    RpgScreenSurface() = default;
    virtual ~RpgScreenSurface() = default;

    RpgScreenSurface(const RpgScreenSurface&) = delete;
    RpgScreenSurface& operator=(const RpgScreenSurface&) = delete;
    RpgScreenSurface(RpgScreenSurface&&) = delete;
    RpgScreenSurface& operator=(RpgScreenSurface&&) = delete;

    /// Applique la langue active à tous les libellés de l'écran.
    virtual void retranslateUi(const Localization& loc) = 0;

    /// Pose les valeurs, par identifiant. Ce qu'aucune entrée ne nomme **reste au tiret cadratin**
    /// — c'est ce qui distingue « ce champ n'a pas de source » de « ce champ vaut zéro ».
    virtual void setValues(const std::map<std::string, std::string>& values) = 0;

    /// Donne le focus clavier à la porte d'entrée du parcours (`EX-IHM-071`).
    virtual void focusDefaultAction() = 0;

    /// @return Le widget à héberger dans la pile.
    [[nodiscard]] virtual QWidget* widget() = 0;
};

}  // namespace hmi
