# TACHE-01 — Validation à la lecture (empreinte de niveau) {#lot-annexe-17-tache-01-validation-lecture-empreinte-niveau}

**Lot :** [LOT-ANNEXE-17](epic.md) · **Emplacement :** `Source/AiSolver/Replay` · **Statut :** fait

## Contexte
Le format de rejeu v1 (`LOT-ANNEXE-07`) enregistre déjà, dans ses métadonnées, le chemin du niveau
d'origine et une empreinte calculée à l'export — mais rien, aujourd'hui, ne recalcule cette
empreinte à la lecture ni ne la compare. Un rejeu chargé sur un niveau modifié depuis
l'entraînement (tuile déplacée, budget de sauts changé) rejouerait une séquence d'entrées devenue
incohérente avec la géométrie courante : au mieux une trajectoire absurde, au pire un blocage ou
une chute hors niveau. Cette tâche ferme ce trou avant que `LOT-ANNEXE-18` ne branche le rejeu sur
le jeu réel.

## Travail à réaliser
- **`aisolver::LevelFingerprint`** (`Source/AiSolver/Replay/LevelFingerprint.h`) : alias
  `using LevelFingerprint = std::uint64_t;`.
- **`aisolver::computeLevelFingerprint`** (même dossier) : `LevelFingerprint
  computeLevelFingerprint(std::string_view levelFileContent);` — hache le contenu **brut** (octets
  UTF-8 du fichier, tel que lu, sans reformatage ni reparsing JSON) via FNV-1a 64 bits (algorithme
  fixe, sans bibliothèque tierce, implémenté à la main). Fonction pure, testable sans accès disque
  (l'appelant lit le fichier, cette fonction ne prend qu'une chaîne).
- **`aisolver::ReplayValidationError`** (`Source/AiSolver/Replay/ReplayValidation.h`) : `enum class`
  à deux valeurs, `LevelFileMissing` et `LevelFingerprintMismatch`.
- **`aisolver::validateReplay`** (même fichier) : `std::optional<ReplayValidationError>
  validateReplay(const ReplayFile& replay, const std::filesystem::path& levelsDir);` — résout le
  chemin du niveau référencé par `replay` sous `levelsDir`, renvoie `LevelFileMissing` s'il
  n'existe pas, sinon lit son contenu, calcule son empreinte via `computeLevelFingerprint` et la
  compare à celle stockée dans `replay` ; renvoie `LevelFingerprintMismatch` en cas de divergence,
  `std::nullopt` si tout correspond (rejeu valide). Ne lève jamais d'exception ; la lecture réelle
  du fichier de rejeu lui-même (désérialisation JSON) reste du ressort du lecteur défini par
  `LOT-ANNEXE-07`, appelé **avant** cette validation par tout point d'entrée (jeu, CLI, script).
- Point d'intégration : tout chargeur de rejeu (`LOT-ANNEXE-18` côté jeu, `LOT-ANNEXE-19` côté CLI)
  appelle `validateReplay` immédiatement après la désérialisation et **avant** toute utilisation de
  la séquence d'entrées — jamais après coup.

## Fichiers impactés
- `Source/AiSolver/Replay/LevelFingerprint.h` (nouveau), `LevelFingerprint.cpp` (nouveau).
- `Source/AiSolver/Replay/ReplayValidation.h` (nouveau), `ReplayValidation.cpp` (nouveau).
- `Source/Test/CMakeLists.txt` (ajout des nouveaux fichiers de test à `UnitTests`).
- Tests : `Source/Test/Unit/AiSolver/Replay/test_level_fingerprint.cpp`,
  `Source/Test/Unit/AiSolver/Replay/test_replay_validation.cpp` (nouveaux).

## Tests (obligatoires)
- **Empreintes identiques pour un contenu identique** : deux appels de `computeLevelFingerprint`
  sur la même chaîne produisent la même valeur (déterminisme de la fonction elle-même).
- **Empreintes différentes pour un contenu différent** : un changement d'un seul caractère du
  fichier de niveau change l'empreinte (test de non-trivialité, pas une preuve d'absence de
  collision).
- **Niveau absent** : `validateReplay` renvoie `LevelFileMissing` si le fichier référencé n'existe
  pas sous `levelsDir`.
- **Empreinte divergente** : `validateReplay` renvoie `LevelFingerprintMismatch` si le contenu du
  fichier de niveau a changé depuis l'export (rejeu construit avec une empreinte factice, fichier
  de niveau réel présent).
- **Rejeu valide** : `validateReplay` renvoie `std::nullopt` quand le fichier de niveau existe et
  correspond exactement à l'empreinte enregistrée.

## Points d'attention
- **Le hachage porte sur les octets bruts du fichier, jamais sur le modèle `core::Level` déjà
  chargé** : deux fichiers textuellement différents (espacement, ordre de champs JSON) mais
  sémantiquement identiques seraient perçus comme un changement — accepté délibérément, la
  simplicité et la vérifiabilité l'emportent sur une notion d'équivalence sémantique dont aucun
  lot n'a besoin.
- **FNV-1a est choisi aussi pour sa simplicité de réimplémentation** : `LOT-ANNEXE-20` doit porter
  ce même calcul en Python pur (garde-fou CI, pas de build C++ dans ce job) — un algorithme
  standard, non paramétré, à convention d'octets stricte (UTF-8, aucune normalisation de fin de
  ligne) réduit fortement le risque de divergence entre les deux implémentations. Documenter cette
  convention d'octets ici, précisément, est ce qui permet à `LOT-ANNEXE-20` de la reproduire sans
  ambiguïté.
- **`validateReplay` ne lit jamais le fichier de rejeu lui-même** : elle prend un `ReplayFile` déjà
  désérialisé — la responsabilité de gérer un fichier de rejeu introuvable ou JSON malformé reste
  entièrement au lecteur de `LOT-ANNEXE-07`, pas dupliquée ici.

## Définition de fait (DoD)
- `computeLevelFingerprint` et `validateReplay` disponibles et testés (`ctest` vert) ; build
  `/W4 /WX` sans avertissement ; Doxygen à jour ; `EX-IA-018` déclarée.

## Notions abordées
Aucune notion d'apprentissage automatique nouvelle : cette tâche est d'ordre logiciel (format de
fichier, outillage, intégration continue). Le vocabulaire employé (épisode, rejeu, politique, agent)
est défini dans @ref guide-annexe-apprentissage-renforcement.

## Exigences
`EX-IA-018` (nouvelle).
