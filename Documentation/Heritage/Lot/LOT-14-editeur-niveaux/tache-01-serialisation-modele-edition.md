# TACHE-01 — Sérialisation JSON + modèle d'édition mutable {#lot-14-tache-01-serialisation-modele-edition}

**Lot :** [LOT-14](epic.md) · **Emplacement :** `Core/Levels` · **Statut :** à faire

## Contexte
`core::LevelLoader` sait **lire** un niveau JSON, mais rien ne sait l'**écrire** — brique manquante
avant tout enregistrement depuis l'éditeur. `core::Level`/`core::TileMap` sont par ailleurs conçus
comme **immuables** une fois construits (LOT-07) : l'éditeur a besoin d'une représentation
**mutable** distincte, convertie en `Level` **validé** au moment de l'enregistrement, sans changer
la conception de `Level` lui-même ni dupliquer sa validation (`EX-EDIT-010`).

Avant toute chose, corriger le texte obsolète d'`EX-EDIT-006` dans `editeur-niveaux.md` (« format
hybride ASCII + JSON », jamais implémenté) pour référencer le format **JSON pur** réellement défini
par `EX-LVL-003`.

## Travail à réaliser
- **Spec** : mettre à jour `Documentation/Specification/editeur-niveaux.md`, `EX-EDIT-006` — remplacer
  la mention du format hybride par une référence au format JSON de `EX-LVL-003` (aucun nouvel
  identifiant d'exigence, uniquement le texte).
- **Écrivain JSON (Core)** : une fonction pure `Level → JSON` (ex. `core::LevelWriter::toJson(const
  Level&)` ou `Level::toJson() const`), symétrique au parsing de `LevelLoader` — mêmes clés/format
  (`name`, `width`, `height`, `jumpBudget`/`dashBudget` si non `-1`, `tiles` en liste éparse
  d'objets, liaisons interrupteur↔porte réexprimées par identifiant `id`/`opensWith`).
- **Modèle d'édition mutable** : une nouvelle classe (ex. `core::LevelDraft`, `Core/Levels`)
  portant une grille de tuiles **mutable**, l'entrée/la sortie courantes, les liaisons de
  mécanismes et les budgets — avec des mutateurs élémentaires : `paintTile(colonne, ligne, type)`,
  `setEntry`/`setExit` (déplace l'existant, garantit l'unicité), `linkMechanism`/`unlinkMechanism`,
  `resize(largeur, hauteur)` (préserve le contenu dans les bornes communes).
- **Conversion vers `Level` validé** : `LevelDraft::toLevel()` (ou équivalent) produit un
  `LevelLoadResult` en repassant par la **même validation** que `LevelLoader` (bornes, unicité
  entrée/sortie, liaisons résolues, `EX-LVL-004`) — aucune règle de validation dupliquée.
- **Construction depuis l'existant** : `LevelDraft::fromLevel(const Level&)` et
  `LevelDraft::empty(largeur, hauteur)` (nouveau niveau vierge).

## Fichiers impactés
- `Source/Core/Levels/LevelWriter.h`/`.cpp` (nouveau), `Source/Core/Levels/LevelDraft.h`/`.cpp`
  (nouveau).
- `Documentation/Specification/editeur-niveaux.md` (correction `EX-EDIT-006`).
- Tests unitaires (`Source/Test/Unit/Core/Levels/`).

## Tests (obligatoires)
- **Round-trip** : `Level` (chargé depuis un fichier existant, ex. `demo4.json`) → `toJson` →
  `LevelLoader::loadFromString` → `Level` **identique** (mêmes tuiles, entrée/sortie, mécanismes,
  budgets) — preuve directe d'`EX-EDIT-011`.
- `LevelDraft::empty` produit une grille entièrement `Empty` des dimensions demandées.
- `paintTile`/`setEntry`/`setExit`/`linkMechanism`/`resize` modifient l'état attendu, y compris cas
  limites (redimensionner plus petit tronque, `setEntry` déplace une entrée déjà posée).
- `toLevel()` sur un brouillon **invalide** (sans sortie, liaison non résolue) renvoie une erreur
  **récupérable** (pas d'exception, `EX-NFR-040`) avec un message exploitable ; sur un brouillon
  **valide**, renvoie un `Level` conforme.

## Points d'attention
- **Aucune duplication de validation** : `LevelDraft::toLevel()` doit s'appuyer sur le même chemin
  de validation que `LevelLoader` (factoriser si nécessaire), jamais réimplémenter les règles
  d'`EX-LVL-004` une seconde fois.
- `Level`/`TileMap` restent **immuables** : ce lot n'ajoute pas de mutateurs à ces types existants,
  toute la mutabilité vit dans le nouveau `LevelDraft`.
- Logique **pure**, sans dépendance rendu ni fenêtre — testable sans GPU (`EX-NFR-010`), comme le
  reste de `Core`.

## Définition de fait (DoD)
- Écrivain JSON, `LevelDraft` et conversion validée ajoutés, documentés, **testés** (`ctest` vert) ;
  round-trip prouvé sur au moins un niveau livré ; build `/W4 /WX` ; spec corrigée ; `lint` des
  exigences vert.

## Exigences
`EX-EDIT-006`, `EX-EDIT-007`, `EX-EDIT-010`, `EX-EDIT-011`, `EX-LVL-002`, `EX-LVL-003`, `EX-LVL-004`,
`EX-ARCH-011`, `EX-NFR-010`, `EX-NFR-040`.
