// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: PolyForm-Noncommercial-1.0.0

/**
 * @file test_input_state.cpp
 * @brief Tests unitaires de l'état des entrées : fronts clavier/souris et position.
 */

#include <gtest/gtest.h>

#include "HMI/Input/InputState.h"

/**
 * @brief Une touche passée d'« absente » à « présente » est « pressée » exactement une frame.
 * \castest{<b>Une touche passée d'« absente » à « présente » est « pressée » exactement une
 * frame.</b><br/>
 * \tcat Unitaire · Input State<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et
 * verifier les assertions.<br/>
 * \tattendu Une touche passée d'« absente » à « présente » est « pressée » exactement une frame.
 * }
 */
TEST(InputStateTest, FrontMontantClavier) {
    hmi::InputState input;

    input.beginFrame();
    input.onKeyDown(hmi::Key::Enter);
    EXPECT_TRUE(input.keyDown(hmi::Key::Enter));
    EXPECT_TRUE(input.keyPressed(hmi::Key::Enter));
    EXPECT_FALSE(input.keyReleased(hmi::Key::Enter));

    // Frame suivante sans nouvel événement : toujours maintenue, mais plus « pressée ».
    input.beginFrame();
    EXPECT_TRUE(input.keyDown(hmi::Key::Enter));
    EXPECT_FALSE(input.keyPressed(hmi::Key::Enter));
}

/**
 * @brief Une touche restée enfoncée n'est « pressée » qu'à la première frame.
 * \castest{<b>Une touche restée enfoncée n'est « pressée » qu'à la première frame.</b><br/>
 * \tcat Unitaire · Input State<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et
 * verifier les assertions.<br/>
 * \tattendu Une touche restée enfoncée n'est « pressée » qu'à la première frame.
 * }
 */
TEST(InputStateTest, MaintienClavier) {
    hmi::InputState input;

    input.beginFrame();
    input.onKeyDown(hmi::Key::Down);
    EXPECT_TRUE(input.keyPressed(hmi::Key::Down));

    for (int frame = 0; frame < 3; ++frame) {
        input.beginFrame();  // aucune nouvelle entrée : la touche reste enfoncée
        EXPECT_TRUE(input.keyDown(hmi::Key::Down));
        EXPECT_FALSE(input.keyPressed(hmi::Key::Down));
        EXPECT_FALSE(input.keyReleased(hmi::Key::Down));
    }
}

/**
 * @brief Le relâchement d'une touche est détecté « relâchée » pendant exactement une frame.
 * \castest{<b>Le relâchement d'une touche est détecté « relâchée » pendant exactement une
 * frame.</b><br/>
 * \tcat Unitaire · Input State<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et
 * verifier les assertions.<br/>
 * \tattendu Le relâchement d'une touche est détecté « relâchée » pendant exactement une frame.
 * }
 */
TEST(InputStateTest, FrontDescendantClavier) {
    hmi::InputState input;

    input.beginFrame();
    input.onKeyDown(hmi::Key::Space);
    input.beginFrame();  // maintenue

    input.beginFrame();
    input.onKeyUp(hmi::Key::Space);
    EXPECT_FALSE(input.keyDown(hmi::Key::Space));
    EXPECT_TRUE(input.keyReleased(hmi::Key::Space));

    input.beginFrame();
    EXPECT_FALSE(input.keyReleased(hmi::Key::Space));
}

/**
 * @brief Les touches sont indépendantes : un front sur l'une n'affecte pas les autres.
 * \castest{<b>Les touches sont indépendantes : un front sur l'une n'affecte pas les
 * autres.</b><br/>
 * \tcat Unitaire · Input State<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et
 * verifier les assertions.<br/>
 * \tattendu Les touches sont indépendantes : un front sur l'une n'affecte pas les autres.
 * }
 */
TEST(InputStateTest, TouchesIndependantes) {
    hmi::InputState input;

    input.beginFrame();
    input.onKeyDown(hmi::Key::Up);
    EXPECT_TRUE(input.keyPressed(hmi::Key::Up));
    EXPECT_FALSE(input.keyDown(hmi::Key::Down));
    EXPECT_FALSE(input.keyPressed(hmi::Key::Down));
}

/**
 * @brief Un bouton de souris suit la même logique pressé/cliqué/relâché que les touches.
 * \castest{<b>Un bouton de souris suit la même logique pressé/cliqué/relâché que les
 * touches.</b><br/>
 * \tcat Unitaire · Input State<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et
 * verifier les assertions.<br/>
 * \tattendu Un bouton de souris suit la même logique pressé/cliqué/relâché que les touches.
 * }
 */
TEST(InputStateTest, BoutonSouris) {
    hmi::InputState input;

    input.beginFrame();
    input.onMouseButtonDown(hmi::MouseButton::Left);
    EXPECT_TRUE(input.mouseButtonDown(hmi::MouseButton::Left));
    EXPECT_TRUE(input.mouseButtonPressed(hmi::MouseButton::Left));

    input.beginFrame();  // maintenu
    EXPECT_TRUE(input.mouseButtonDown(hmi::MouseButton::Left));
    EXPECT_FALSE(input.mouseButtonPressed(hmi::MouseButton::Left));

    input.beginFrame();
    input.onMouseButtonUp(hmi::MouseButton::Left);
    EXPECT_FALSE(input.mouseButtonDown(hmi::MouseButton::Left));
    EXPECT_TRUE(input.mouseButtonReleased(hmi::MouseButton::Left));
}

/**
 * @brief La position de la souris reflète le dernier déplacement injecté.
 * \castest{<b>La position de la souris reflète le dernier déplacement injecté.</b><br/>
 * \tcat Unitaire · Input State<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et
 * verifier les assertions.<br/>
 * \tattendu La position de la souris reflète le dernier déplacement injecté.
 * }
 */
TEST(InputStateTest, PositionSouris) {
    hmi::InputState input;

    input.onMouseMove(42, 99);
    EXPECT_EQ(input.mouseX(), 42);
    EXPECT_EQ(input.mouseY(), 99);

    input.onMouseMove(-3, 7);  // coordonnées signées (souris hors zone client)
    EXPECT_EQ(input.mouseX(), -3);
    EXPECT_EQ(input.mouseY(), 7);
}

/**
 * @brief Les incréments de molette d'une frame s'additionnent et repartent de zéro ensuite.
 * \castest{<b>Les incréments de molette d'une frame s'additionnent et repartent de zéro
 * ensuite.</b><br/>
 * \tcat Unitaire · Input State<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et
 * verifier les assertions.<br/>
 * \tattendu Les incréments de molette d'une frame s'additionnent et repartent de zéro ensuite.
 * }
 */
TEST(InputStateTest, MoletteAccumuleEtSeReinitialise) {
    hmi::InputState input;

    input.beginFrame();
    EXPECT_EQ(input.wheelDelta(), 0);
    input.onMouseWheel(120);
    input.onMouseWheel(-40);
    EXPECT_EQ(input.wheelDelta(), 80);

    input.beginFrame();  // nouvelle frame sans nouvel événement molette
    EXPECT_EQ(input.wheelDelta(), 0);
}

/**
 * @brief Les caractères tapés s'accumulent dans l'ordre puis sont vidés à la frame suivante.
 * \castest{<b>Les caractères tapés s'accumulent dans l'ordre puis sont vidés à la frame
 * suivante.</b><br/>
 * \tcat Unitaire · Input State<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et
 * verifier les assertions.<br/>
 * \tattendu Les caractères tapés s'accumulent dans l'ordre puis sont vidés à la frame suivante.
 * }
 */
TEST(InputStateTest, CaracteresTapesAccumulesEtVides) {
    hmi::InputState input;

    input.beginFrame();
    EXPECT_TRUE(input.typedCharacters().empty());
    input.onCharTyped(L'N');
    input.onCharTyped(L'1');
    input.onCharTyped(L'\xE9');  // 'é', caractère accentué (saisie non anglophone, EX-EDIT-009)
    ASSERT_EQ(input.typedCharacters().size(), 3u);
    EXPECT_EQ(input.typedCharacters()[0], L'N');
    EXPECT_EQ(input.typedCharacters()[1], L'1');
    EXPECT_EQ(input.typedCharacters()[2], L'\xE9');

    input.beginFrame();
    EXPECT_TRUE(input.typedCharacters().empty());
}

/**
 * @brief Un bouton manette seul rend `keyDown`/`keyPressed` vrais, comme au clavier
 * (`EX-CTRL-002`).
 * \castest{<b>Un bouton manette seul rend `keyDown`/`keyPressed` vrais, comme au clavier.</b><br/>
 * \tcat Unitaire · Input State<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et
 * verifier les assertions.<br/>
 * \tattendu Un bouton manette seul rend `keyDown`/`keyPressed` vrais, comme au clavier.
 * }
 */
TEST(InputStateTest, ManetteSeuleActiveLaTouche) {
    hmi::InputState input;

    input.beginFrame();
    input.onGamepadKeyDown(hmi::Key::Enter);
    EXPECT_TRUE(input.keyDown(hmi::Key::Enter));
    EXPECT_TRUE(input.keyPressed(hmi::Key::Enter));

    input.beginFrame();  // maintenue, plus de front
    EXPECT_TRUE(input.keyDown(hmi::Key::Enter));
    EXPECT_FALSE(input.keyPressed(hmi::Key::Enter));

    input.beginFrame();
    input.onGamepadKeyUp(hmi::Key::Enter);
    EXPECT_FALSE(input.keyDown(hmi::Key::Enter));
    EXPECT_TRUE(input.keyReleased(hmi::Key::Enter));
}

/**
 * @brief Clavier et manette combinés sur la même touche ne produisent pas de double front.
 * \castest{<b>Clavier et manette combinés sur la même touche ne produisent pas de double
 * front.</b><br/>
 * \tcat Unitaire · Input State<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et
 * verifier les assertions.<br/>
 * \tattendu Clavier et manette combinés sur la même touche ne produisent pas de double front.
 * }
 */
TEST(InputStateTest, ClavierEtManetteMemeToucheUnSeulFront) {
    hmi::InputState input;

    // Les deux sources s'enfoncent la même frame : un seul front "pressée".
    input.beginFrame();
    input.onKeyDown(hmi::Key::Space);
    input.onGamepadKeyDown(hmi::Key::Space);
    EXPECT_TRUE(input.keyPressed(hmi::Key::Space));

    input.beginFrame();
    EXPECT_FALSE(input.keyPressed(hmi::Key::Space));  // toujours maintenue, plus de front

    // Le clavier relâche seul : la manette tient encore la touche, pas de relâchement.
    input.beginFrame();
    input.onKeyUp(hmi::Key::Space);
    EXPECT_TRUE(input.keyDown(hmi::Key::Space));
    EXPECT_FALSE(input.keyReleased(hmi::Key::Space));

    // La manette relâche à son tour : relâchement détecté cette fois.
    input.beginFrame();
    input.onGamepadKeyUp(hmi::Key::Space);
    EXPECT_FALSE(input.keyDown(hmi::Key::Space));
    EXPECT_TRUE(input.keyReleased(hmi::Key::Space));
}

/**
 * @brief La manette relâchée ne masque jamais une touche clavier réellement maintenue
 * (non-stomping, décision de cadrage LOT-20).
 * \castest{<b>La manette relâchée ne masque jamais une touche clavier réellement
 * maintenue.</b><br/>
 * \tcat Unitaire · Input State<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et
 * verifier les assertions.<br/>
 * \tattendu La manette relâchée ne masque jamais une touche clavier réellement maintenue.
 * }
 */
TEST(InputStateTest, ManetteRelacheeNeMasquePasLeClavier) {
    hmi::InputState input;

    input.beginFrame();
    input.onKeyDown(hmi::Key::Left);  // clavier seul, jamais touché par la manette
    EXPECT_TRUE(input.keyDown(hmi::Key::Left));

    // Sondage manette d'une frame sans manette connectee : relache la touche cote manette
    // (comme le ferait Window::pollGamepad), le clavier ne doit pas en être affecté.
    for (int frame = 0; frame < 3; ++frame) {
        input.beginFrame();
        input.onGamepadKeyUp(hmi::Key::Left);
        EXPECT_TRUE(input.keyDown(hmi::Key::Left));
        EXPECT_FALSE(input.keyReleased(hmi::Key::Left));
    }
}

/**
 * @brief `gamepadConnected` reflète le dernier `setGamepadConnected` appelé.
 * \castest{<b>`gamepadConnected` reflète le dernier `setGamepadConnected` appelé.</b><br/>
 * \tcat Unitaire · Input State<br/>
 * \tcrit Mineur<br/>
 * \tetapes 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et
 * verifier les assertions.<br/>
 * \tattendu `gamepadConnected` reflète le dernier `setGamepadConnected` appelé.
 * }
 */
TEST(InputStateTest, GamepadConnecteReecrasable) {
    hmi::InputState input;
    EXPECT_FALSE(input.gamepadConnected());

    input.setGamepadConnected(true);
    EXPECT_TRUE(input.gamepadConnected());

    input.setGamepadConnected(false);
    EXPECT_FALSE(input.gamepadConnected());
}

/**
 * @brief Un bouton manette (piste brute) passé d'« absent » à « présent » est « pressé »
 *        exactement une frame, comme une touche clavier.
 * \castest{<b>Un bouton manette (piste brute) est « pressé » exactement une frame.</b><br/>
 * \tcat Unitaire · Input State<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et
 * verifier les assertions.<br/>
 * \tattendu Un bouton manette (piste brute) est « pressé » exactement une frame.
 * }
 */
TEST(InputStateTest, FrontMontantBoutonManetteBrut) {
    hmi::InputState input;

    input.beginFrame();
    input.onGamepadButtonDown(hmi::GamepadButton::A);
    EXPECT_TRUE(input.gamepadButtonDown(hmi::GamepadButton::A));
    EXPECT_TRUE(input.gamepadButtonPressed(hmi::GamepadButton::A));

    input.beginFrame();
    EXPECT_TRUE(input.gamepadButtonDown(hmi::GamepadButton::A));
    EXPECT_FALSE(input.gamepadButtonPressed(hmi::GamepadButton::A));

    input.onGamepadButtonUp(hmi::GamepadButton::A);
    EXPECT_FALSE(input.gamepadButtonDown(hmi::GamepadButton::A));
}

/**
 * @brief Un bouton manette (piste brute) relâché après avoir été enfoncé est « relâché »
 *        exactement une frame (`LOT-63`, action Interagir).
 * \castest{<b>Un bouton manette (piste brute) est « relâché » exactement une frame.</b><br/>
 * \tcat Unitaire · Input State<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et
 * verifier les assertions.<br/>
 * \tattendu `gamepadButtonReleased` est vrai la frame où le bouton passe d'enfoncé à relâché,
 * faux avant et après.
 * }
 */
TEST(InputStateTest, FrontDescendantBoutonManetteBrut) {
    hmi::InputState input;

    input.onGamepadButtonDown(hmi::GamepadButton::X);
    input.beginFrame();
    EXPECT_FALSE(input.gamepadButtonReleased(hmi::GamepadButton::X));

    input.onGamepadButtonUp(hmi::GamepadButton::X);
    EXPECT_TRUE(input.gamepadButtonReleased(hmi::GamepadButton::X));

    input.beginFrame();
    EXPECT_FALSE(input.gamepadButtonReleased(hmi::GamepadButton::X));
}

/**
 * @brief La piste manette brute (`GamepadButton`) est indépendante de la fusion clavier/manette
 *        existante sur `Key` : l'une n'affecte jamais l'autre.
 * \castest{<b>La piste manette brute est indépendante de la fusion clavier/manette sur
 * Key.</b><br/>
 * \tcat Unitaire · Input State<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et
 * verifier les assertions.<br/>
 * \tattendu La piste manette brute est indépendante de la fusion clavier/manette sur Key.
 * }
 */
TEST(InputStateTest, PisteBrutIndependanteDeLaFusionKey) {
    hmi::InputState input;

    input.beginFrame();
    input.onGamepadButtonDown(hmi::GamepadButton::A);
    EXPECT_FALSE(input.keyDown(hmi::Key::Enter));  // aucune touche Key affectee

    input.onGamepadKeyDown(hmi::Key::Enter);
    EXPECT_FALSE(input.gamepadButtonDown(hmi::GamepadButton::B));  // aucun bouton brut affecte
}

/**
 * @brief `releaseAll()` relâche toutes les entrées maintenues sans produire de front « relâchée »,
 *        évitant les touches « collées » à la perte de focus (Alt+Tab).
 * \castest{<b>releaseAll relâche tout sans produire de front « relâchée ».</b><br/>
 * \tcat Unitaire · Input State<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Maintenir des entrées clavier/manette/souris.<br/>2. Appeler releaseAll et verifier
 * qu'aucune n'est plus enfoncee ni signalee « relâchée ».<br/>
 * \tattendu Toutes les entrees sont relachees, sans aucun front.
 * }
 */
TEST(InputStateTest, RelacheToutSansFront) {
    hmi::InputState input;

    input.beginFrame();
    input.onKeyDown(hmi::Key::Right);
    input.onGamepadKeyDown(hmi::Key::Space);
    input.onGamepadButtonDown(hmi::GamepadButton::A);
    input.onMouseButtonDown(hmi::MouseButton::Left);

    // Perte de focus : tout est relâché immédiatement, sans front « relâchée » (courant ET
    // précédent remis à zéro), pour ne pas déclencher une action de relâchement fantôme.
    input.releaseAll();
    EXPECT_FALSE(input.keyDown(hmi::Key::Right));
    EXPECT_FALSE(input.keyReleased(hmi::Key::Right));
    EXPECT_FALSE(input.keyDown(hmi::Key::Space));
    EXPECT_FALSE(input.keyReleased(hmi::Key::Space));
    EXPECT_FALSE(input.gamepadButtonDown(hmi::GamepadButton::A));
    EXPECT_FALSE(input.mouseButtonDown(hmi::MouseButton::Left));
    EXPECT_FALSE(input.mouseButtonReleased(hmi::MouseButton::Left));

    // La frame suivante ne fait pas non plus réapparaître de front.
    input.beginFrame();
    EXPECT_FALSE(input.keyReleased(hmi::Key::Right));
    EXPECT_FALSE(input.keyPressed(hmi::Key::Right));
}
