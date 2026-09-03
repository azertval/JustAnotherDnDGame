# TACHE-02 — Documentation et vérification {#lot-17-tache-02-documentation-verification}

**Lot :** [LOT-17](epic.md) · **Emplacement :** `Documentation` · **Statut :** fait

## Contexte
Dernière tâche du lot : aligner la documentation sur ce qui a été livré, comme pour chaque lot
précédent.

## Travail à réaliser
- **`Documentation/Guide/guide-rendu.md`** : décrire la région dédiée au personnage (pourquoi elle
  vit dans `TextureAtlas` plutôt que dans une classe séparée façon `SaveIcon`/`FlagIcons` — la
  contrainte une-texture-par-passe de `SpriteRenderer`), et la technique de silhouette par blocs
  rectangulaires.
- **`Source/HMI/Graphics/README.md`** : mettre à jour la ligne décrivant `TextureAtlas` pour
  mentionner la région du personnage.
- **`CHANGELOG.md`** : entrée `[Non publié]` décrivant LOT-17 (`EX-REN-011`, silhouette statique ;
  `EX-REN-012` animation explicitement reportée à un lot séparé).
- **Doxygen** : génération locale complète (`doxygen Doxyfile` depuis `Documentation/`) avant de
  pousser.
- **`scripts/lint_exigences.py`** : doit rester vert (aucune nouvelle exigence déclarée — `EX-REN-011`
  déjà déclarée en LOT-05, seulement référencée à nouveau ici).

## Fichiers impactés
- `Documentation/Guide/guide-rendu.md`.
- `Source/HMI/Graphics/README.md`.
- `CHANGELOG.md`.
- `Documentation/Lot/lots.md` (ajout de la ligne LOT-17).

## Tests (obligatoires)
- `python scripts/lint_exigences.py` retourne un code de sortie `0`.
- `doxygen Doxyfile` (depuis `Documentation/`) se termine avec un code de sortie `0` et **aucune**
  ligne de sortie (`QUIET=YES`, `WARN_AS_ERROR=FAIL_ON_WARNINGS`).

## Points d'attention
- Cette tâche clôt le lot : vérifier que les critères d'acceptation d'`epic.md` sont tous
  effectivement remplis avant de le marquer terminé (statut + cases ✅ du tableau des tâches).

## Définition de fait (DoD)
- Documentation cohérente avec le code livré ; lint des exigences et génération Doxygen locale
  verts ; `CHANGELOG.md` à jour ; `epic.md` marqué **terminé**, chaque tâche marquée ✅.

## Exigences
Aucune exigence propre — tâche de cohérence documentaire pour l'ensemble du lot.
