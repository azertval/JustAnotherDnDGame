# TACHE-03 — Documentation et vérification {#lot-20-tache-03-documentation-verification}

**Lot :** [LOT-20](epic.md) · **Emplacement :** `Documentation` · **Statut :** fait

## Contexte
Dernière tâche du lot : aligner la documentation sur ce qui a été livré, comme pour chaque lot
précédent.

## Travail à réaliser
- **`Documentation/Guide/guide-entrees.md`** : nouvelle section sur la fusion manette — pourquoi
  elle vit dans `Window` (pas `InputState`), pourquoi deux sources indépendantes combinées en
  lecture plutôt qu'une écriture partagée (le bug de stomping évité), la table de correspondance.
- **`Documentation/Guide/guide-entrees.md`** ou nouvelle section dans un guide d'interface : le
  menu d'options, `OptionsModel`/`OptionsScreen`, réutilisation des constantes de mise en page de
  `MenuModel`.
- **`CHANGELOG.md`** : entrée `[Non publié]` décrivant LOT-20 (`EX-CTRL-002`).
- **Doxygen** : génération locale complète (`doxygen Doxyfile` depuis `Documentation/`) avant de
  pousser.
- **`scripts/lint_exigences.py`** : doit rester vert (aucune nouvelle exigence déclarée —
  `EX-CTRL-002` déjà déclarée, seulement son marqueur « souhaité » retiré).

## Fichiers impactés
- `Documentation/Guide/guide-entrees.md`.
- `CHANGELOG.md`.
- `Documentation/Lot/lots.md` (ajout de la ligne LOT-20).

## Tests (obligatoires)
- `python scripts/lint_exigences.py` retourne un code de sortie `0`.
- `doxygen Doxyfile` (depuis `Documentation/`) se termine avec un code de sortie `0` et **aucune**
  ligne de sortie.
- **Vérification visuelle obligatoire** dans l'application compilée : naviguer le menu principal,
  entrer dans Options, basculer V-Sync et la langue, revenir au menu — au clavier **et** à la
  souris (la manette réelle n'est pas disponible en CI/automatisation, vérifiée par les tests
  unitaires de TACHE-01 à la place).

## Points d'attention
- Cette tâche clôt le lot : vérifier que les critères d'acceptation d'`epic.md` sont tous
  effectivement remplis avant de le marquer terminé (statut + cases ✅ du tableau des tâches).

## Définition de fait (DoD)
- Documentation cohérente avec le code livré ; lint des exigences et génération Doxygen locale
  verts ; `CHANGELOG.md` à jour ; `epic.md` marqué **terminé**, chaque tâche marquée ✅.

## Exigences
Aucune exigence propre — tâche de cohérence documentaire pour l'ensemble du lot.
