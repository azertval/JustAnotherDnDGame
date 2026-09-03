// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: GPL-3.0-or-later

#include <gtest/gtest.h>

#include "HMI/Graphics/MechanismVisuals.h"

namespace hmi {
namespace {

// Chaque famille de mecanisme, chaque etat : la correspondance renvoie le clip attendu (LOT-47
// TACHE-01) -- table petite et figee, donc testable exhaustivement.

/**
 * @brief La porte demande le clip « fermée » à l'état inactif et « ouverte » à l'état actif : la
 * table est petite et figée, donc vérifiée famille par famille, exhaustivement.
 * \castest{<b>La porte demande le clip fermé ou ouvert selon son état logique.</b><br/>
 * \tcat Unitaire · Apparence des mécanismes<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et
 * verifier les assertions.<br/>
 * }
 */
TEST(MechanismVisualsTest, DoorTargetClips) {
    EXPECT_EQ(mechanismTargetClip(core::TileType::Door, false), MECHANISM_CLIP_DOOR_CLOSED);
    EXPECT_EQ(mechanismTargetClip(core::TileType::Door, true), MECHANISM_CLIP_DOOR_OPEN);
}

/**
 * @brief L'interrupteur demande le clip inactif ou actif selon son état.
 * \castest{<b>L'interrupteur demande le clip inactif ou actif selon son état.</b><br/>
 * \tcat Unitaire · Apparence des mécanismes<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et
 * verifier les assertions.<br/>
 * }
 */
TEST(MechanismVisualsTest, SwitchTargetClips) {
    EXPECT_EQ(mechanismTargetClip(core::TileType::Switch, false), MECHANISM_CLIP_SWITCH_INACTIVE);
    EXPECT_EQ(mechanismTargetClip(core::TileType::Switch, true), MECHANISM_CLIP_SWITCH_ACTIVE);
}

/**
 * @brief La plaque de pression demande le clip relâché ou enfoncé selon son état.
 * \castest{<b>La plaque de pression demande le clip relâché ou enfoncé selon son état.</b><br/>
 * \tcat Unitaire · Apparence des mécanismes<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et
 * verifier les assertions.<br/>
 * }
 */
TEST(MechanismVisualsTest, PressurePlateTargetClips) {
    EXPECT_EQ(mechanismTargetClip(core::TileType::PressurePlate, false),
              MECHANISM_CLIP_PLATE_RELEASED);
    EXPECT_EQ(mechanismTargetClip(core::TileType::PressurePlate, true),
              MECHANISM_CLIP_PLATE_PRESSED);
}

/**
 * @brief La clé demande le clip « présente » quand elle n'a pas encore été ramassée, « ramassée »
 * une fois consommée — même booléen que la porte verrouillée qui lui est liée (`EX-GP-023`).
 * \castest{<b>La clé demande le clip présente ou ramassée selon son état.</b><br/>
 * \tcat Unitaire · Apparence des mécanismes<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et
 * verifier les assertions.<br/>
 * }
 */
TEST(MechanismVisualsTest, KeyTargetClips) {
    EXPECT_EQ(mechanismTargetClip(core::TileType::Key, false), MECHANISM_CLIP_KEY_PRESENT);
    EXPECT_EQ(mechanismTargetClip(core::TileType::Key, true), MECHANISM_CLIP_KEY_COLLECTED);
}

/**
 * @brief La porte verrouillée demande le clip fermée ou ouverte selon son état, comme la porte
 * classique mais avec ses propres noms de clip (`EX-GP-023`).
 * \castest{<b>La porte verrouillée demande le clip fermée ou ouverte selon son état.</b><br/>
 * \tcat Unitaire · Apparence des mécanismes<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et
 * verifier les assertions.<br/>
 * }
 */
TEST(MechanismVisualsTest, LockedDoorTargetClips) {
    EXPECT_EQ(mechanismTargetClip(core::TileType::LockedDoor, false),
              MECHANISM_CLIP_LOCKED_DOOR_CLOSED);
    EXPECT_EQ(mechanismTargetClip(core::TileType::LockedDoor, true),
              MECHANISM_CLIP_LOCKED_DOOR_OPEN);
}

/**
 * @brief Une tuile sans état ne demande aucun clip et n'est pas reconnue comme mécanisme : c'est ce
 * qui évite d'installer une horloge d'animation sur chaque mur du niveau.
 * \castest{<b>Une tuile sans état ne demande aucun clip et n'est pas un mécanisme.</b><br/>
 * \tcat Unitaire · Apparence des mécanismes<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et
 * verifier les assertions.<br/>
 * }
 */
TEST(MechanismVisualsTest, StatelessTileProducesNoClipRequest) {
    EXPECT_FALSE(mechanismTargetClip(core::TileType::Solid, false).has_value());
    EXPECT_FALSE(mechanismTargetClip(core::TileType::Block, true).has_value());
    EXPECT_FALSE(mechanismTargetClip(core::TileType::Empty, false).has_value());
    EXPECT_FALSE(isStatefulMechanism(core::TileType::Solid));
    EXPECT_FALSE(isStatefulMechanism(core::TileType::Block));
}

/**
 * @brief La porte ne demande un clip de transition que sur un **changement** d'état — ouverture ou
 * fermeture — et rien quand l'état est stable : c'est ce qui empêche de rejouer l'ouverture en
 * boucle sur une porte qui reste ouverte.
 * \castest{<b>La porte ne demande une transition que sur un changement d'état.</b><br/>
 * \tcat Unitaire · Apparence des mécanismes<br/>
 * \tcrit Critique<br/>
 * \tetapes 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et
 * verifier les assertions.<br/>
 * }
 */
TEST(MechanismVisualsTest, DoorTransitionsOnStateChangeOnly) {
    const auto opening = mechanismTransitionClip(core::TileType::Door, false, true);
    ASSERT_TRUE(opening.has_value());
    EXPECT_EQ(*opening, MECHANISM_CLIP_DOOR_OPENING);

    const auto closing = mechanismTransitionClip(core::TileType::Door, true, false);
    ASSERT_TRUE(closing.has_value());
    EXPECT_EQ(*closing, MECHANISM_CLIP_DOOR_CLOSING);

    EXPECT_FALSE(mechanismTransitionClip(core::TileType::Door, true, true).has_value());
    EXPECT_FALSE(mechanismTransitionClip(core::TileType::Door, false, false).has_value());
}

/**
 * @brief En mode Texture, l'opacité de diagnostic vaut toujours 1 : le rendu habillé montre
 * l'apparence de l'asset, jamais une modulation de débogage héritée du mode Physique.
 * \castest{<b>En mode Texture, l'opacité de diagnostic vaut toujours 1.</b><br/>
 * \tcat Unitaire · Apparence des mécanismes<br/>
 * \tcrit Critique<br/>
 * \tetapes 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et
 * verifier les assertions.<br/>
 * }
 */
TEST(MechanismVisualsTest, TextureModeAlwaysFullyOpaque) {
    EXPECT_FLOAT_EQ(mechanismDiagnosticAlpha(RenderMode::Texture, true, 0.25f, 1.0f), 1.0f);
    EXPECT_FLOAT_EQ(mechanismDiagnosticAlpha(RenderMode::Texture, false, 0.25f, 1.0f), 1.0f);
}

/**
 * @brief En mode Physique, la paire d'opacités fournie est utilisée telle quelle, dans un sens
 * comme dans l'autre : une porte ouverte devient plus transparente, un danger actif plus opaque —
 * deux conventions opposées servies par la même fonction, sans cas particulier codé en dur.
 * \castest{<b>En mode Physique, la paire d'opacités fournie est utilisée dans les deux
 * sens.</b><br/>
 * \tcat Unitaire · Apparence des mécanismes<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et
 * verifier les assertions.<br/>
 * }
 */
TEST(MechanismVisualsTest, PhysiqueModeUsesTheProvidedAlphaPair) {
    // Porte : ouverte (active) plus TRANSPARENTE.
    EXPECT_FLOAT_EQ(mechanismDiagnosticAlpha(RenderMode::Physique, true, 0.25f, 1.0f), 0.25f);
    EXPECT_FLOAT_EQ(mechanismDiagnosticAlpha(RenderMode::Physique, false, 0.25f, 1.0f), 1.0f);
    // Danger : actif plus OPAQUE -- sens oppose, meme fonction.
    EXPECT_FLOAT_EQ(mechanismDiagnosticAlpha(RenderMode::Physique, true, 1.0f, 0.35f), 1.0f);
    EXPECT_FLOAT_EQ(mechanismDiagnosticAlpha(RenderMode::Physique, false, 1.0f, 0.35f), 0.35f);
}

}  // namespace
}  // namespace hmi
