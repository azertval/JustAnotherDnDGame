# TACHE-04 — Documentation et vérification {#lot-24-tache-04-documentation-verification}

**Lot :** [LOT-24](epic.md) · **Emplacement :** `Documentation` · **Statut :** fait

## Contexte
Dernière tâche du lot : aligner la documentation sur ce qui a été livré.

## Travail à réaliser
- **`Documentation/Specification/gameplay.md`** : retire le marqueur « ⚠️ non implémenté » d'
  `EX-GP-005`.
- **`Documentation/Specification/niveaux.md`** : ajoute `blockHalf`/`blockQuarter` à la liste des
  types de tuiles du format JSON.
- **`Documentation/Guide/guide-niveaux.md`** : section « Blocs poussables » (`LOT-21`) complétée
  pour couvrir les tailles réduites, la boîte centrée, et **pourquoi** une routine de collision
  séparée a été nécessaire (limite du modèle de grille pour une occupation partielle de case).
- **`Documentation/Manuel/partager-un-niveau.md`** : nouvelle ligne dans le tableau des actions de
  l'éditeur.
- **`Documentation/Lot/lots.md`** : ajoute `@subpage lot-24`.
- **`CHANGELOG.md`** : entrée `[Non publié]` décrivant LOT-24 (`EX-GP-005`).
- **Doxygen**, **Cahier de test**, **lint des exigences** : mêmes vérifications que les lots
  précédents.

## Fichiers impactés
- `Documentation/Specification/gameplay.md`, `niveaux.md`.
- `Documentation/Guide/guide-niveaux.md`.
- `Documentation/Manuel/partager-un-niveau.md`.
- `Documentation/Lot/lots.md`.
- `Documentation/CahierTest.md` (régénéré).
- `CHANGELOG.md`.

## Tests (obligatoires)
- `python scripts/lint_exigences.py` retourne un code de sortie `0`.
- `python scripts/generate_cahier_test.py --check` confirme que le Cahier de test est à jour.
- `doxygen Doxyfile` (depuis `Documentation/`) se termine avec un code de sortie `0` et **aucune**
  ligne de sortie.

## Points d'attention
- Cette tâche clôt le lot : vérifier que les 4 critères d'acceptation d'`epic.md` sont
  effectivement remplis avant de le marquer terminé.

## Définition de fait (DoD)
- Documentation cohérente avec le code livré ; `epic.md` marqué **terminé**, chaque tâche
  marquée ✅.

## Exigences
Aucune exigence propre — tâche de cohérence documentaire pour l'ensemble du lot.
