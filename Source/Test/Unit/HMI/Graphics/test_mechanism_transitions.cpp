// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: PolyForm-Noncommercial-1.0.0

/**
 * @file test_mechanism_transitions.cpp
 * @brief Tests unitaires des transitions et du repli sur clip manquant (LOT-47 TACHE-02).
 */

#include <set>
#include <string>

#include <gtest/gtest.h>

#include "Core/Ecs/AnimationClip.h"
#include "HMI/Graphics/AnimationCatalog.h"
#include "HMI/Graphics/MechanismVisuals.h"

namespace hmi {
namespace {

// Description d'un asset de porte avec ses quatre clips conventionnels : "closed"/"open" bouclent
// (une image), "opening"/"closing" sont des clips joues une fois qui enchainent sur l'etat cible --
// exactement le contrat attendu par un asset de porte (LOT-47 TACHE-01/02).
AnimationDescription doorDescription() {
    AnimationDescription description;
    description.frameWidth = 16;
    description.frameHeight = 16;

    core::AnimationClip closed;
    closed.name = MECHANISM_CLIP_DOOR_CLOSED;
    closed.frames = {0};
    description.clips.addClip(closed);

    core::AnimationClip opening;
    opening.name = MECHANISM_CLIP_DOOR_OPENING;
    opening.frames = {1, 2};
    opening.frameDuration = 0.1f;
    opening.endMode = core::ClipEndMode::OneShot;
    opening.nextClip = MECHANISM_CLIP_DOOR_OPEN;
    description.clips.addClip(opening);

    core::AnimationClip open;
    open.name = MECHANISM_CLIP_DOOR_OPEN;
    open.frames = {3};
    description.clips.addClip(open);

    core::AnimationClip closing;
    closing.name = MECHANISM_CLIP_DOOR_CLOSING;
    closing.frames = {4, 5};
    closing.frameDuration = 0.1f;
    closing.endMode = core::ClipEndMode::OneShot;
    closing.nextClip = MECHANISM_CLIP_DOOR_CLOSED;
    description.clips.addClip(closing);

    return description;
}

// Le resultat porte la region a afficher : non pertinent pour ces tests, qui verifient l'etat de
// l'horloge (clip/image) et le journal d'avertissements. [[nodiscard]] oblige a le nommer.
void step(MechanismVisualState& state, const AnimationDescription& description, core::TileType type,
          bool active, const std::string& assetKey, float fixedDelta,
          std::set<std::string>& warned) {
    const core::AtlasRegion region =
        advanceMechanismVisual(state, description, type, active, assetKey, fixedDelta, warned);
    (void)region;
}

}  // namespace

/**
 * @brief Le tout premier calcul rejoint directement l'état cible, **sans** jouer de transition :
 * une porte déjà ouverte au chargement du niveau doit apparaître ouverte, pas s'ouvrir sous les
 * yeux du joueur comme si un interrupteur venait d'être actionné.
 * \castest{<b>Le premier calcul rejoint l'état cible sans jouer de transition.</b><br/>
 * \tcat Unitaire · Transitions de mécanismes<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et
 * verifier les assertions.<br/>
 * }
 */
TEST(MechanismTransitionsTest, PremierCalculRejointEtatCibleSansTransition) {
    MechanismVisualState state;
    const AnimationDescription description = doorDescription();
    std::set<std::string> warned;

    // Premier appel pour une instance (etat initial d'un niveau/rechargement) : pas de transition,
    // meme si l'etat logique demarre "actif" (porte ouverte des le chargement, cas legitime).
    step(state, description, core::TileType::Door, true, "door.png", 0.0f, warned);

    EXPECT_EQ(state.animation.clips->clipAt(state.animation.clipIndex).name,
              MECHANISM_CLIP_DOOR_OPEN);
}

/**
 * @brief Un changement d'état déclenche la transition **une seule fois** : les pas suivants la
 * font progresser jusqu'à l'état cible au lieu de la relancer depuis sa première image. Un
 * redéclenchement par pas figerait l'ouverture sur son premier quart, indéfiniment.
 * \castest{<b>Un changement d'état déclenche la transition une seule fois, puis la laisse
 * aboutir.</b><br/>
 * \tcat Unitaire · Transitions de mécanismes<br/>
 * \tcrit Critique<br/>
 * \tetapes 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et
 * verifier les assertions.<br/>
 * }
 */
TEST(MechanismTransitionsTest, ChangementDetatDeclencheLaTransitionUneSeuleFois) {
    MechanismVisualState state;
    const AnimationDescription description = doorDescription();
    std::set<std::string> warned;

    step(state, description, core::TileType::Door, false, "door.png", 0.0f, warned);
    ASSERT_EQ(state.animation.clips->clipAt(state.animation.clipIndex).name,
              MECHANISM_CLIP_DOOR_CLOSED);

    // Changement d'etat (fermee -> ouverte) : clip de transition demande.
    step(state, description, core::TileType::Door, true, "door.png", 0.0f, warned);
    EXPECT_EQ(state.animation.clips->clipAt(state.animation.clipIndex).name,
              MECHANISM_CLIP_DOOR_OPENING);

    // Aucun changement : pas de redeclenchement (l'animation continue de progresser, mais ne
    // revient pas au debut du clip d'ouverture a chaque pas).
    for (int stepIndex = 0; stepIndex < 4; ++stepIndex) {
        step(state, description, core::TileType::Door, true, "door.png", 0.05f, warned);
    }
    // Le OneShot "opening" (2 images, 0.1s chacune) a fini d'enchainer sur "open" apres 0.2s.
    EXPECT_EQ(state.animation.clips->clipAt(state.animation.clipIndex).name,
              MECHANISM_CLIP_DOOR_OPEN);
}

/**
 * @brief Un asset qui ne fournit que les deux états, sans clips de transition, bascule directement
 * et **en silence** : n'animer que les états est un choix d'artiste légitime, pas une erreur à
 * signaler.
 * \castest{<b>Un clip de transition absent fait basculer directement, sans avertissement.</b><br/>
 * \tcat Unitaire · Transitions de mécanismes<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et
 * verifier les assertions.<br/>
 * }
 */
TEST(MechanismTransitionsTest, ClipDeTransitionAbsentBasculeDirectementSansAvertissement) {
    // Asset sans clips "opening"/"closing" : seulement les deux etats.
    AnimationDescription description;
    description.frameWidth = 16;
    description.frameHeight = 16;
    core::AnimationClip closed;
    closed.name = MECHANISM_CLIP_DOOR_CLOSED;
    closed.frames = {0};
    description.clips.addClip(closed);
    core::AnimationClip open;
    open.name = MECHANISM_CLIP_DOOR_OPEN;
    open.frames = {1};
    description.clips.addClip(open);

    MechanismVisualState state;
    std::set<std::string> warned;
    step(state, description, core::TileType::Door, false, "door_no_transition.png", 0.0f, warned);
    step(state, description, core::TileType::Door, true, "door_no_transition.png", 0.0f, warned);

    EXPECT_EQ(state.animation.clips->clipAt(state.animation.clipIndex).name,
              MECHANISM_CLIP_DOOR_OPEN);
    EXPECT_TRUE(warned.empty()) << "une transition absente est un cas legitime, silencieux";
}

/**
 * @brief Un clip d'**état** manquant, lui, se replie et se journalise — mais une seule fois par
 * couple (asset, clip manquant), pas par instance : un niveau posant cinquante portes du même
 * asset incomplet produirait sinon cinquante fois le même message.
 * \castest{<b>Un clip d'état absent se replie et n'est journalisé qu'une fois par asset.</b><br/>
 * \tcat Unitaire · Transitions de mécanismes<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et
 * verifier les assertions.<br/>
 * }
 */
TEST(MechanismTransitionsTest, ClipDEtatAbsentReplieEtJournaliseUneSeuleFois) {
    // Asset ne fournissant AUCUN des quatre clips conventionnels de porte.
    AnimationDescription description;
    description.frameWidth = 16;
    description.frameHeight = 16;
    core::AnimationClip onlyClip;
    onlyClip.name = "decorative";
    onlyClip.frames = {0};
    description.clips.addClip(onlyClip);

    MechanismVisualState state;
    std::set<std::string> warned;
    step(state, description, core::TileType::Door, false, "door_incomplete.png", 0.0f, warned);
    EXPECT_EQ(warned.size(), 1u);

    // Un second mecanisme du MEME asset, avec le MEME clip manquant : pas un second message.
    MechanismVisualState otherState;
    step(otherState, description, core::TileType::Door, false, "door_incomplete.png", 0.0f, warned);
    EXPECT_EQ(warned.size(), 1u) << "un seul message par (asset, clip manquant), pas par instance";
}

/**
 * @brief À état inchangé, un appel supplémentaire ne remet pas l'image courante à zéro : c'est
 * l'invariant qui rend le calcul appelable à chaque frame de rendu sans figer l'animation.
 * \castest{<b>À état inchangé, un appel supplémentaire ne réinitialise pas l'image en
 * cours.</b><br/>
 * \tcat Unitaire · Transitions de mécanismes<br/>
 * \tcrit Critique<br/>
 * \tetapes 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et
 * verifier les assertions.<br/>
 * }
 */
TEST(MechanismTransitionsTest, AucunChangementNeReinitialisePasLimageEnCours) {
    MechanismVisualState state;
    const AnimationDescription description = doorDescription();
    std::set<std::string> warned;

    step(state, description, core::TileType::Door, false, "door.png", 0.0f, warned);
    step(state, description, core::TileType::Door, true, "door.png", 0.0f, warned);
    // Avance jusqu'a la seconde image du clip d'ouverture (0.1s/image).
    step(state, description, core::TileType::Door, true, "door.png", 0.06f, warned);
    step(state, description, core::TileType::Door, true, "door.png", 0.06f, warned);
    ASSERT_EQ(state.animation.clips->clipAt(state.animation.clipIndex).name,
              MECHANISM_CLIP_DOOR_OPENING);
    ASSERT_EQ(state.animation.frameIndex, 1);

    // Aucun changement d'etat : un appel supplementaire ne doit PAS remettre l'image a zero.
    step(state, description, core::TileType::Door, true, "door.png", 0.0f, warned);
    EXPECT_EQ(state.animation.frameIndex, 1);
}

/**
 * @brief Seule la porte a des clips de transition ; un interrupteur bascule directement entre ses
 * deux états, silencieusement. Chercher une transition pour tous les mécanismes produirait des
 * avertissements pour des assets pourtant complets.
 * \castest{<b>Un interrupteur bascule directement entre ses deux états, sans
 * avertissement.</b><br/>
 * \tcat Unitaire · Transitions de mécanismes<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et
 * verifier les assertions.<br/>
 * }
 */
TEST(MechanismTransitionsTest, SeuleLaPorteTransitionneLesAutresBasculentDirectement) {
    AnimationDescription description;
    description.frameWidth = 16;
    description.frameHeight = 16;
    core::AnimationClip inactive;
    inactive.name = MECHANISM_CLIP_SWITCH_INACTIVE;
    inactive.frames = {0};
    description.clips.addClip(inactive);
    core::AnimationClip active;
    active.name = MECHANISM_CLIP_SWITCH_ACTIVE;
    active.frames = {1};
    description.clips.addClip(active);

    MechanismVisualState state;
    std::set<std::string> warned;
    step(state, description, core::TileType::Switch, false, "switch.png", 0.0f, warned);
    step(state, description, core::TileType::Switch, true, "switch.png", 0.0f, warned);

    EXPECT_EQ(state.animation.clips->clipAt(state.animation.clipIndex).name,
              MECHANISM_CLIP_SWITCH_ACTIVE);
    EXPECT_TRUE(warned.empty());
}

}  // namespace hmi
