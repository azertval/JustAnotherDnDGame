# TACHE-03 — Documentation et vérification {#lot-26-tache-03-documentation-verification}

**Lot :** [LOT-26](epic.md) · **Emplacement :** `Documentation` · **Statut :** fait

## Contexte
Dernière tâche du lot : aligner la documentation sur le comportement final (physique de suivi,
pas la solidité simple envisagée un temps en cours de route — voir la décision de cadrage de
l'épic).

## Travail à réaliser
- **`Documentation/Specification/gameplay.md`** : nouvelle exigence `EX-GP-006`, ligne de tableau
  pour la variante de plafond.
- **`Documentation/Specification/niveaux.md`** : les quatre nouveaux types JSON listés dans le
  format de niveau.
- **`Source/Elements/Levels/README.md`** : les quatre nouveaux types listés.
- **`Documentation/Lot/lots.md`** : ajoute `@subpage lot-26`.
- **`CHANGELOG.md`** : entrée `[Non publié]` décrivant le lot (comportement final, pas
  l'implémentation intermédiaire solide/visuelle-seule).
- **Doxygen**, **Cahier de test**, **lint des exigences** : mêmes vérifications que les lots
  précédents.

## Fichiers impactés
- `Documentation/Specification/gameplay.md`, `niveaux.md`.
- `Source/Elements/Levels/README.md`.
- `Documentation/Lot/lots.md`.
- `Documentation/CahierTest.md` (régénéré).
- `CHANGELOG.md`.

## Tests (obligatoires)
- `python scripts/lint_exigences.py` retourne un code de sortie `0`.
- `python scripts/generate_cahier_test.py --check` confirme que le Cahier de test est à jour.
- `doxygen Doxyfile` (depuis `Documentation/`) se termine avec un code de sortie `0` et **aucune**
  ligne de sortie.

## Points d'attention
- **La documentation doit refléter le comportement FINAL**, pas l'implémentation intermédiaire
  (tuiles solides à silhouette purement visuelle) rédigée puis abandonnée en cours de lot — les
  passages déjà écrits pour cette version intermédiaire (`gameplay.md`, `niveaux.md`,
  `Elements/Levels/README.md`) ont été réécrits, pas seulement complétés.

## Définition de fait (DoD)
- Documentation cohérente avec le comportement livré ; `epic.md` marqué **terminé**, chaque tâche
  marquée ✅.

## Exigences
Aucune exigence propre — tâche de cohérence documentaire pour l'ensemble du lot.
