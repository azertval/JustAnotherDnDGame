# TACHE-04 — Historique annuler/refaire {#lot-14-tache-04-undo-redo}

**Lot :** [LOT-14](epic.md) · **Emplacement :** `Core/Levels`, `HMI/Editor` · **Statut :** à faire

## Contexte
Une erreur de peinture ou de liaison ne doit pas obliger à recommencer un niveau depuis le début —
`EX-EDIT-005` demande **annuler/refaire**. Comme le reste de la logique du moteur, l'historique est
porté par `Core` (donc pur et testable), `HMI` ne fait qu'y déclencher `undo()`/`redo()` sur un
raccourci clavier.

## Travail à réaliser
- **Pile de commandes (Core)** : chaque mutation de `core::LevelDraft` (TACHE-01 :
  `paintTile`/`setEntry`/`setExit`/`linkMechanism`/`unlinkMechanism`/`resize`) est enregistrée comme
  une **commande réversible** (état avant/après, ou delta minimal) dans une pile d'annulation portée
  par le `LevelDraft` (ou un objet dédié qui l'enveloppe, ex. `core::LevelEditHistory`).
- **`undo()`/`redo()`** : dépilent/repilent une commande et réappliquent l'état correspondant au
  `LevelDraft` ; une nouvelle mutation après un `undo()` **tronque** la pile de refaire (convention
  standard d'un historique linéaire).
- **Raccourcis clavier (HMI)** : `Ctrl+Z` déclenche `undo()`, `Ctrl+Y` (ou `Ctrl+Maj+Z`) déclenche
  `redo()`, lus depuis l'`InputState` de l'écran éditeur (TACHE-02).

## Fichiers impactés
- `Source/Core/Levels/LevelDraft.h`/`.cpp` (ou nouveau `LevelEditHistory.h`/`.cpp`).
- `Source/HMI/Interface/EditorScreen.h`/`.cpp` (câblage des raccourcis).
- Tests unitaires (`Source/Test/Unit/Core/Levels/`).

## Tests (obligatoires)
- Une séquence de N mutations suivie de N `undo()` restitue l'état **initial** exact du `LevelDraft`.
- `undo()` puis `redo()` restitue l'état **muté** exact (pas de perte d'information).
- Une nouvelle mutation après un `undo()` **invalide** la branche de refaire (un `redo()` ultérieur
  est sans effet ou refusé proprement).
- `undo()`/`redo()` sur une pile **vide** est sans effet (pas de plantage, pas d'état incohérent).

## Points d'attention
- Éviter une complexité disproportionnée : un historique de **snapshots complets** du `LevelDraft`
  (probablement petit — une grille de niveau, pas un monde ouvert) est acceptable et plus simple à
  garantir correct qu'un delta fin par type de mutation ; à arbitrer selon la taille réelle des
  niveaux du projet (typiquement ≤ 14×8, cf. niveaux livrés).
- Logique **pure**, testable sans GPU (`EX-NFR-010`), comme `LevelDraft` lui-même.

## Définition de fait (DoD)
- `undo`/`redo` fonctionnels et testés (`ctest` vert) sur toutes les mutations de `LevelDraft`,
  raccourcis clavier opérationnels dans l'éditeur ; build `/W4 /WX` ; Doxygen à jour.

## Exigences
`EX-EDIT-005`, `EX-NFR-010`, `EX-NFR-002`.
