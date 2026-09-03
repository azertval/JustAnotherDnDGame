# TACHE-04 — Documentation et vérification {#lot-30-tache-04-documentation-verification}

**Lot :** [LOT-30](epic.md) · **Emplacement :** `Documentation` · **Statut :** ✅

## Contexte
Dernière tâche : aligne la documentation sur le comportement livré par TACHE-01/02/03.

## Travail à réaliser
- **`Documentation/Specification/controles.md`** : colonne « Manette (défaut) » du tableau des
  actions mise à jour pour préciser que le mapping est désormais reconfigurable (`EX-CTRL-012`
  s'étend à la manette).
- **`Documentation/Guide/guide-entrees.md`** : met à jour la section sur le filet de sécurité
  manette de `LOT-29` (retiré, remplacé par `GamepadBindings`) ; nouvelle section sur le
  remappage manette, même style que celle du remappage clavier.
- **`Documentation/Lot/lots.md`** : ajoute `@subpage lot-30`.
- **`CHANGELOG.md`** : entrée `[Non publié]` décrivant le lot.
- **Doxygen**, **Cahier de test**, **lint des exigences** : lancés localement (`python
  scripts/lint_exigences.py`, `python scripts/generate_cahier_test.py`,
  `python scripts/check_demo_sequence.py` — tous fonctionnels dans ce sandbox), et Doxygen
  **vérifié avec le binaire 1.9.8 exact de la CI** avant tout push (voir mémoire
  `project_ci_local_reproduction`).

## Fichiers impactés
- `Documentation/Specification/controles.md`.
- `Documentation/Guide/guide-entrees.md`.
- `Documentation/Lot/lots.md`.
- `Documentation/CahierTest.md` (régénéré).
- `CHANGELOG.md`.

## Tests (obligatoires)
- `python scripts/lint_exigences.py` retourne un code de sortie `0`.
- `python scripts/generate_cahier_test.py --check` confirme que le Cahier de test est à jour.
- `python scripts/check_demo_sequence.py` retourne un code de sortie `0`.
- Doxygen (binaire 1.9.8 local, voir ci-dessus) se termine avec un code de sortie `0` et **aucune**
  ligne de sortie.
- Build complet `/W4 /WX` et suite de tests complète, sans régression.

## Définition de fait (DoD)
- Documentation cohérente avec le comportement livré ; `epic.md` marqué **terminé**, chaque tâche
  marquée ✅.

## Exigences
`EX-CTRL-002`, `EX-CTRL-012`.
