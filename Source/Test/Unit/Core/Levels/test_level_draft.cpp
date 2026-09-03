// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_level_draft.cpp
 * @brief Tests unitaires du modèle d'édition mutable (LOT-14, EX-EDIT-002 à EX-EDIT-007).
 */

#include <vector>

#include <gtest/gtest.h>

#include "Core/Levels/CameraFraming.h"
#include "Core/Levels/GridPosition.h"
#include "Core/Levels/LevelDraft.h"
#include "Core/Levels/LevelLoader.h"
#include "Core/Levels/Plane.h"
#include "Core/Levels/TileType.h"

namespace {

using core::GridPosition;
using core::LevelDraft;
using core::TileType;

}  // namespace

/**
 * @brief paintTile pose le type demandé sur la case visée.
 * \castest{<b>paintTile pose le type demandé sur la case visée.</b><br/>
 * \tcat Unitaire · Level Draft<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et
 * verifier les assertions.<br/>
 * \tattendu paintTile pose le type demandé sur la case visée.
 * }
 */
TEST(LevelDraftTest, PaintTilePoseLeType) {
    LevelDraft draft = LevelDraft::empty("N", 3, 3);
    draft.paintTile(1, 1, TileType::Solid);
    EXPECT_EQ(draft.tileMap().tile(1, 1), TileType::Solid);
}

/**
 * @brief Poser une seconde entrée déplace la première (unicité, EX-EDIT-004).
 * \castest{<b>Poser une seconde entrée déplace la première (unicité, EX-EDIT-004).</b><br/>
 * \tcat Unitaire · Level Draft<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et
 * verifier les assertions.<br/>
 * \tattendu Poser une seconde entrée déplace la première (unicité, EX-EDIT-004).
 * }
 */
TEST(LevelDraftTest, SetEntryDeplaceLEntreeExistante) {
    LevelDraft draft = LevelDraft::empty("N", 4, 4);
    draft.setEntry(0, 0);
    ASSERT_TRUE(draft.entry().has_value());
    EXPECT_EQ(*draft.entry(), (GridPosition{0, 0}));

    draft.setEntry(2, 2);
    EXPECT_EQ(*draft.entry(), (GridPosition{2, 2}));
    EXPECT_EQ(draft.tileMap().tile(0, 0), TileType::Empty);
    EXPECT_EQ(draft.tileMap().tile(2, 2), TileType::Entry);
}

/**
 * @brief Poser une seconde sortie déplace la première (unicité, EX-EDIT-004).
 * \castest{<b>Poser une seconde sortie déplace la première (unicité, EX-EDIT-004).</b><br/>
 * \tcat Unitaire · Level Draft<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et
 * verifier les assertions.<br/>
 * \tattendu Poser une seconde sortie déplace la première (unicité, EX-EDIT-004).
 * }
 */
TEST(LevelDraftTest, SetExitDeplaceLaSortieExistante) {
    LevelDraft draft = LevelDraft::empty("N", 4, 4);
    draft.setExit(1, 1);
    draft.setExit(3, 3);
    EXPECT_EQ(*draft.exit(), (GridPosition{3, 3}));
    EXPECT_EQ(draft.tileMap().tile(1, 1), TileType::Empty);
    EXPECT_EQ(draft.tileMap().tile(3, 3), TileType::Exit);
}

/**
 * @brief Peindre par-dessus l'entrée invalide la position d'entrée mémorisée.
 * \castest{<b>Peindre par-dessus l'entrée invalide la position d'entrée mémorisée.</b><br/>
 * \tcat Unitaire · Level Draft<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et
 * verifier les assertions.<br/>
 * \tattendu Peindre par-dessus l'entrée invalide la position d'entrée mémorisée.
 * }
 */
TEST(LevelDraftTest, PeindrePardessusLEntreeLInvalide) {
    LevelDraft draft = LevelDraft::empty("N", 3, 3);
    draft.setEntry(1, 1);
    draft.paintTile(1, 1, TileType::Solid);
    EXPECT_FALSE(draft.entry().has_value());
    EXPECT_EQ(draft.tileMap().tile(1, 1), TileType::Solid);
}

/**
 * @brief Lier un interrupteur à une porte crée un mécanisme ; le délier le retire.
 * \castest{<b>Lier un interrupteur à une porte crée un mécanisme ; le délier le retire.</b><br/>
 * \tcat Unitaire · Level Draft<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et
 * verifier les assertions.<br/>
 * \tattendu Lier un interrupteur à une porte crée un mécanisme ; le délier le retire.
 * }
 */
TEST(LevelDraftTest, LierPuisDelierUnMecanisme) {
    LevelDraft draft = LevelDraft::empty("N", 4, 4);
    draft.paintTile(0, 0, TileType::Switch);
    draft.paintTile(3, 3, TileType::Door);

    draft.linkMechanism(GridPosition{0, 0}, GridPosition{3, 3});
    ASSERT_EQ(draft.mechanisms().size(), 1u);
    EXPECT_EQ(draft.mechanisms().front().switchPosition, (GridPosition{0, 0}));
    EXPECT_EQ(draft.mechanisms().front().doorPosition, (GridPosition{3, 3}));

    draft.unlinkMechanism(GridPosition{3, 3});
    EXPECT_TRUE(draft.mechanisms().empty());
}

/**
 * @brief Lier une clé à une porte verrouillée crée un mécanisme, comme un interrupteur↔porte —
 * même liaison, aucune notion dupliquée (`EX-GP-023`, `LOT-63`).
 * \castest{<b>Lier une clé à une porte verrouillée crée un mécanisme.</b><br/>
 * \tcat Unitaire · Level Draft<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et
 * verifier les assertions.<br/>
 * \tattendu Lier une clé à une porte verrouillée crée un mécanisme dans le même vecteur.
 * }
 */
TEST(LevelDraftTest, LierUneCleAUnePorteVerrouillee) {
    LevelDraft draft = LevelDraft::empty("N", 4, 4);
    draft.paintTile(0, 0, TileType::Key);
    draft.paintTile(3, 3, TileType::LockedDoor);

    draft.linkMechanism(GridPosition{0, 0}, GridPosition{3, 3});
    ASSERT_EQ(draft.mechanisms().size(), 1u);
    EXPECT_EQ(draft.mechanisms().front().switchPosition, (GridPosition{0, 0}));
    EXPECT_EQ(draft.mechanisms().front().doorPosition, (GridPosition{3, 3}));
}

/**
 * @brief Poser une clé et une porte verrouillée, les lier, enregistrer (`toLevel`) puis recharger
 * donne un modèle identique — round-trip complet du geste éditeur (`EX-EDIT-011`, `LOT-63`).
 * \castest{<b>Poser, lier, enregistrer et recharger une clé/porte verrouillée donne un modèle
 * identique.</b><br/>
 * \tcat Unitaire · Level Draft<br/>
 * \tcrit Critique<br/>
 * \tetapes 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et
 * verifier les assertions.<br/>
 * \tattendu Le niveau rechargé porte la même liaison clé↔porte verrouillée que le brouillon.
 * }
 */
TEST(LevelDraftTest, RoundTripEditionCleEtPorteVerrouillee) {
    LevelDraft draft = LevelDraft::empty("N", 4, 4);
    draft.setEntry(0, 1);
    draft.setExit(3, 2);
    draft.paintTile(1, 0, TileType::Key);
    draft.paintTile(2, 0, TileType::LockedDoor);
    draft.linkMechanism(GridPosition{1, 0}, GridPosition{2, 0});

    const core::LevelLoadResult reloaded = draft.toLevel();
    ASSERT_TRUE(reloaded.ok()) << reloaded.error;
    ASSERT_EQ(reloaded.level->mechanisms().size(), 1u);
    EXPECT_EQ(reloaded.level->mechanisms().front().switchPosition, (GridPosition{1, 0}));
    EXPECT_EQ(reloaded.level->mechanisms().front().doorPosition, (GridPosition{2, 0}));
    EXPECT_EQ(reloaded.level->tileMap().tile(1, 0), TileType::Key);
    EXPECT_EQ(reloaded.level->tileMap().tile(2, 0), TileType::LockedDoor);
}

/**
 * @brief Lier une plaque de pression à une porte crée un mécanisme, comme un interrupteur
 * (`EX-GP-025`).
 * \castest{<b>Lier une plaque de pression à une porte crée un mécanisme.</b><br/>
 * \tcat Unitaire · Level Draft<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et
 * verifier les assertions.<br/>
 * \tattendu Lier une plaque de pression à une porte crée un mécanisme, comme un interrupteur.
 * }
 */
TEST(LevelDraftTest, LierUnePlaqueDePression) {
    LevelDraft draft = LevelDraft::empty("N", 4, 4);
    draft.paintTile(0, 0, TileType::PressurePlate);
    draft.paintTile(3, 3, TileType::Door);

    draft.linkMechanism(GridPosition{0, 0}, GridPosition{3, 3});
    ASSERT_EQ(draft.mechanisms().size(), 1u);
    EXPECT_EQ(draft.mechanisms().front().switchPosition, (GridPosition{0, 0}));
    EXPECT_EQ(draft.mechanisms().front().doorPosition, (GridPosition{3, 3}));
}

/**
 * @brief Relier une porte déjà liée remplace la liaison précédente (une porte, un interrupteur).
 * \castest{<b>Relier une porte déjà liée remplace la liaison précédente (une porte, un
 * interrupteur).</b><br/>
 * \tcat Unitaire · Level Draft<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et
 * verifier les assertions.<br/>
 * \tattendu Relier une porte déjà liée remplace la liaison précédente (une porte, un
 * interrupteur).
 * }
 */
TEST(LevelDraftTest, RelierUnePorteRemplaceLaLiaisonPrecedente) {
    LevelDraft draft = LevelDraft::empty("N", 4, 4);
    draft.paintTile(0, 0, TileType::Switch);
    draft.paintTile(1, 0, TileType::Switch);
    draft.paintTile(3, 3, TileType::Door);

    draft.linkMechanism(GridPosition{0, 0}, GridPosition{3, 3});
    draft.linkMechanism(GridPosition{1, 0}, GridPosition{3, 3});

    ASSERT_EQ(draft.mechanisms().size(), 1u);
    EXPECT_EQ(draft.mechanisms().front().switchPosition, (GridPosition{1, 0}));
}

/**
 * @brief Un interrupteur peut ouvrir plusieurs portes.
 * \castest{<b>Un interrupteur peut ouvrir plusieurs portes.</b><br/>
 * \tcat Unitaire · Level Draft<br/>
 * \tcrit Mineur<br/>
 * \tetapes 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et
 * verifier les assertions.<br/>
 * \tattendu Un interrupteur peut ouvrir plusieurs portes.
 * }
 */
TEST(LevelDraftTest, UnInterrupteurPeutOuvrirPlusieursPortes) {
    LevelDraft draft = LevelDraft::empty("N", 4, 4);
    draft.paintTile(0, 0, TileType::Switch);
    draft.paintTile(3, 3, TileType::Door);
    draft.paintTile(3, 0, TileType::Door);

    draft.linkMechanism(GridPosition{0, 0}, GridPosition{3, 3});
    draft.linkMechanism(GridPosition{0, 0}, GridPosition{3, 0});

    EXPECT_EQ(draft.mechanisms().size(), 2u);
}

/**
 * @brief Peindre par-dessus un interrupteur retire les liaisons qui le référencent.
 * \castest{<b>Peindre par-dessus un interrupteur retire les liaisons qui le référencent.</b><br/>
 * \tcat Unitaire · Level Draft<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et
 * verifier les assertions.<br/>
 * \tattendu Peindre par-dessus un interrupteur retire les liaisons qui le référencent.
 * }
 */
TEST(LevelDraftTest, PeindrePardessusUnInterrupteurRetireSesLiaisons) {
    LevelDraft draft = LevelDraft::empty("N", 4, 4);
    draft.paintTile(0, 0, TileType::Switch);
    draft.paintTile(3, 3, TileType::Door);
    draft.linkMechanism(GridPosition{0, 0}, GridPosition{3, 3});

    draft.paintTile(0, 0, TileType::Empty);
    EXPECT_TRUE(draft.mechanisms().empty());
}

/**
 * @brief Agrandir la grille conserve le contenu existant et complète en cases vides.
 * \castest{<b>Agrandir la grille conserve le contenu existant et complète en cases
 * vides.</b><br/>
 * \tcat Unitaire · Level Draft<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et
 * verifier les assertions.<br/>
 * \tattendu Agrandir la grille conserve le contenu existant et complète en cases vides.
 * }
 */
TEST(LevelDraftTest, AgrandirConserveLeContenu) {
    LevelDraft draft = LevelDraft::empty("N", 3, 3);
    draft.setEntry(1, 1);
    draft.resize(5, 5);

    EXPECT_EQ(draft.tileMap().width(), 5);
    EXPECT_EQ(draft.tileMap().height(), 5);
    EXPECT_EQ(draft.tileMap().tile(1, 1), TileType::Entry);
    EXPECT_EQ(*draft.entry(), (GridPosition{1, 1}));
    EXPECT_EQ(draft.tileMap().tile(4, 4), TileType::Empty);
}

/**
 * @brief Réduire la grille tronque le contenu hors bornes et invalide l'entrée perdue.
 * \castest{<b>Réduire la grille tronque le contenu hors bornes et invalide l'entrée
 * perdue.</b><br/>
 * \tcat Unitaire · Level Draft<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et
 * verifier les assertions.<br/>
 * \tattendu Réduire la grille tronque le contenu hors bornes et invalide l'entrée perdue.
 * }
 */
TEST(LevelDraftTest, ReduireTronqueEtInvalideLEntreePerdue) {
    LevelDraft draft = LevelDraft::empty("N", 5, 5);
    draft.setEntry(4, 4);
    draft.paintTile(1, 1, TileType::Solid);

    draft.resize(3, 3);

    EXPECT_EQ(draft.tileMap().width(), 3);
    EXPECT_EQ(draft.tileMap().height(), 3);
    EXPECT_FALSE(draft.entry().has_value());
    EXPECT_EQ(draft.tileMap().tile(1, 1), TileType::Solid);
}

/**
 * @brief Réduire la grille retire les mécanismes dont une extrémité sort des bornes.
 * \castest{<b>Réduire la grille retire les mécanismes dont une extrémité sort des
 * bornes.</b><br/>
 * \tcat Unitaire · Level Draft<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et
 * verifier les assertions.<br/>
 * \tattendu Réduire la grille retire les mécanismes dont une extrémité sort des bornes.
 * }
 */
TEST(LevelDraftTest, ReduireRetireLesMecanismesHorsBornes) {
    LevelDraft draft = LevelDraft::empty("N", 5, 5);
    draft.paintTile(0, 0, TileType::Switch);
    draft.paintTile(4, 4, TileType::Door);
    draft.linkMechanism(GridPosition{0, 0}, GridPosition{4, 4});

    draft.resize(2, 2);
    EXPECT_TRUE(draft.mechanisms().empty());
}

/**
 * @brief toLevel() sur un brouillon sans sortie échoue avec un message récupérable (EX-EDIT-007).
 * \castest{<b>toLevel() sur un brouillon sans sortie échoue avec un message récupérable
 * (EX-EDIT-007).</b><br/>
 * \tcat Unitaire · Level Draft<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et
 * verifier les assertions.<br/>
 * \tattendu toLevel() sur un brouillon sans sortie échoue avec un message récupérable
 * (EX-EDIT-007).
 * }
 */
TEST(LevelDraftTest, ToLevelSansSortieEchoueProprement) {
    LevelDraft draft = LevelDraft::empty("Incomplet", 4, 4);
    draft.setEntry(0, 0);

    const core::LevelLoadResult result = draft.toLevel();
    EXPECT_FALSE(result.ok());
    EXPECT_FALSE(result.error.empty());
}

/**
 * @brief Un brouillon reconstruit depuis un niveau existant restitue son contenu.
 * \castest{<b>Un brouillon reconstruit depuis un niveau existant restitue son contenu.</b><br/>
 * \tcat Unitaire · Level Draft<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et
 * verifier les assertions.<br/>
 * \tattendu Un brouillon reconstruit depuis un niveau existant restitue son contenu.
 * }
 */
TEST(LevelDraftTest, FromLevelRestitueLeContenu) {
    const core::LevelLoadResult loaded = core::LevelLoader::loadFromString(R"({
        "name": "Depart", "width": 3, "height": 3,
        "tiles": [ {"x":0,"y":0,"type":"entry"}, {"x":2,"y":2,"type":"exit"} ] })");
    ASSERT_TRUE(loaded.ok()) << loaded.error;

    const LevelDraft draft = LevelDraft::fromLevel(*loaded.level);
    EXPECT_EQ(draft.name(), "Depart");
    EXPECT_EQ(*draft.entry(), (GridPosition{0, 0}));
    EXPECT_EQ(*draft.exit(), (GridPosition{2, 2}));
}

/**
 * @brief Un brouillon neuf ne peut ni annuler ni refaire.
 * \castest{<b>Un brouillon neuf ne peut ni annuler ni refaire.</b><br/>
 * \tcat Unitaire · Level Draft<br/>
 * \tcrit Mineur<br/>
 * \tetapes 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et
 * verifier les assertions.<br/>
 * \tattendu Un brouillon neuf ne peut ni annuler ni refaire.
 * }
 */
TEST(LevelDraftTest, BrouillonNeufSansHistorique) {
    const LevelDraft draft = LevelDraft::empty("N", 4, 4);
    EXPECT_FALSE(draft.canUndo());
    EXPECT_FALSE(draft.canRedo());
}

/**
 * @brief undo() après une peinture restitue l'état exact précédent.
 * \castest{<b>undo() après une peinture restitue l'état exact précédent.</b><br/>
 * \tcat Unitaire · Level Draft<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et
 * verifier les assertions.<br/>
 * \tattendu undo() après une peinture restitue l'état exact précédent.
 * }
 */
TEST(LevelDraftTest, UndoApresPeintureRestitueLEtatPrecedent) {
    LevelDraft draft = LevelDraft::empty("N", 3, 3);
    draft.paintTile(1, 1, TileType::Solid);
    ASSERT_TRUE(draft.canUndo());

    const bool undone = draft.undo();

    EXPECT_TRUE(undone);
    EXPECT_EQ(draft.tileMap().tile(1, 1), TileType::Empty);
    EXPECT_FALSE(draft.canUndo());
    EXPECT_TRUE(draft.canRedo());
}

/**
 * @brief redo() après un undo() restitue l'état muté.
 * \castest{<b>redo() après un undo() restitue l'état muté.</b><br/>
 * \tcat Unitaire · Level Draft<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et
 * verifier les assertions.<br/>
 * \tattendu redo() après un undo() restitue l'état muté.
 * }
 */
TEST(LevelDraftTest, RedoApresUndoRestitueLEtatMute) {
    LevelDraft draft = LevelDraft::empty("N", 3, 3);
    draft.paintTile(1, 1, TileType::Solid);
    draft.undo();

    const bool redone = draft.redo();

    EXPECT_TRUE(redone);
    EXPECT_EQ(draft.tileMap().tile(1, 1), TileType::Solid);
    EXPECT_TRUE(draft.canUndo());
    EXPECT_FALSE(draft.canRedo());
}

/**
 * @brief Une séquence de N mutations suivie de N undo() restitue l'état initial exact.
 * \castest{<b>Une séquence de N mutations suivie de N undo() restitue l'état initial
 * exact.</b><br/>
 * \tcat Unitaire · Level Draft<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et
 * verifier les assertions.<br/>
 * \tattendu Une séquence de N mutations suivie de N undo() restitue l'état initial exact.
 * }
 */
TEST(LevelDraftTest, SequenceDeMutationsPuisUndoRestitueLEtatInitial) {
    LevelDraft draft = LevelDraft::empty("N", 5, 5);
    draft.paintTile(0, 0, TileType::Solid);
    draft.setEntry(1, 1);
    draft.setExit(4, 4);
    draft.paintTile(2, 2, TileType::Danger);

    ASSERT_TRUE(draft.undo());
    ASSERT_TRUE(draft.undo());
    ASSERT_TRUE(draft.undo());
    ASSERT_TRUE(draft.undo());

    EXPECT_FALSE(draft.canUndo());
    EXPECT_EQ(draft.tileMap().tile(0, 0), TileType::Empty);
    EXPECT_EQ(draft.tileMap().tile(1, 1), TileType::Empty);
    EXPECT_EQ(draft.tileMap().tile(2, 2), TileType::Empty);
    EXPECT_EQ(draft.tileMap().tile(4, 4), TileType::Empty);
    EXPECT_FALSE(draft.entry().has_value());
    EXPECT_FALSE(draft.exit().has_value());
}

/**
 * @brief Une nouvelle mutation après un undo() invalide la branche de refaire.
 * \castest{<b>Une nouvelle mutation après un undo() invalide la branche de refaire.</b><br/>
 * \tcat Unitaire · Level Draft<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et
 * verifier les assertions.<br/>
 * \tattendu Une nouvelle mutation après un undo() invalide la branche de refaire.
 * }
 */
TEST(LevelDraftTest, MutationApresUndoInvalideLeRefaire) {
    LevelDraft draft = LevelDraft::empty("N", 3, 3);
    draft.paintTile(0, 0, TileType::Solid);
    draft.undo();
    ASSERT_TRUE(draft.canRedo());

    draft.paintTile(1, 1, TileType::Danger);

    EXPECT_FALSE(draft.canRedo());
    EXPECT_EQ(draft.tileMap().tile(1, 1), TileType::Danger);
}

/**
 * @brief undo()/redo() sur une pile vide est sans effet (pas de plantage).
 * \castest{<b>undo()/redo() sur une pile vide est sans effet (pas de plantage).</b><br/>
 * \tcat Unitaire · Level Draft<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et
 * verifier les assertions.<br/>
 * \tattendu undo()/redo() sur une pile vide est sans effet (pas de plantage).
 * }
 */
TEST(LevelDraftTest, UndoRedoSurPileVideSansEffet) {
    LevelDraft draft = LevelDraft::empty("N", 3, 3);
    EXPECT_FALSE(draft.undo());
    EXPECT_FALSE(draft.redo());
    EXPECT_EQ(draft.tileMap().tile(0, 0), TileType::Empty);
}

/**
 * @brief L'annulation d'une liaison de mécanisme restitue la liaison précédente.
 * \castest{<b>L'annulation d'une liaison de mécanisme restitue la liaison précédente.</b><br/>
 * \tcat Unitaire · Level Draft<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et
 * verifier les assertions.<br/>
 * \tattendu L'annulation d'une liaison de mécanisme restitue la liaison précédente.
 * }
 */
TEST(LevelDraftTest, UndoApresLiaisonMecanismeRestitueLAbsenceDeLiaison) {
    LevelDraft draft = LevelDraft::empty("N", 4, 4);
    draft.paintTile(0, 0, TileType::Switch);
    draft.paintTile(3, 3, TileType::Door);
    draft.linkMechanism(GridPosition{0, 0}, GridPosition{3, 3});
    ASSERT_EQ(draft.mechanisms().size(), 1u);

    draft.undo();

    EXPECT_TRUE(draft.mechanisms().empty());
}

/**
 * @brief setBackground/setSkinSet assignent et retirent les champs de fond et de jeu de skins
 * (`EX-REN-044`, `EX-EDIT-024`), sans effet l'un sur l'autre.
 * \castest{<b>setBackground/setSkinSet assignent et retirent les champs correspondants.</b><br/>
 * \tcat Unitaire · Level Draft<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et
 * verifier les assertions.<br/>
 * \tattendu setBackground/setSkinSet assignent et retirent les champs correspondants.
 * }
 */
TEST(LevelDraftTest, SetBackgroundEtSetSkinSetAssignentEtRetirent) {
    LevelDraft draft = LevelDraft::empty("N", 3, 3);
    EXPECT_FALSE(draft.background().has_value());
    EXPECT_FALSE(draft.skinSet().has_value());

    draft.setBackground(std::string{"forest.png"});
    ASSERT_TRUE(draft.background().has_value());
    EXPECT_EQ(*draft.background(), "forest.png");
    EXPECT_FALSE(draft.skinSet().has_value());  // pas de contamination croisee

    draft.setSkinSet(std::string{"foret"});
    ASSERT_TRUE(draft.skinSet().has_value());
    EXPECT_EQ(*draft.skinSet(), "foret");

    draft.setBackground(std::nullopt);
    EXPECT_FALSE(draft.background().has_value());
    ASSERT_TRUE(draft.skinSet().has_value());  // retirer le fond ne touche pas le jeu de skins
    EXPECT_EQ(*draft.skinSet(), "foret");
}

/**
 * @brief L'annulation d'un changement de fond ou de jeu de skins restitue la valeur précédente
 * (`EX-REN-044`, `EX-EDIT-024`).
 * \castest{<b>L'annulation d'un changement de fond ou de jeu de skins restitue la valeur
 * precedente.</b><br/>
 * \tcat Unitaire · Level Draft<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et
 * verifier les assertions.<br/>
 * \tattendu L'annulation d'un changement de fond ou de jeu de skins restitue la valeur precedente.
 * }
 */
TEST(LevelDraftTest, UndoRedoApresChangementDeFondEtDeJeuDeSkins) {
    LevelDraft draft = LevelDraft::empty("N", 3, 3);
    draft.setBackground(std::string{"forest.png"});
    draft.setSkinSet(std::string{"foret"});
    ASSERT_TRUE(draft.canUndo());

    ASSERT_TRUE(draft.undo());  // annule setSkinSet
    EXPECT_FALSE(draft.skinSet().has_value());
    ASSERT_TRUE(draft.background().has_value());
    EXPECT_EQ(*draft.background(), "forest.png");

    ASSERT_TRUE(draft.undo());  // annule setBackground
    EXPECT_FALSE(draft.background().has_value());

    ASSERT_TRUE(draft.redo());  // refait setBackground
    ASSERT_TRUE(draft.background().has_value());
    EXPECT_EQ(*draft.background(), "forest.png");
}

/**
 * @brief Un brouillon vierge a le cadrage par défaut (niveau entier) ; `setCameraFraming` change
 * le mode et l'annulation restitue le précédent (`EX-LVL-006`, `EX-EDIT-028`).
 * \castest{<b>setCameraFraming change le mode de cadrage et s'annule.</b><br/>
 * \tcat Unitaire · Level Draft<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Créer un brouillon vierge et vérifier son cadrage par défaut.<br/>2. Appeler
 * `setCameraFraming` avec le mode *suivi*.<br/>3. Annuler.<br/>
 * \tattendu Le cadrage par défaut d'un brouillon vierge est *niveau entier* ; `setCameraFraming`
 * remplace le mode courant ; l'annulation restitue le cadrage précédent.
 * }
 */
TEST(LevelDraftTest, SetCameraFramingChangeLeModeEtSAnnule) {
    LevelDraft draft = LevelDraft::empty("N", 3, 3);
    EXPECT_EQ(draft.cameraFraming().mode, core::CameraFramingMode::WholeLevel);

    draft.setCameraFraming(core::CameraFramingConfig{.mode = core::CameraFramingMode::Follow});
    EXPECT_EQ(draft.cameraFraming().mode, core::CameraFramingMode::Follow);
    ASSERT_TRUE(draft.canUndo());

    ASSERT_TRUE(draft.undo());
    EXPECT_EQ(draft.cameraFraming().mode, core::CameraFramingMode::WholeLevel);
}

/**
 * @brief `addCameraZone` ajoute une zone en fin de liste, annulable ; `removeCameraZone` la
 * retire, également annulable (`EX-LVL-007`, `EX-EDIT-029`).
 * \castest{<b>addCameraZone/removeCameraZone ajoutent et retirent une zone, annulables.</b><br/>
 * \tcat Unitaire · Level Draft<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Ajouter deux zones sur un brouillon vierge.<br/>2. Retirer la première.<br/>3.
 * Annuler le retrait, puis annuler les deux ajouts.<br/>
 * \tattendu Chaque étape restitue l'état attendu ; l'annulation complète revient à une liste
 * vide.
 * }
 */
TEST(LevelDraftTest, AddEtRemoveCameraZoneSontAnnulables) {
    LevelDraft draft = LevelDraft::empty("N", 30, 20);
    EXPECT_TRUE(draft.cameraFraming().zones.empty());

    draft.addCameraZone(core::CameraZone{.x = 0, .y = 0, .width = 10, .height = 10});
    draft.addCameraZone(core::CameraZone{.x = 10, .y = 0, .width = 10, .height = 10});
    ASSERT_EQ(draft.cameraFraming().zones.size(), 2u);
    EXPECT_EQ(draft.cameraFraming().zones[0].x, 0);
    EXPECT_EQ(draft.cameraFraming().zones[1].x, 10);

    draft.removeCameraZone(0);
    ASSERT_EQ(draft.cameraFraming().zones.size(), 1u);
    EXPECT_EQ(draft.cameraFraming().zones[0].x, 10);  // la seconde zone reste, l'index se decale

    ASSERT_TRUE(draft.undo());  // annule removeCameraZone
    ASSERT_EQ(draft.cameraFraming().zones.size(), 2u);

    ASSERT_TRUE(draft.undo());  // annule le second addCameraZone
    ASSERT_TRUE(draft.undo());  // annule le premier addCameraZone
    EXPECT_TRUE(draft.cameraFraming().zones.empty());
}

/**
 * @brief `removeCameraZone` sur un index hors bornes reste sans effet (`EX-NFR-040`).
 * \castest{<b>removeCameraZone sur un index hors bornes reste sans effet.</b><br/>
 * \tcat Unitaire · Level Draft<br/>
 * \tcrit Mineur<br/>
 * \tetapes 1. Appeler `removeCameraZone` avec un index hors bornes sur une liste vide.<br/>
 * \tattendu Aucun effet : la liste reste vide, aucune entrée d'historique n'est empilée.
 * }
 */
TEST(LevelDraftTest, RemoveCameraZoneHorsBornesSansEffet) {
    LevelDraft draft = LevelDraft::empty("N", 30, 20);
    draft.removeCameraZone(0);
    EXPECT_TRUE(draft.cameraFraming().zones.empty());
    EXPECT_FALSE(draft.canUndo());
}

/**
 * @brief paintRegion applique un bloc homogène comme une succession de paintTile équivalente.
 * \castest{<b>paintRegion applique un bloc homogène comme une succession de paintTile
 * équivalente.</b><br/>
 * \tcat Unitaire · Level Draft<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et
 * verifier les assertions.<br/>
 * \tattendu paintRegion applique un bloc homogène comme une succession de paintTile équivalente.
 * }
 */
TEST(LevelDraftTest, PaintRegionAppliqueLeBlocEntier) {
    LevelDraft draft = LevelDraft::empty("N", 5, 5);
    const std::vector<std::vector<TileType>> block = {
        {TileType::Solid, TileType::Solid},
        {TileType::Solid, TileType::Solid},
    };
    draft.paintRegion(1, 1, block);

    EXPECT_EQ(draft.tileMap().tile(1, 1), TileType::Solid);
    EXPECT_EQ(draft.tileMap().tile(2, 1), TileType::Solid);
    EXPECT_EQ(draft.tileMap().tile(1, 2), TileType::Solid);
    EXPECT_EQ(draft.tileMap().tile(2, 2), TileType::Solid);
    EXPECT_EQ(draft.tileMap().tile(0, 0), TileType::Empty);
}

/**
 * @brief paintRegion ne pousse qu'un seul snapshot undo pour tout le bloc.
 * \castest{<b>paintRegion ne pousse qu'un seul snapshot undo pour tout le bloc.</b><br/>
 * \tcat Unitaire · Level Draft<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et
 * verifier les assertions.<br/>
 * \tattendu paintRegion ne pousse qu'un seul snapshot undo pour tout le bloc.
 * }
 */
TEST(LevelDraftTest, PaintRegionUnSeulSnapshotUndo) {
    LevelDraft draft = LevelDraft::empty("N", 5, 5);
    const std::vector<std::vector<TileType>> block = {
        {TileType::Danger, TileType::Danger, TileType::Danger},
    };
    draft.paintRegion(0, 0, block);

    ASSERT_TRUE(draft.undo());
    EXPECT_FALSE(draft.canUndo());
    EXPECT_EQ(draft.tileMap().tile(0, 0), TileType::Empty);
    EXPECT_EQ(draft.tileMap().tile(1, 0), TileType::Empty);
    EXPECT_EQ(draft.tileMap().tile(2, 0), TileType::Empty);
}

/**
 * @brief paintRegion découpe silencieusement le bloc aux bords de la grille.
 * \castest{<b>paintRegion découpe silencieusement le bloc aux bords de la grille.</b><br/>
 * \tcat Unitaire · Level Draft<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et
 * verifier les assertions.<br/>
 * \tattendu paintRegion découpe silencieusement le bloc aux bords de la grille.
 * }
 */
TEST(LevelDraftTest, PaintRegionDecoupeAuxBords) {
    LevelDraft draft = LevelDraft::empty("N", 3, 3);
    const std::vector<std::vector<TileType>> block = {
        {TileType::Solid, TileType::Solid, TileType::Solid},
        {TileType::Solid, TileType::Solid, TileType::Solid},
    };
    draft.paintRegion(1, 2, block);  // deborde largeur (colonne 3) et hauteur (ligne 3)

    EXPECT_EQ(draft.tileMap().tile(1, 2), TileType::Solid);
    EXPECT_EQ(draft.tileMap().tile(2, 2), TileType::Solid);
}

/**
 * @brief paintRegion qui inclut une position d'entrée déplace l'entrée existante (même sémantique
 *        que paintTile).
 * \castest{<b>paintRegion qui inclut une position d'entrée déplace l'entrée existante.</b><br/>
 * \tcat Unitaire · Level Draft<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et
 * verifier les assertions.<br/>
 * \tattendu paintRegion qui inclut une position d'entrée déplace l'entrée existante.
 * }
 */
TEST(LevelDraftTest, PaintRegionDeplaceLEntree) {
    LevelDraft draft = LevelDraft::empty("N", 4, 4);
    draft.setEntry(0, 0);
    const std::vector<std::vector<TileType>> block = {{TileType::Entry}};
    draft.paintRegion(2, 2, block);

    EXPECT_EQ(draft.tileMap().tile(0, 0), TileType::Empty);
    EXPECT_EQ(*draft.entry(), (GridPosition{2, 2}));
}

/**
 * @brief paintRegion qui recouvre un interrupteur retire les liaisons qui le référencent.
 * \castest{<b>paintRegion qui recouvre un interrupteur retire les liaisons qui le
 * référencent.</b><br/>
 * \tcat Unitaire · Level Draft<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et
 * verifier les assertions.<br/>
 * \tattendu paintRegion qui recouvre un interrupteur retire les liaisons qui le référencent.
 * }
 */
TEST(LevelDraftTest, PaintRegionRetireLesLiaisonsRecouvertes) {
    LevelDraft draft = LevelDraft::empty("N", 4, 4);
    draft.paintTile(0, 0, TileType::Switch);
    draft.paintTile(3, 3, TileType::Door);
    draft.linkMechanism(GridPosition{0, 0}, GridPosition{3, 3});

    const std::vector<std::vector<TileType>> block = {{TileType::Empty}};
    draft.paintRegion(0, 0, block);

    EXPECT_TRUE(draft.mechanisms().empty());
}

/**
 * @brief paintRegion avec un bloc vide est sans effet (pas de snapshot undo créé).
 * \castest{<b>paintRegion avec un bloc vide est sans effet.</b><br/>
 * \tcat Unitaire · Level Draft<br/>
 * \tcrit Mineur<br/>
 * \tetapes 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et
 * verifier les assertions.<br/>
 * \tattendu paintRegion avec un bloc vide est sans effet.
 * }
 */
TEST(LevelDraftTest, PaintRegionBlocVideSansEffet) {
    LevelDraft draft = LevelDraft::empty("N", 3, 3);
    draft.paintRegion(0, 0, {});
    EXPECT_FALSE(draft.canUndo());
}

/**
 * @brief wouldResizeDropContent détecte la perte de l'entrée, de la sortie ou d'une liaison.
 * \castest{<b>wouldResizeDropContent détecte la perte de l'entrée, de la sortie ou d'une
 * liaison.</b><br/>
 * \tcat Unitaire · Level Draft<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et
 * verifier les assertions.<br/>
 * \tattendu wouldResizeDropContent détecte la perte de l'entrée, de la sortie ou d'une liaison.
 * }
 */
TEST(LevelDraftTest, WouldResizeDropContentDetecteLaPerte) {
    LevelDraft draft = LevelDraft::empty("N", 5, 5);
    draft.setEntry(4, 4);
    draft.setExit(0, 4);
    draft.paintTile(4, 0, TileType::Switch);
    draft.paintTile(0, 0, TileType::Door);
    draft.linkMechanism(GridPosition{4, 0}, GridPosition{0, 0});

    // L'entree (4,4) sortirait des bornes d'une grille 3x3.
    EXPECT_TRUE(draft.wouldResizeDropContent(3, 3));
    // Reduire uniquement la largeur : la sortie (0,4) et la liaison (0,0)/(4,0) sont concernees.
    EXPECT_TRUE(draft.wouldResizeDropContent(2, 5));
    // Agrandir ne perd jamais rien.
    EXPECT_FALSE(draft.wouldResizeDropContent(10, 10));
    // Memes dimensions : rien ne bouge.
    EXPECT_FALSE(draft.wouldResizeDropContent(5, 5));
}

/**
 * @brief wouldResizeDropContent est faux sur un brouillon vierge, quelle que soit la taille visee.
 * \castest{<b>wouldResizeDropContent est faux sur un brouillon vierge, quelle que soit la taille
 * visee.</b><br/>
 * \tcat Unitaire · Level Draft<br/>
 * \tcrit Mineur<br/>
 * \tetapes 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et
 * verifier les assertions.<br/>
 * \tattendu wouldResizeDropContent est faux sur un brouillon vierge, quelle que soit la taille
 * visee.
 * }
 */
TEST(LevelDraftTest, WouldResizeDropContentFauxSurBrouillonVierge) {
    const LevelDraft draft = LevelDraft::empty("N", 5, 5);
    EXPECT_FALSE(draft.wouldResizeDropContent(1, 1));
}

/**
 * @brief L'annulation d'un redimensionnement restitue les dimensions et le contenu précédents.
 * \castest{<b>L'annulation d'un redimensionnement restitue les dimensions et le contenu
 * précédents.</b><br/>
 * \tcat Unitaire · Level Draft<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et
 * verifier les assertions.<br/>
 * \tattendu L'annulation d'un redimensionnement restitue les dimensions et le contenu
 * précédents.
 * }
 */
TEST(LevelDraftTest, UndoApresRedimensionnementRestitueLesDimensions) {
    LevelDraft draft = LevelDraft::empty("N", 5, 5);
    draft.setEntry(4, 4);

    draft.resize(2, 2);
    ASSERT_FALSE(draft.entry().has_value());

    draft.undo();

    EXPECT_EQ(draft.tileMap().width(), 5);
    EXPECT_EQ(draft.tileMap().height(), 5);
    ASSERT_TRUE(draft.entry().has_value());
    EXPECT_EQ(*draft.entry(), (GridPosition{4, 4}));
}

/**
 * @brief `fromLevel` restitue les **plans picturaux** et le drapeau de parallaxe d'un niveau déjà
 * chargé (`EX-DEC-040`, `EX-DEC-043`).
 *
 * Défaut réel constaté à l'essai : le brouillon ouvrait un niveau habillé sans ses plans, et
 * l'enregistrer effaçait tout l'habillage — sans le moindre message, puisqu'un champ simplement
 * absent ne fait rien échouer. C'est la classe de défaut qu'aucun test de comportement n'attrape :
 * seul un test de **restitution champ par champ** la voit.
 * \castest{<b>fromLevel restitue les plans picturaux et le drapeau de parallaxe.</b><br/>
 * \tcat Unitaire · Level Draft<br/>
 * \tcrit Critique<br/>
 * \tetapes 1. Charger un niveau declarant deux plans et la parallaxe desactivee.<br/>2. En
 * construire un brouillon par fromLevel.<br/>
 * \tattendu Les deux plans sont restitues dans l'ordre, avec leurs reglages, et le drapeau de
 * parallaxe est conserve.
 * }
 */
TEST(LevelDraftTest, FromLevelRestitueLesPlansEtLaParallaxe) {
    const core::LevelLoadResult loaded = core::LevelLoader::loadFromString(R"({
      "width": 5, "height": 5,
      "parallax": false,
      "planes": [
        { "file": "fond.png", "pixelsPerUnit": 8, "parallaxX": 0.5, "opacity": 0.75 },
        { "file": "devant.png", "parallaxX": 1.2, "depth": "front" }
      ],
      "tiles": [
        { "x": 0, "y": 0, "type": "entry" },
        { "x": 4, "y": 4, "type": "exit" }
      ]
    })");
    ASSERT_TRUE(loaded.ok()) << loaded.error;

    const LevelDraft draft = LevelDraft::fromLevel(*loaded.level);

    ASSERT_EQ(draft.planes().size(), 2u);
    EXPECT_EQ(draft.planes()[0].fileName, "fond.png");
    EXPECT_EQ(draft.planes()[0].pixelsPerUnit, 8);
    EXPECT_FLOAT_EQ(draft.planes()[0].parallaxX, 0.5F);
    EXPECT_FLOAT_EQ(draft.planes()[0].opacity, 0.75F);
    EXPECT_EQ(draft.planes()[0].depth, core::PlaneDepth::Behind);
    // L'ORDRE compte : il porte la superposition. Un brouillon qui les restituerait triés, ou
    // inversés, changerait l'apparence du niveau sans que rien ne le signale.
    EXPECT_EQ(draft.planes()[1].fileName, "devant.png");
    EXPECT_EQ(draft.planes()[1].depth, core::PlaneDepth::Front);
    EXPECT_FALSE(draft.parallaxEnabled());
}

/**
 * @brief Ouvrir un niveau habillé puis le réécrire **sans y toucher** le laisse identique.
 *
 * C'est la formulation qui compte pour la perte d'habillage : le brouillon est le seul chemin par
 * lequel un niveau livré est réécrit, et un aller-retour qui perd un champ perd le travail de son
 * auteur en silence.
 * \castest{<b>Un aller-retour niveau -> brouillon -> niveau conserve les plans.</b><br/>
 * \tcat Unitaire · Level Draft<br/>
 * \tcrit Critique<br/>
 * \tetapes 1. Charger un niveau declarant deux plans.<br/>2. En faire un brouillon, puis le
 * reconvertir en niveau sans aucune mutation.<br/>
 * \tattendu Le niveau reconverti declare les memes plans, dans le meme ordre.
 * }
 */
TEST(LevelDraftTest, AllerRetourBrouillonConserveLesPlans) {
    const core::LevelLoadResult loaded = core::LevelLoader::loadFromString(R"({
      "width": 5, "height": 5,
      "planes": [
        { "file": "fond.png", "pixelsPerUnit": 4 },
        { "file": "devant.png", "depth": "front" }
      ],
      "tiles": [
        { "x": 0, "y": 0, "type": "entry" },
        { "x": 4, "y": 4, "type": "exit" }
      ]
    })");
    ASSERT_TRUE(loaded.ok()) << loaded.error;

    const LevelDraft draft = LevelDraft::fromLevel(*loaded.level);
    const core::LevelLoadResult rewritten = draft.toLevel();
    ASSERT_TRUE(rewritten.ok()) << rewritten.error;

    ASSERT_EQ(rewritten.level->planes().size(), 2u);
    EXPECT_EQ(rewritten.level->planes()[0].fileName, "fond.png");
    EXPECT_EQ(rewritten.level->planes()[0].pixelsPerUnit, 4);
    EXPECT_EQ(rewritten.level->planes()[1].fileName, "devant.png");
    EXPECT_EQ(rewritten.level->planes()[1].depth, core::PlaneDepth::Front);
}
