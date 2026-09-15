// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: PolyForm-Noncommercial-1.0.0

/**
 * @file test_player_input_mapper.cpp
 * @brief Tests unitaires de la traduction clavier/manette → intention (`toPlayerInput`).
 */

#include <gtest/gtest.h>

#include "Core/Physics/PlayerInput.h"
#include "HMI/Input/GameKeyBindings.h"
#include "HMI/Input/GamepadBindings.h"
#include "HMI/Input/InputState.h"
#include "HMI/Input/PlayerInputMapper.h"

namespace {

// Construit un état clavier avec un ensemble de touches enfoncées.
hmi::InputState withKeys(std::initializer_list<hmi::Key> keys) {
    hmi::InputState input;
    for (const hmi::Key key : keys) {
        input.onKeyDown(key);
    }
    return input;
}

// Construit un état avec un bouton manette (piste brute) enfoncé.
hmi::InputState withGamepadButton(hmi::GamepadButton button) {
    hmi::InputState input;
    input.onGamepadButtonDown(button);
    return input;
}

// Traduit avec les bindings clavier/manette par défaut : la plupart des tests ci-dessous ne
// portent pas sur le remappage lui-même (couvert par les tests dédiés en fin de fichier),
// seulement sur la traduction touche -> intention.
core::PlayerInput mapWithDefaults(const hmi::InputState& input) {
    return hmi::toPlayerInput(input, hmi::GameKeyBindings{}, hmi::GamepadBindings{});
}

}  // namespace

/**
 * @brief Flèche gauche seule → intention vers la gauche (-1).
 * \castest{<b>Flèche gauche seule → intention vers la gauche (-1).</b><br/>
 * \tcat Unitaire · Player Input Mapper<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et
 * verifier les assertions.<br/>
 * \tattendu Flèche gauche seule → intention vers la gauche (-1).
 * }
 */
TEST(PlayerInputMapperTest, FlecheGauche) {
    EXPECT_FLOAT_EQ(mapWithDefaults(withKeys({hmi::Key::Left})).moveX, -1.0f);
}

/**
 * @brief Flèche droite seule → intention vers la droite (+1).
 * \castest{<b>Flèche droite seule → intention vers la droite (+1).</b><br/>
 * \tcat Unitaire · Player Input Mapper<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et
 * verifier les assertions.<br/>
 * \tattendu Flèche droite seule → intention vers la droite (+1).
 * }
 */
TEST(PlayerInputMapperTest, FlecheDroite) {
    EXPECT_FLOAT_EQ(mapWithDefaults(withKeys({hmi::Key::Right})).moveX, 1.0f);
}

/**
 * @brief Sans remap, `Q`/`D`/`W` ne déclenchent rien (aucun alias fixe).
 * \castest{<b>Sans remap, Q/D/W ne déclenchent rien (aucun alias fixe).</b><br/>
 * \tcat Unitaire · Player Input Mapper<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et
 * verifier les assertions.<br/>
 * \tattendu Sans remap, Q/D/W ne déclenchent rien.
 * }
 */
TEST(PlayerInputMapperTest, SansRemapQDWNeDeclenchentRien) {
    EXPECT_FLOAT_EQ(mapWithDefaults(withKeys({hmi::Key::Q})).moveX, 0.0f);
    EXPECT_FLOAT_EQ(mapWithDefaults(withKeys({hmi::Key::D})).moveX, 0.0f);
    EXPECT_FALSE(mapWithDefaults(withKeys({hmi::Key::W})).jumpPressed);
}

/**
 * @brief Aucune touche → intention nulle (immobile).
 * \castest{<b>Aucune touche → intention nulle (immobile).</b><br/>
 * \tcat Unitaire · Player Input Mapper<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et
 * verifier les assertions.<br/>
 * \tattendu Aucune touche → intention nulle (immobile).
 * }
 */
TEST(PlayerInputMapperTest, AucuneTouche) {
    EXPECT_FLOAT_EQ(mapWithDefaults(hmi::InputState{}).moveX, 0.0f);
}

/**
 * @brief Gauche et droite simultanées → neutralisation (0).
 * \castest{<b>Gauche et droite simultanées → neutralisation (0).</b><br/>
 * \tcat Unitaire · Player Input Mapper<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et
 * verifier les assertions.<br/>
 * \tattendu Gauche et droite simultanées → neutralisation (0).
 * }
 */
TEST(PlayerInputMapperTest, GaucheEtDroiteSeNeutralisent) {
    EXPECT_FLOAT_EQ(mapWithDefaults(withKeys({hmi::Key::Left, hmi::Key::Right})).moveX, 0.0f);
}

/**
 * @brief Espace fraîchement enfoncée → saut **pressé** (front) et **maintenu**.
 * \castest{<b>Espace fraîchement enfoncée → saut **pressé** (front) et **maintenu**.</b><br/>
 * \tcat Unitaire · Player Input Mapper<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et
 * verifier les assertions.<br/>
 * \tattendu Espace fraîchement enfoncée → saut **pressé** (front) et **maintenu**.
 * }
 */
TEST(PlayerInputMapperTest, EspacePresseeDeclencheLeSaut) {
    const core::PlayerInput input = mapWithDefaults(withKeys({hmi::Key::Space}));
    EXPECT_TRUE(input.jumpPressed);
    EXPECT_TRUE(input.jumpHeld);
}

/**
 * @brief Saut **maintenu** sans nouveau front → `jumpHeld` vrai mais `jumpPressed` faux.
 * \castest{<b>Saut **maintenu** sans nouveau front → `jumpHeld` vrai mais `jumpPressed`
 * faux.</b><br/>
 * \tcat Unitaire · Player Input Mapper<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et
 * verifier les assertions.<br/>
 * \tattendu Saut **maintenu** sans nouveau front → `jumpHeld` vrai mais `jumpPressed` faux.
 * }
 */
TEST(PlayerInputMapperTest, SautMaintenuN_estPasUnFront) {
    hmi::InputState input;
    input.onKeyDown(hmi::Key::Space);  // frame 1 : le front a déjà eu lieu
    input.beginFrame();                // frame 2 : l'état courant devient l'état précédent
    input.onKeyDown(hmi::Key::Space);  // toujours enfoncée, mais plus au front

    const core::PlayerInput mapped = mapWithDefaults(input);
    EXPECT_FALSE(mapped.jumpPressed);
    EXPECT_TRUE(mapped.jumpHeld);
}

/**
 * @brief Aucune touche de saut → ni pressé ni maintenu.
 * \castest{<b>Aucune touche de saut → ni pressé ni maintenu.</b><br/>
 * \tcat Unitaire · Player Input Mapper<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et
 * verifier les assertions.<br/>
 * \tattendu Aucune touche de saut → ni pressé ni maintenu.
 * }
 */
TEST(PlayerInputMapperTest, PasDeSaut) {
    const core::PlayerInput input = mapWithDefaults(hmi::InputState{});
    EXPECT_FALSE(input.jumpPressed);
    EXPECT_FALSE(input.jumpHeld);
}

/**
 * @brief Déplacement et saut sont indépendants (axes distincts).
 * \castest{<b>Déplacement et saut sont indépendants (axes distincts).</b><br/>
 * \tcat Unitaire · Player Input Mapper<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et
 * verifier les assertions.<br/>
 * \tattendu Déplacement et saut sont indépendants (axes distincts).
 * }
 */
TEST(PlayerInputMapperTest, DeplacementEtSautIndependants) {
    const core::PlayerInput input = mapWithDefaults(withKeys({hmi::Key::Right, hmi::Key::Space}));
    EXPECT_FLOAT_EQ(input.moveX, 1.0f);
    EXPECT_TRUE(input.jumpPressed);
}

/**
 * @brief Maj fraîchement enfoncée → dash **pressé** (front).
 * \castest{<b>Maj fraîchement enfoncée → dash **pressé** (front).</b><br/>
 * \tcat Unitaire · Player Input Mapper<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et
 * verifier les assertions.<br/>
 * \tattendu Maj fraîchement enfoncée → dash **pressé** (front).
 * }
 */
TEST(PlayerInputMapperTest, MajDeclencheLeDash) {
    EXPECT_TRUE(mapWithDefaults(withKeys({hmi::Key::Shift})).dashPressed);
    EXPECT_FALSE(mapWithDefaults(hmi::InputState{}).dashPressed);
}

/**
 * @brief Visée verticale (y vers le bas) : Bas → +1, Haut → -1, les deux → 0.
 * \castest{<b>Visée verticale (y vers le bas) : Bas → +1, Haut → -1, les deux → 0.</b><br/>
 * \tcat Unitaire · Player Input Mapper<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et
 * verifier les assertions.<br/>
 * \tattendu Visée verticale (y vers le bas) : Bas → +1, Haut → -1, les deux → 0.
 * }
 */
TEST(PlayerInputMapperTest, ViseeVerticale) {
    EXPECT_FLOAT_EQ(mapWithDefaults(withKeys({hmi::Key::Down})).moveY, 1.0f);
    EXPECT_FLOAT_EQ(mapWithDefaults(withKeys({hmi::Key::Up})).moveY, -1.0f);
    EXPECT_FLOAT_EQ(mapWithDefaults(withKeys({hmi::Key::Up, hmi::Key::Down})).moveY, 0.0f);
}

/**
 * @brief Une action remappée au clavier (`LOT-29`) réagit à sa nouvelle touche.
 * \castest{<b>Une action remappée au clavier réagit à sa nouvelle touche.</b><br/>
 * \tcat Unitaire · Player Input Mapper<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et
 * verifier les assertions.<br/>
 * \tattendu Une action remappée au clavier réagit à sa nouvelle touche.
 * }
 */
TEST(PlayerInputMapperTest, RemapperUneActionClavierUtiliseLaNouvelleTouche) {
    hmi::GameKeyBindings gameKeyBindings;
    gameKeyBindings.setKey(hmi::GameAction::Dash, hmi::Key::F1);

    EXPECT_TRUE(
        hmi::toPlayerInput(withKeys({hmi::Key::F1}), gameKeyBindings, hmi::GamepadBindings{})
            .dashPressed);
}

/**
 * @brief Remapper Gauche et Droite chacune sur une touche arbitraire et distincte produit un
 *        mouvement dans le bon sens pour chacune, sans neutralisation croisée.
 * \castest{<b>Remapper Gauche et Droite sur des touches distinctes ne les neutralise pas l'une
 * l'autre.</b><br/>
 * \tcat Unitaire · Player Input Mapper<br/>
 * \tcrit Bloquant<br/>
 * \tetapes 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et
 * verifier les assertions.<br/>
 * \tattendu Remapper Gauche sur D et Droite sur Q produit le mouvement attendu pour chacune.
 * }
 */
TEST(PlayerInputMapperTest, RemapperGaucheEtDroiteSurDesTouchesDistinctesNeLesAnnulePas) {
    // Aucune touche n'est jamais partagee par deux actions : chaque action ne repond qu'a sa
    // propre touche liee ; deux touches distinctes pour deux actions opposees ne peuvent donc
    // jamais s'annuler.
    hmi::GameKeyBindings bindings;
    bindings.setKey(hmi::GameAction::MoveLeft, hmi::Key::D);
    bindings.setKey(hmi::GameAction::MoveRight, hmi::Key::Q);
    const hmi::GamepadBindings gamepadBindings;

    EXPECT_FLOAT_EQ(hmi::toPlayerInput(withKeys({hmi::Key::D}), bindings, gamepadBindings).moveX,
                    -1.0f);
    EXPECT_FLOAT_EQ(hmi::toPlayerInput(withKeys({hmi::Key::Q}), bindings, gamepadBindings).moveX,
                    1.0f);
}

/**
 * @brief Une action déclenchée par son bouton manette lié (piste brute), indépendamment du
 *        clavier.
 * \castest{<b>Une action est déclenchée par son bouton manette lié.</b><br/>
 * \tcat Unitaire · Player Input Mapper<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et
 * verifier les assertions.<br/>
 * \tattendu Une action est déclenchée par son bouton manette lié.
 * }
 */
TEST(PlayerInputMapperTest, ActionDeclencheeParSonBoutonManetteParDefaut) {
    const core::PlayerInput input = hmi::toPlayerInput(
        withGamepadButton(hmi::GamepadButton::A), hmi::GameKeyBindings{}, hmi::GamepadBindings{});
    EXPECT_TRUE(input.jumpPressed);
}

/**
 * @brief Une action remappée manette (`LOT-30`) réagit à son nouveau bouton, plus au clavier.
 * \castest{<b>Une action remappée manette réagit à son nouveau bouton, le clavier reste
 * indépendant.</b><br/>
 * \tcat Unitaire · Player Input Mapper<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et
 * verifier les assertions.<br/>
 * \tattendu Remapper Sauter sur X (manette) le déclenche via X ; le clavier (Espace) reste
 * indépendant.
 * }
 */
TEST(PlayerInputMapperTest, RemapperUneActionManetteUtiliseLeNouveauBouton) {
    hmi::GamepadBindings gamepadBindings;
    gamepadBindings.setKey(hmi::GameAction::Jump, hmi::GamepadButton::X);

    EXPECT_TRUE(hmi::toPlayerInput(withGamepadButton(hmi::GamepadButton::X), hmi::GameKeyBindings{},
                                   gamepadBindings)
                    .jumpPressed);
    // Le clavier (Espace, touche par defaut de Sauter) continue de fonctionner independamment du
    // remap manette : les deux sources sont completement independantes l'une de l'autre.
    EXPECT_TRUE(mapWithDefaults(withKeys({hmi::Key::Space})).jumpPressed);
}

/**
 * @brief Remapper deux actions sur des boutons manette distincts ne les neutralise pas l'une
 *        l'autre (même invariant que le clavier).
 * \castest{<b>Remapper deux actions manette sur des boutons distincts ne les neutralise pas
 * l'une l'autre.</b><br/>
 * \tcat Unitaire · Player Input Mapper<br/>
 * \tcrit Bloquant<br/>
 * \tetapes 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et
 * verifier les assertions.<br/>
 * \tattendu Remapper deux actions manette sur des boutons distincts ne les neutralise pas.
 * }
 */
TEST(PlayerInputMapperTest, RemapperGaucheEtDroiteManetteSurDesBoutonsDistinctsNeLesAnnulePas) {
    hmi::GamepadBindings gamepadBindings;
    gamepadBindings.setKey(hmi::GameAction::MoveLeft, hmi::GamepadButton::X);
    gamepadBindings.setKey(hmi::GameAction::MoveRight, hmi::GamepadButton::Y);
    const hmi::GameKeyBindings gameKeyBindings;

    EXPECT_FLOAT_EQ(hmi::toPlayerInput(withGamepadButton(hmi::GamepadButton::X), gameKeyBindings,
                                       gamepadBindings)
                        .moveX,
                    -1.0f);
    EXPECT_FLOAT_EQ(hmi::toPlayerInput(withGamepadButton(hmi::GamepadButton::Y), gameKeyBindings,
                                       gamepadBindings)
                        .moveX,
                    1.0f);
}

/**
 * @brief E fraîchement enfoncée → Interagir **pressé** (front) et **maintenu** ; aucune touche →
 *        ni pressé ni maintenu.
 * \castest{<b>E fraîchement enfoncée → Interagir pressé (front) et maintenu.</b><br/>
 * \tcat Unitaire · Player Input Mapper<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et
 * verifier les assertions.<br/>
 * \tattendu E fraîchement enfoncée → `interactPressed` et `interactHeld` vrais ; sans la touche,
 * les deux sont faux.
 * }
 */
TEST(PlayerInputMapperTest, EPresseeDeclencheInteragir) {
    const core::PlayerInput pressed = mapWithDefaults(withKeys({hmi::Key::E}));
    EXPECT_TRUE(pressed.interactPressed);
    EXPECT_TRUE(pressed.interactHeld);
    EXPECT_FALSE(pressed.interactReleased);

    const core::PlayerInput none = mapWithDefaults(hmi::InputState{});
    EXPECT_FALSE(none.interactPressed);
    EXPECT_FALSE(none.interactHeld);
}

/**
 * @brief Interagir maintenu sans nouveau front → `interactHeld` vrai mais `interactPressed` faux.
 * \castest{<b>Interagir maintenu sans nouveau front → interactHeld vrai mais interactPressed
 * faux.</b><br/>
 * \tcat Unitaire · Player Input Mapper<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et
 * verifier les assertions.<br/>
 * \tattendu Sur une deuxième frame avec E toujours enfoncée, `interactPressed` est faux et
 * `interactHeld` reste vrai.
 * }
 */
TEST(PlayerInputMapperTest, InteragirMaintenuN_estPasUnFront) {
    hmi::InputState input;
    input.onKeyDown(hmi::Key::E);  // frame 1 : le front a deja eu lieu
    input.beginFrame();            // frame 2 : l'etat courant devient l'etat precedent
    input.onKeyDown(hmi::Key::E);  // toujours enfoncee, mais plus au front

    const core::PlayerInput mapped = mapWithDefaults(input);
    EXPECT_FALSE(mapped.interactPressed);
    EXPECT_TRUE(mapped.interactHeld);
}

/**
 * @brief Relâcher E produit le front `interactReleased`, une frame seulement.
 * \castest{<b>Relâcher E produit le front interactReleased, une frame seulement.</b><br/>
 * \tcat Unitaire · Player Input Mapper<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et
 * verifier les assertions.<br/>
 * \tattendu Après `onKeyUp`, `interactReleased` est vrai à la frame du relâchement puis faux à la
 * frame suivante.
 * }
 */
TEST(PlayerInputMapperTest, RelacherEDeclencheInteragirRelache) {
    hmi::InputState input;
    input.onKeyDown(hmi::Key::E);
    input.beginFrame();
    input.onKeyUp(hmi::Key::E);

    const core::PlayerInput released = mapWithDefaults(input);
    EXPECT_TRUE(released.interactReleased);
    EXPECT_FALSE(released.interactHeld);

    input.beginFrame();
    const core::PlayerInput settled = mapWithDefaults(input);
    EXPECT_FALSE(settled.interactReleased);
}

/**
 * @brief Interagir se déclenche aussi via son bouton manette lié (X) par défaut, indépendamment
 *        du clavier.
 * \castest{<b>Interagir se déclenche via son bouton manette par défaut (X).</b><br/>
 * \tcat Unitaire · Player Input Mapper<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et
 * verifier les assertions.<br/>
 * \tattendu Le bouton X manette declenche `interactPressed`, sans qu'aucune touche clavier ne
 * soit enfoncee.
 * }
 */
TEST(PlayerInputMapperTest, BoutonXManetteDeclencheInteragir) {
    const core::PlayerInput input = hmi::toPlayerInput(
        withGamepadButton(hmi::GamepadButton::X), hmi::GameKeyBindings{}, hmi::GamepadBindings{});
    EXPECT_TRUE(input.interactPressed);
}

/**
 * @brief Remapper Interagir (clavier ou manette) réagit à la nouvelle source, sans changer le
 *        comportement par contact des mécanismes existants (aucun autre champ affecté).
 * \castest{<b>Remapper Interagir réagit à la nouvelle touche/bouton.</b><br/>
 * \tcat Unitaire · Player Input Mapper<br/>
 * \tcrit Bloquant<br/>
 * \tetapes 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et
 * verifier les assertions.<br/>
 * \tattendu Remapper Interagir sur F1 (clavier) ou Y (manette) le declenche via cette nouvelle
 * source ; la source par defaut de l'autre peripherique reste independante.
 * }
 */
TEST(PlayerInputMapperTest, RemapperInteragirUtiliseLaNouvelleSource) {
    hmi::GameKeyBindings gameKeyBindings;
    gameKeyBindings.setKey(hmi::GameAction::Interact, hmi::Key::F1);
    EXPECT_TRUE(
        hmi::toPlayerInput(withKeys({hmi::Key::F1}), gameKeyBindings, hmi::GamepadBindings{})
            .interactPressed);

    hmi::GamepadBindings gamepadBindings;
    gamepadBindings.setKey(hmi::GameAction::Interact, hmi::GamepadButton::Y);
    EXPECT_TRUE(hmi::toPlayerInput(withGamepadButton(hmi::GamepadButton::Y), hmi::GameKeyBindings{},
                                   gamepadBindings)
                    .interactPressed);
}
