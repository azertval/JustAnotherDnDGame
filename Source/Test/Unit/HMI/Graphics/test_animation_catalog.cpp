// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: PolyForm-Noncommercial-1.0.0

/**
 * @file test_animation_catalog.cpp
 * @brief Tests unitaires du format `nom-asset.anim.json` (`hmi::AnimationCatalog`, LOT-46
 *        TACHE-03) : lecture, validation, traduction en région de texture. Sans GPU ni Qt.
 */

#include <gtest/gtest.h>

#include "HMI/Graphics/AnimationCatalog.h"

namespace {
constexpr const char* VALID_JSON = R"({
  "version": 1,
  "frameWidth": 16,
  "frameHeight": 16,
  "clips": {
    "closed":  { "frames": [0], "loop": true },
    "opening": { "frames": [1, 2, 3, 4], "frameDuration": 0.06, "loop": false, "next": "open" },
    "open":    { "frames": [5], "loop": true }
  }
})";
}  // namespace

/**
 * @brief Un descripteur valide se relit intégralement : dimensions d'image, clips multiples, durée
 * **par défaut** quand elle est omise, mode joué-une-fois et clip suivant résolu dans le même jeu.
 * C'est le contrat complet du format, vérifié en une fois sur un cas nominal.
 * \castest{<b>Un descripteur valide se relit intégralement, durée par défaut et clip suivant
 * compris.</b><br/>
 * \tcat Unitaire · Catalogue d'animations<br/>
 * \tcrit Critique<br/>
 * \tetapes 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et
 * verifier les assertions.<br/>
 * }
 */
TEST(AnimationCatalogTest, RoundTripClipsMultiplesDureeParDefautEtOneShot) {
    const hmi::AnimationDescriptionResult result =
        hmi::AnimationCatalog::loadFromString(VALID_JSON);
    ASSERT_TRUE(result.ok()) << result.error;

    const hmi::AnimationDescription& description = *result.description;
    EXPECT_EQ(description.frameWidth, 16);
    EXPECT_EQ(description.frameHeight, 16);
    ASSERT_EQ(description.clips.clipCount(), 3);

    const core::AnimationClip& closed =
        description.clips.clipAt(description.clips.indexOf("closed"));
    EXPECT_EQ(closed.frames, (std::vector<int>{0}));
    EXPECT_EQ(closed.endMode, core::ClipEndMode::Loop);
    // "frameDuration" absent : durée par défaut documentée (AnimationCatalog::
    // DEFAULT_FRAME_DURATION_SECONDS).
    EXPECT_FLOAT_EQ(closed.frameDuration, hmi::AnimationCatalog::DEFAULT_FRAME_DURATION_SECONDS);

    const core::AnimationClip& opening =
        description.clips.clipAt(description.clips.indexOf("opening"));
    EXPECT_EQ(opening.frames, (std::vector<int>{1, 2, 3, 4}));
    EXPECT_FLOAT_EQ(opening.frameDuration, 0.06f);
    EXPECT_EQ(opening.endMode, core::ClipEndMode::OneShot);
    EXPECT_EQ(opening.nextClip, "open");
    EXPECT_GE(description.clips.indexOf(opening.nextClip), 0);
}

/**
 * @brief Un JSON illisible donne une erreur d'analyse **exploitable**, avec un message non vide :
 * les descripteurs sont écrits à la main, une erreur muette laisserait l'artiste sans piste.
 * \castest{<b>Un JSON invalide donne une erreur d'analyse avec un message exploitable.</b><br/>
 * \tcat Unitaire · Catalogue d'animations<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et
 * verifier les assertions.<br/>
 * }
 */
TEST(AnimationCatalogTest, JsonInvalideEstUneErreurExploitable) {
    const hmi::AnimationDescriptionResult result =
        hmi::AnimationCatalog::loadFromString("{ pas du json");
    EXPECT_FALSE(result.ok());
    EXPECT_EQ(result.errorCode, hmi::AnimationCatalogError::ParseError);
    EXPECT_FALSE(result.error.empty());
}

/**
 * @brief Une version de format inconnue est refusée explicitement plutôt que lue au mieux : mieux
 * vaut un refus net qu'une animation silencieusement fausse issue d'un format futur.
 * \castest{<b>Une version de format inconnue est refusée.</b><br/>
 * \tcat Unitaire · Catalogue d'animations<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et
 * verifier les assertions.<br/>
 * }
 */
TEST(AnimationCatalogTest, VersionInconnueEstRefusee) {
    constexpr const char* json = R"({
      "version": 99,
      "frameWidth": 16, "frameHeight": 16,
      "clips": { "idle": { "frames": [0] } }
    })";
    const hmi::AnimationDescriptionResult result = hmi::AnimationCatalog::loadFromString(json);
    EXPECT_FALSE(result.ok());
    EXPECT_EQ(result.errorCode, hmi::AnimationCatalogError::UnsupportedVersion);
}

/**
 * @brief Un clip suivant qui ne désigne aucun clip du descripteur est refusé au chargement : cette
 * incohérence ne se manifesterait sinon qu'à la fin de la transition, en pleine partie.
 * \castest{<b>Un clip suivant inexistant est refusé au chargement.</b><br/>
 * \tcat Unitaire · Catalogue d'animations<br/>
 * \tcrit Critique<br/>
 * \tetapes 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et
 * verifier les assertions.<br/>
 * }
 */
TEST(AnimationCatalogTest, ClipSuivantInexistantEstRefuse) {
    constexpr const char* json = R"({
      "version": 1,
      "frameWidth": 16, "frameHeight": 16,
      "clips": {
        "opening": { "frames": [0, 1], "loop": false, "next": "inconnu" }
      }
    })";
    const hmi::AnimationDescriptionResult result = hmi::AnimationCatalog::loadFromString(json);
    EXPECT_FALSE(result.ok());
    EXPECT_EQ(result.errorCode, hmi::AnimationCatalogError::MalformedStructure);
}

/**
 * @brief Un indice d'image négatif est refusé : il indexerait la spritesheet hors de ses bornes.
 * \castest{<b>Un indice d'image négatif est refusé.</b><br/>
 * \tcat Unitaire · Catalogue d'animations<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et
 * verifier les assertions.<br/>
 * }
 */
TEST(AnimationCatalogTest, IndiceDImageNegatifEstRefuse) {
    constexpr const char* json = R"({
      "version": 1,
      "frameWidth": 16, "frameHeight": 16,
      "clips": { "idle": { "frames": [-1] } }
    })";
    const hmi::AnimationDescriptionResult result = hmi::AnimationCatalog::loadFromString(json);
    EXPECT_FALSE(result.ok());
    EXPECT_EQ(result.errorCode, hmi::AnimationCatalogError::MalformedStructure);
}

/**
 * @brief Un clip sans aucune image est refusé : il n'aurait rien à afficher, et son absence
 * d'images ne se verrait qu'au moment de le jouer.
 * \castest{<b>Un clip sans images est refusé.</b><br/>
 * \tcat Unitaire · Catalogue d'animations<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et
 * verifier les assertions.<br/>
 * }
 */
TEST(AnimationCatalogTest, ClipSansFramesEstRefuse) {
    constexpr const char* json = R"({
      "version": 1,
      "frameWidth": 16, "frameHeight": 16,
      "clips": { "idle": { "loop": true } }
    })";
    const hmi::AnimationDescriptionResult result = hmi::AnimationCatalog::loadFromString(json);
    EXPECT_FALSE(result.ok());
    EXPECT_EQ(result.errorCode, hmi::AnimationCatalogError::MalformedStructure);
}

/**
 * @brief Un fichier absent donne un code d'erreur dédié, **sans exception** : la plupart des assets
 * n'ont pas de descripteur, l'absence est le cas courant et non une anomalie.
 * \castest{<b>Un fichier absent donne un code dédié, sans exception.</b><br/>
 * \tcat Unitaire · Catalogue d'animations<br/>
 * \tcrit Critique<br/>
 * \tetapes 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et
 * verifier les assertions.<br/>
 * }
 */
TEST(AnimationCatalogTest, FichierAbsentEstFileNotFoundSansException) {
    const hmi::AnimationDescriptionResult result =
        hmi::AnimationCatalog::loadFromFile("Z:/chemin/totalement/inexistant.anim.json");
    EXPECT_FALSE(result.ok());
    EXPECT_EQ(result.errorCode, hmi::AnimationCatalogError::FileNotFound);
}

/**
 * @brief Le nom du descripteur se déduit de celui de l'image en remplaçant l'extension, chemin de
 * sous-dossier **conservé** : perdre le préfixe ferait chercher le descripteur au mauvais endroit,
 * un piège déjà rencontré sur les clés du cache de textures.
 * \castest{<b>Le nom du descripteur remplace l'extension en conservant le chemin.</b><br/>
 * \tcat Unitaire · Catalogue d'animations<br/>
 * \tcrit Critique<br/>
 * \tetapes 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et
 * verifier les assertions.<br/>
 * }
 */
TEST(AnimationCatalogTest, DescriptorFileNameRemplaceLExtension) {
    EXPECT_EQ(hmi::AnimationCatalog::descriptorFileName("water.png"), "water.anim.json");
    EXPECT_EQ(hmi::AnimationCatalog::descriptorFileName("Skins/lava.png"), "Skins/lava.anim.json");
}

/**
 * @brief Un descripteur cohérent avec les dimensions réelles du PNG est validé : six images de 16
 * px sur un rang de 16 px de haut.
 * \castest{<b>Un descripteur cohérent avec les dimensions du PNG est validé.</b><br/>
 * \tcat Unitaire · Catalogue d'animations<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et
 * verifier les assertions.<br/>
 * }
 */
TEST(AnimationCatalogTest, CoherenceAvecLePngValideeSurSpritesheetAUnRang) {
    const hmi::AnimationDescriptionResult result =
        hmi::AnimationCatalog::loadFromString(VALID_JSON);
    ASSERT_TRUE(result.ok());

    // 6 images de 16 px sur un seul rang de 16 px de haut : coherent.
    const hmi::AssetValidation valid =
        hmi::AnimationCatalog::validateAgainstTexture(*result.description, "door.png", 96, 16);
    EXPECT_TRUE(valid.valid) << valid.message;
}

/**
 * @brief Une hauteur de PNG différente de la hauteur d'image (plusieurs rangs, non supportés) ou
 * une largeur qui n'est pas un multiple de la largeur d'image sont refusées, avec un message : ce
 * sont les deux façons de se tromper en découpant une planche.
 * \castest{<b>Des dimensions de PNG incohérentes avec l'image sont refusées.</b><br/>
 * \tcat Unitaire · Catalogue d'animations<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et
 * verifier les assertions.<br/>
 * }
 */
TEST(AnimationCatalogTest, TailleDImageIncoherenteAvecLePngEstRefusee) {
    const hmi::AnimationDescriptionResult result =
        hmi::AnimationCatalog::loadFromString(VALID_JSON);
    ASSERT_TRUE(result.ok());

    // Hauteur du PNG differente de frameHeight : plus d'un rang, non supporte (TACHE-03).
    const hmi::AssetValidation wrongHeight =
        hmi::AnimationCatalog::validateAgainstTexture(*result.description, "door.png", 96, 32);
    EXPECT_FALSE(wrongHeight.valid);
    EXPECT_FALSE(wrongHeight.message.empty());

    // Largeur qui n'est pas un multiple de frameWidth.
    const hmi::AssetValidation wrongWidth =
        hmi::AnimationCatalog::validateAgainstTexture(*result.description, "door.png", 90, 16);
    EXPECT_FALSE(wrongWidth.valid);
}

/**
 * @brief Un indice d'image au-delà de ce que contient réellement la spritesheet est refusé, même
 * quand la largeur est un multiple valide : la validation confronte les clips au fichier, pas
 * seulement le fichier à lui-même.
 * \castest{<b>Un indice d'image hors des bornes réelles de la spritesheet est refusé.</b><br/>
 * \tcat Unitaire · Catalogue d'animations<br/>
 * \tcrit Critique<br/>
 * \tetapes 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et
 * verifier les assertions.<br/>
 * }
 */
TEST(AnimationCatalogTest, IndiceDImageHorsBornesDeLaSpritesheetReelleEstRefuse) {
    const hmi::AnimationDescriptionResult result =
        hmi::AnimationCatalog::loadFromString(VALID_JSON);
    ASSERT_TRUE(result.ok());

    // Le clip "open" reference l'image 5 : une spritesheet de 5 images (80 px / 16) est trop
    // courte, bien que sa largeur soit un multiple valide de frameWidth.
    const hmi::AssetValidation tooShort =
        hmi::AnimationCatalog::validateAgainstTexture(*result.description, "door.png", 80, 16);
    EXPECT_FALSE(tooShort.valid);
    EXPECT_FALSE(tooShort.message.empty());
}

/**
 * @brief La région d'une image se déduit de son indice par simple décalage horizontal, l'ordonnée
 * ne variant jamais : c'est la convention de planche à un seul rang, qui rend le découpage
 * prévisible pour l'artiste.
 * \castest{<b>La région d'une image se déduit de son indice par décalage horizontal, ordonnée
 * constante.</b><br/>
 * \tcat Unitaire · Catalogue d'animations<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et
 * verifier les assertions.<br/>
 * }
 */
TEST(AnimationCatalogTest, FrameRegionPremiereEtDerniereImageSpritesheetAUnRang) {
    hmi::AnimationDescription description;
    description.frameWidth = 16;
    description.frameHeight = 16;

    const core::AtlasRegion first = hmi::AnimationCatalog::frameRegion(description, 0);
    EXPECT_EQ(first.x, 0);
    EXPECT_EQ(first.y, 0);
    EXPECT_EQ(first.width, 16);
    EXPECT_EQ(first.height, 16);

    const core::AtlasRegion last = hmi::AnimationCatalog::frameRegion(description, 5);
    EXPECT_EQ(last.x, 5 * 16);
    EXPECT_EQ(last.y, 0);  // un seul rang : y ne varie jamais.
    EXPECT_EQ(last.width, 16);
    EXPECT_EQ(last.height, 16);
}
