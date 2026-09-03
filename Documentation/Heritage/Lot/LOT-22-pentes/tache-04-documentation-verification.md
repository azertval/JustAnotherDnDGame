# TACHE-04 — Documentation et vérification {#lot-22-tache-04-documentation-verification}

**Lot :** [LOT-22](epic.md) · **Emplacement :** `Documentation` · **Statut :** fait

## Contexte
Dernière tâche du lot : aligner la documentation sur ce qui a été livré, comme pour chaque lot
précédent.

## Travail à réaliser
- **`Documentation/Specification/gameplay.md`** : retire le marqueur « ⚠️ non implémenté » d'
  `EX-GP-003` ; met à jour la ligne « Pente » du tableau des types de tuiles (§1).
- **`Documentation/Specification/niveaux.md`** : ajoute `slopeUpRight`/`slopeUpLeft` à la liste des
  types de tuiles du format JSON.
- **`Documentation/Guide/guide-physique.md`** : nouvelle section sur le suivi de pente — pourquoi
  une pente n'est pas solide pour `sweepX`, comment la nouvelle passe de résolution se compose avec
  le balayage existant, le compromis retenu (calage sauf en cas de saut volontaire).
- **`Documentation/Guide/guide-niveaux.md`** : ligne `SlopeUpRight`/`SlopeUpLeft` dans le tableau
  `core::TileType`.
- **`Documentation/Manuel/partager-un-niveau.md`** : nouvelle ligne dans le tableau des actions de
  l'éditeur.
- **`Documentation/Lot/lots.md`** : ajoute `@subpage lot-22`.
- **`CHANGELOG.md`** : entrée `[Non publié]` décrivant LOT-22 (`EX-GP-003`).
- **Doxygen** : génération locale complète (`doxygen Doxyfile` depuis `Documentation/`) avant de
  pousser.
- **`scripts/generate_cahier_test.py`** : relancé pour intégrer les nouveaux cas de test.
- **`scripts/lint_exigences.py`** : doit rester vert.

## Fichiers impactés
- `Documentation/Specification/gameplay.md`, `niveaux.md`.
- `Documentation/Guide/guide-physique.md`, `guide-niveaux.md`.
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
- Cette tâche clôt le lot : vérifier que les 5 critères d'acceptation d'`epic.md` sont
  effectivement remplis (en particulier le critère 4, non-régression) avant de le marquer terminé.

## Définition de fait (DoD)
- Documentation cohérente avec le code livré ; lint des exigences, Cahier de test et génération
  Doxygen locale verts ; `CHANGELOG.md` à jour ; `epic.md` marqué **terminé**, chaque tâche
  marquée ✅.

## Exigences
Aucune exigence propre — tâche de cohérence documentaire pour l'ensemble du lot.
