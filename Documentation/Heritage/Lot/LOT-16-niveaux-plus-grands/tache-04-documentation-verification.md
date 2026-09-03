# TACHE-04 — Documentation et vérification {#lot-16-tache-04-documentation-verification}

**Lot :** [LOT-16](epic.md) · **Emplacement :** `Documentation` · **Statut :** fait

## Contexte
Dernière tâche du lot : aligner la documentation sur ce qui a été livré, comme pour chaque lot
précédent.

## Travail à réaliser
- **`Documentation/Guide/guide-editeur.md`** : nouvelle section décrivant la boîte de dialogue de
  redimensionnement (généralisation du slot de saisie de texte, réutilisation de
  `LevelSizeValidation` sur le modèle de `LevelNameValidation`) et la correction du cadrage
  caméra (pourquoi le zoom devient fractionnaire au-delà de l'ajustement ×1, et pourquoi c'est une
  exception locale à `EX-ARCH-022` plutôt qu'un changement de politique).
- **`Documentation/Guide/guide-rendu.md`** : vérifier si ce guide décrit le calcul de zoom
  d'ajustement de `GameScreen` (caméra fixe cadrant le tableau) ; si oui, corriger dans le même
  esprit que `EX-REN-013` (caméra qui englobe le niveau entier, pas de suivi du joueur).
- **`Documentation/Manuel/partager-un-niveau.md`** : ajouter `Ctrl+R` au tableau des actions de
  l'éditeur (guide non-codeur).
- **`CHANGELOG.md`** : entrée `[Non publié]` décrivant LOT-16 (`EX-EDIT-017`, `EX-REN-013`
  corrigée), à l'image des entrées LOT-14/15 déjà présentes.
- **Doxygen** : génération locale complète (`doxygen Doxyfile` depuis `Documentation/`) avant de
  pousser — LOT-15 a laissé un échec CI (caractères `< >` non échappés dans un span de code) qui
  aurait été attrapé par cette vérification ; ne pas s'y reprendre à deux fois.
- **`scripts/lint_exigences.py`** : doit rester vert (nouvelle exigence `EX-EDIT-017` déclarée une
  fois, `EX-REN-013` réutilisée sans nouvelle déclaration, aucune référence orpheline).

## Fichiers impactés
- `Documentation/Guide/guide-editeur.md`, potentiellement `Documentation/Guide/guide-rendu.md`.
- `Documentation/Manuel/partager-un-niveau.md`.
- `CHANGELOG.md`.

## Tests (obligatoires)
- `python scripts/lint_exigences.py` retourne un code de sortie `0`.
- `doxygen Doxyfile` (depuis `Documentation/`) se termine avec un code de sortie `0` et **aucune**
  ligne de sortie (le `Doxyfile` du projet est configuré `QUIET=YES`, `WARN_AS_ERROR=
  FAIL_ON_WARNINGS` : un avertissement produirait une sortie non vide et ferait échouer le CI
  `docs.yml`).

## Points d'attention
- Si un passage nouvellement écrit doit montrer un caractère `<`/`>` littéral dans un span en
  accent grave, ne pas le faire — Doxygen l'interprète comme une balise HTML mal fermée à
  l'intérieur du `<tt>` qu'il génère (rencontré en LOT-15). Écrire le caractère en toutes lettres
  (« chevron ouvrant/fermant ») plutôt que par son glyphe.
- Cette tâche clôt le lot : vérifier que les critères d'acceptation d'`epic.md` sont tous
  effectivement remplis avant de le marquer terminé (statut + cases ✅ du tableau des tâches).

## Définition de fait (DoD)
- Documentation cohérente avec le code livré ; lint des exigences et génération Doxygen locale
  verts ; `CHANGELOG.md` à jour ; `epic.md` marqué **terminé**, chaque tâche marquée ✅.

## Exigences
Aucune exigence propre — tâche de cohérence documentaire pour l'ensemble du lot.
