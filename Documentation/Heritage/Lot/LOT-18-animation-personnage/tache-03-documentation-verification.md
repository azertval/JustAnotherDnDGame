# TACHE-03 — Documentation et vérification {#lot-18-tache-03-documentation-verification}

**Lot :** [LOT-18](epic.md) · **Emplacement :** `Documentation` · **Statut :** fait

## Contexte
Dernière tâche du lot : aligner la documentation sur ce qui a été livré, comme pour chaque lot
précédent.

## Travail à réaliser
- **`Documentation/Guide/guide-rendu.md`** : étendre la section sur la région du personnage
  (introduite en LOT-17) pour décrire la grille de frames, le composant/système `Animation`
  (`Core`) et le rebranchement de `sprite.region` à chaque frame plutôt qu'au spawn seul.
- **`Documentation/Guide/guide-ecs.md`** : si ce guide illustre les systèmes existants
  (`MovementSystem`, `CharacterPhysicsSystem`) comme exemples, vérifier s'il faut y ajouter
  `AnimationSystem` comme illustration supplémentaire (système dérivant un état de présentation
  d'un état de simulation, sans nouvel état ajouté à `Player`).
- **`Source/HMI/Graphics/README.md`** : mettre à jour la ligne `TextureAtlas` (grille de frames au
  lieu d'une région unique).
- **`Source/Core/Ecs/Components/README.md`** / **`Source/Core/Ecs/Systems/README.md`** (si
  présents) : ajouter `Animation`/`AnimationSystem` à la liste.
- **`CHANGELOG.md`** : entrée `[Non publié]` décrivant LOT-18 (`EX-REN-012`, couverture complète
  des trois clips repos/course/saut).
- **Doxygen** : génération locale complète (`doxygen Doxyfile` depuis `Documentation/`) avant de
  pousser.
- **`scripts/lint_exigences.py`** : doit rester vert (aucune nouvelle exigence déclarée —
  `EX-REN-012` déjà déclarée en LOT-05, seulement référencée à nouveau ici).

## Fichiers impactés
- `Documentation/Guide/guide-rendu.md` (et `guide-ecs.md` si applicable).
- `Source/HMI/Graphics/README.md` (et READMEs `Core/Ecs` si présents).
- `CHANGELOG.md`.
- `Documentation/Lot/lots.md` (ajout de la ligne LOT-18).

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
