# TACHE-02 — Opérations fichiers (créer/renommer/dupliquer/supprimer) + couche testable {#lot-36-tache-02-operations-fichiers}

**Lot :** [LOT-36](epic.md) · **Emplacement :** `Source/Editor` · **Statut :** non commencé

## Contexte
Le panneau « Niveaux » (TACHE-01) devient **actionnable** : créer, renommer, dupliquer, supprimer un
niveau, avec validation de nom et confirmations. La logique (nom valide, nom de duplication unique,
résolution de collision, effets fichiers) est isolée dans une **couche testable** ; Qt ne fait que la
déclencher (menu contextuel, boutons) et afficher le résultat.

## Travail à réaliser
- **Couche « opérations de niveaux »** (`Source/Editor/LevelFileOperations.{h,cpp}`), sans Qt,
  prenant un dossier `Levels` et s'appuyant sur `std::filesystem` :
  - `create(name, width, height)` : valide le nom (`hmi::LevelNameValidation`), refuse une collision,
    écrit un niveau vide via `core::LevelWriter` (brouillon `LevelDraft::empty` → `toLevel`).
  - `rename(oldPath, newName)` : valide, gère la collision, renomme le fichier (et met à jour le champ
    `name` interne au besoin).
  - `duplicate(path)` : produit un **nom unique** (`nom (copie)`, `nom (copie 2)`, …) et copie.
  - `remove(path)` : supprime (option : corbeille applicative / dossier `.trash` pour annulation).
  - Chaque opération renvoie un **résultat** (succès / message d'erreur), jamais d'exception vers l'UI.
- **Intégration Qt** dans le panneau : menu contextuel + boutons (Nouveau, Renommer, Dupliquer,
  Supprimer) ; **dialogues** de saisie de nom (validation en direct) et **confirmation** de suppression
  (`QMessageBox`). Rafraîchir la liste après opération et sélectionner l'élément résultant.

## Fichiers impactés
- `Source/Editor/LevelFileOperations.{h,cpp}` (nouveau, testable, sans Qt).
- `Source/Editor/LevelBrowserPanel.{h,cpp}` (actions, dialogues, câblage).
- `Source/Test/Unit/Editor/test_level_file_operations.cpp` (nouveau).

## Tests (obligatoires)
- **Unitaires sans Qt/GPU** (dossier temporaire) :
  - Création : nom valide → fichier écrit ; nom invalide → refus, message ; collision → refus.
  - Renommage : succès ; collision → refus ; nom invalide → refus.
  - Duplication : nom **unique** généré (y compris duplication répétée : `(copie)`, `(copie 2)`),
    contenu identique.
  - Suppression : fichier retiré ; (si corbeille) restaurable.
- Réutiliser les validateurs existants (`test_level_name_validation.cpp`) — pas de règle dupliquée.
- **Vérification manuelle** : les quatre opérations depuis le panneau, avec dialogues/confirmations.

## Points d'attention
- **Validation et format = autorités existantes** : `LevelNameValidation`, `LevelWriter`,
  `LevelLoader` ; ne pas réimplémenter la validation de nom ni la sérialisation.
- **Robustesse fichiers** : gérer fichier verrouillé / droits / chemin inexistant sans crash
  (résultat d'erreur lisible).
- **Cohérence nom interne / nom de fichier** : décider si le renommage change aussi le champ `name`
  du JSON (recommandé) — le documenter.

## Définition de fait (DoD)
- Créer/renommer/dupliquer/supprimer opérationnels depuis le panneau, validés et confirmés ; couche
  d'opérations **couverte par des tests** ; `/W4 /WX` propre ; vérification manuelle OK.

## Exigences
`EX-IHM-021` (opérations fichiers avec validation et confirmation) ; réutilise `EX-EDIT-006`/
`EX-EDIT-007` (écriture/validation), `EX-NFR-010` (logique testable), `EX-NFR-040` (robustesse).
