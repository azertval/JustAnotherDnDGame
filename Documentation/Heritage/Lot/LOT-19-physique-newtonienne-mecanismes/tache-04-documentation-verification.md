# TACHE-04 — Documentation et vérification {#lot-19-tache-04-documentation-verification}

**Lot :** [LOT-19](epic.md) · **Emplacement :** `Documentation` · **Statut :** fait

## Contexte
Dernière tâche du lot : aligner la documentation sur ce qui a été livré, comme pour chaque lot
précédent.

## Travail à réaliser
- **`Documentation/Guide/guide-physique.md`** : nouvelle section décrivant le modèle de chute
  newtonien (poids − traînée, vitesse terminale émergente, pourquoi la montée reste inchangée) —
  avec la dérivation du coefficient de traînée (continuité avec l'ancienne vitesse terminale).
- **`Documentation/Guide/guide-niveaux.md`** ou **`guide-editeur.md`** (selon où les mécanismes
  sont déjà documentés) : section sur la plaque de pression, sa différence avec l'interrupteur à
  bascule, et le rôle de `MIN_TRIGGER_MASS`.
- **`Documentation/Manuel/partager-un-niveau.md`** : ajouter la plaque de pression au tableau des
  actions de l'éditeur (guide non-codeur), si l'interrupteur y est déjà décrit.
- **`CHANGELOG.md`** : entrée `[Non publié]` décrivant LOT-19 (`EX-GP-019`, `EX-GP-025`).
- **Doxygen** : génération locale complète (`doxygen Doxyfile` depuis `Documentation/`) avant de
  pousser.
- **`scripts/lint_exigences.py`** : doit rester vert (`EX-GP-019`/`EX-GP-025` déclarées une fois
  chacune, aucune référence orpheline).

## Fichiers impactés
- `Documentation/Guide/guide-physique.md`.
- `Documentation/Guide/guide-niveaux.md` et/ou `guide-editeur.md`.
- `Documentation/Manuel/partager-un-niveau.md` (si applicable).
- `CHANGELOG.md`.
- `Documentation/Lot/lots.md` (ajout de la ligne LOT-19).

## Tests (obligatoires)
- `python scripts/lint_exigences.py` retourne un code de sortie `0`.
- `doxygen Doxyfile` (depuis `Documentation/`) se termine avec un code de sortie `0` et **aucune**
  ligne de sortie.

## Points d'attention
- Cette tâche clôt le lot : vérifier que les critères d'acceptation d'`epic.md` sont tous
  effectivement remplis avant de le marquer terminé (statut + cases ✅ du tableau des tâches).

## Définition de fait (DoD)
- Documentation cohérente avec le code livré ; lint des exigences et génération Doxygen locale
  verts ; `CHANGELOG.md` à jour ; `epic.md` marqué **terminé**, chaque tâche marquée ✅.

## Exigences
Aucune exigence propre — tâche de cohérence documentaire pour l'ensemble du lot.
