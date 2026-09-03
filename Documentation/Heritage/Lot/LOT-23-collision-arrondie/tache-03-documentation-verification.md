# TACHE-03 — Documentation et vérification {#lot-23-tache-03-documentation-verification}

**Lot :** [LOT-23](epic.md) · **Emplacement :** `Documentation` · **Statut :** fait

## Contexte
Dernière tâche du lot : aligner la documentation sur ce qui a été livré.

## Travail à réaliser
- **`Documentation/Specification/gameplay.md`** : retire le marqueur « ⚠️ non implémenté » d'
  `EX-GP-004` ; met à jour la ligne « Arrondi » du tableau des types de tuiles (§1).
- **`Documentation/Specification/niveaux.md`** : ajoute `roundedUpRight`/`roundedUpLeft` à la liste
  des types de tuiles du format JSON.
- **`Documentation/Guide/guide-physique.md`** : complète la section « suivi de pente » (`LOT-22`)
  pour couvrir la formule courbe et le point d'extension commun.
- **`Documentation/Guide/guide-niveaux.md`** : lignes `RoundedUpRight`/`RoundedUpLeft` dans le
  tableau `core::TileType`.
- **`Documentation/Manuel/partager-un-niveau.md`** : nouvelle ligne dans le tableau des actions de
  l'éditeur.
- **`Documentation/Lot/lots.md`** : ajoute `@subpage lot-23`.
- **`CHANGELOG.md`** : entrée `[Non publié]` décrivant LOT-23 (`EX-GP-004`).
- **Doxygen**, **Cahier de test**, **lint des exigences** : mêmes vérifications que les lots
  précédents.

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
- Cette tâche clôt le lot : vérifier que les 3 critères d'acceptation d'`epic.md` sont
  effectivement remplis avant de le marquer terminé.

## Définition de fait (DoD)
- Documentation cohérente avec le code livré ; `epic.md` marqué **terminé**, chaque tâche
  marquée ✅.

## Exigences
Aucune exigence propre — tâche de cohérence documentaire pour l'ensemble du lot.
