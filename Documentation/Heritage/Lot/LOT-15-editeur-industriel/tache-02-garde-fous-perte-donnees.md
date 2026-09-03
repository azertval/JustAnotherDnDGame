# TACHE-02 — Garde-fous : redimensionnement destructeur, quitter sans enregistrer {#lot-15-tache-02-garde-fous-perte-donnees}

**Lot :** [LOT-15](epic.md) · **Emplacement :** `Core/Levels`, `HMI/Interface` · **Statut :** à faire

## Contexte
`LevelDraft::resize` tronque déjà silencieusement le contenu hors bornes (entrée, sortie, liaisons)
— un comportement documenté comme volontaire (« l'avertissement de perte revient à l'appelant
`HMI` ») mais jamais câblé côté `EditorScreen` en LOT-14 TACHE-03. De même, `Échap` quitte
l'éditeur sans avertir d'éventuelles modifications non enregistrées. Cette tâche pose un mécanisme
de confirmation **générique**, réutilisé par les deux cas (et par TACHE-03 pour l'écrasement de
fichier).

## Travail à réaliser
- **Requête pure** `LevelDraft::wouldResizeDropContent(int width, int height) const` (`Core`,
  n'altère rien) : vrai si les nouvelles bornes excluraient l'entrée, la sortie, ou une des deux
  extrémités d'une liaison de mécanisme.
- **État de confirmation générique** dans `EditorScreen` : un type `PendingConfirmation` (message +
  action à exécuter si confirmée) remplace les futurs cas ponctuels ; `Entrée` confirme et exécute
  l'action, `Échap` annule sans effet. Affiché via `renderStatus` (extension du message de statut
  existant, distingue visuellement une confirmation d'une simple erreur).
- **Redimensionnement** : avant d'appeler `_draft.resize(...)`, si
  `wouldResizeDropContent(...)` est vrai, poser une `PendingConfirmation` au lieu d'appliquer
  directement ; sinon, appliquer comme aujourd'hui (aucun changement pour un redimensionnement
  anodin).
- **Modifications non enregistrées** : drapeau `_dirty` sur `EditorScreen`, mis à `true` par toute
  mutation du brouillon (peinture, liaison, redimensionnement, annuler/refaire, collage — TACHE-05)
  et remis à `false` après un `saveDraft()` réussi. `Échap` (hors essai immédiat) avec `_dirty ==
  true` pose une `PendingConfirmation` (« Quitter sans enregistrer ? ») avant la transition vers le
  menu ; sans modification en attente, comportement inchangé.

## Fichiers impactés
- `Source/Core/Levels/LevelDraft.h`/`.cpp` (`wouldResizeDropContent`).
- `Source/HMI/Interface/EditorScreen.h`/`.cpp` (`PendingConfirmation`, `_dirty`, câblage
  redimensionnement/`Échap`).
- `Source/Test/Unit/Core/Levels/test_level_draft.cpp`, tests système.

## Tests (obligatoires)
- `wouldResizeDropContent` : vrai quand la nouvelle taille exclurait l'entrée/la sortie/une
  extrémité de liaison ; faux pour un agrandissement ou une réduction qui ne perd rien.
- Redimensionnement destructeur : la grille n'est **pas modifiée** tant que la confirmation n'est
  pas acceptée (`Entrée`) ; `Échap` laisse la grille strictement inchangée.
- `_dirty` passe à `true` après une mutation, à `false` après un enregistrement réussi ; `Échap`
  avec `_dirty == true` ne quitte pas sans confirmation, `Échap` avec `_dirty == false` quitte
  directement (comportement LOT-14 inchangé dans ce cas).

## Points d'attention
- `wouldResizeDropContent` ne doit dupliquer **aucune** règle déjà portée par `resize`/`toLevel` —
  elle inspecte seulement les positions actuelles contre les nouvelles bornes, sans réimplémenter
  la validation de `LevelLoader`.
- Le mécanisme `PendingConfirmation` est conçu pour être réutilisé tel quel par l'avertissement
  d'écrasement de TACHE-03 : ne pas le spécialiser au seul cas du redimensionnement.

## Définition de fait (DoD)
- Les deux garde-fous opérationnels et testés (`ctest` vert) ; build `/W4 /WX` ; Doxygen à jour.

## Exigences
`EX-EDIT-012`.
