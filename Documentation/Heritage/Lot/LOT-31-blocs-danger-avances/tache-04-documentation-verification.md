# TACHE-04 — Documentation et vérification {#lot-31-tache-04-documentation-verification}

**Lot :** [LOT-31](epic.md) · **Emplacement :** `Documentation` · **Statut :** ✅

## Contexte
Dernière tâche : aligne la documentation sur le comportement réellement livré par
TACHE-01/02/03 — conformément au principe du projet, la documentation (`Guide`, format de niveau)
ne décrit que du code déjà implémenté, jamais en avance sur lui.

## Travail réalisé
- **`Documentation/Specification/gameplay.md`** : marqueurs ⚠️ « non implémenté » retirés de
  `EX-GP-050`–`EX-GP-053` et de la sous-section « Dangers avancés » (comportement livré, TACHE-01
  à 03).
- **`Documentation/Specification/niveaux.md`** : section « Format retenu » étendue aux sept
  nouveaux types (`dangerUp`/`dangerDown`/`dangerLeft`/`dangerRight`/`dangerMover`/
  `dangerSwitched`/`dangerBlink`) et leurs champs optionnels (valeurs par défaut incluses).
- **`Documentation/Guide/guide-niveaux.md`** : nouvelle section « Dangers avancés » —
  `core::dangerHitbox` (géométrique, sans état), `core::DangerController` (mobile/temporisé,
  nouveau), extension de `core::MechanismController` (commuté, pas de duplication de la détection
  front/continu), et la nouvelle signature de `core::evaluateOutcome`
  (`extraDangerBoxes`, composée par l'appelant faute de dépendance `Core/Levels` → `Core/Gameplay`).
- **`Documentation/Specification/editeur-niveaux.md`** : nouvelle section 9 — restructuration de la
  catégorie « Piège » (`EX-EDIT-018`), nouvelle exigence `EX-EDIT-019` (liaison danger commuté, même
  geste qu'une porte), et la réduction de périmètre actée en TACHE-03 (pas de widget dédié pour
  l'axe/la portée d'un danger mobile — valeurs de conception par défaut sans geste dédié).
- **`Source/Elements/Levels/README.md`** : liste des types de tuiles étendue aux sept nouveaux ;
  décompte de la séquence de démo mis à jour (14 fichiers).
- **`Documentation/Lot/lots.md`** : `@subpage lot-31` ajouté (fait dès le cadrage du lot).
- **`CHANGELOG.md`** : entrée `[Non publié]` décrivant le lot.
- **Doxygen**, **Cahier de test**, **lint des exigences** : lancés localement (`python
  scripts/lint_exigences.py`, `python scripts/generate_cahier_test.py`,
  `python scripts/check_demo_sequence.py`) — tous verts. Doxygen d'abord vérifié seulement avec le
  binaire **local** (1.17.0, zéro avertissement), poussé sans le binaire **1.9.8 exact de la CI** —
  la CI a échoué sur `tache-03-integration-editeur.md` : un span citait le chemin du fichier
  contenant `buildLevelScene` suivi d'un double deux-points et du nom de la fonction, exactement le
  piège connu (mémoire `project_doxygen_colon_link_pitfall`) — repéré par la CI, pas par le 1.17.0
  local, confirmant la mémoire `project_ci_local_reproduction`. Corrigé en citant seulement le
  symbole réellement documenté et son fichier séparément. Puis **revérifié avec le binaire 1.9.8
  exact** (téléchargé depuis `www.doxygen.nl/files/`) : `0`, aucune ligne de sortie. Relu aussi pour
  l'autre piège connu, deux astérisques fermantes suivies d'un slash refermant un commentaire
  prématurément (mémoire `project_bold_slash_comment_closing_pitfall`, découvert pendant ce lot) —
  aucune occurrence restante.

## Fichiers impactés
- `Documentation/Specification/gameplay.md`, `niveaux.md`, `editeur-niveaux.md`.
- `Documentation/Guide/guide-niveaux.md`.
- `Source/Elements/Levels/README.md`.
- `Documentation/Lot/lots.md`.
- `Documentation/CahierTest.md` (régénéré).
- `CHANGELOG.md`.

## Tests (réalisés)
- `python scripts/lint_exigences.py` : `0` (138 exigences déclarées, 111 référencées).
- `python scripts/generate_cahier_test.py --check` : à jour (535 cas de test, régénéré).
- `python scripts/check_demo_sequence.py` : `0` (14 niveaux, identiques et dans le même ordre).
- Doxygen : `0` aux deux binaires, local (1.17.0) **et** 1.9.8 exact de la CI.
- Build complet `/W4 /WX` et suite de tests complète (453 Unit + 81 Integration + 3 Système)
  vertes, sans régression.

## Définition de fait (DoD)
- Documentation cohérente avec le comportement livré ; `epic.md` marqué **terminé**, chaque tâche
  marquée ✅.
- Doxygen vérifié avec le binaire **1.9.8 exact de la CI**, pas seulement le binaire local — fait
  (après un premier push qui a révélé l'écart, cf. ci-dessus).

## Exigences
`EX-GP-050`, `EX-GP-051`, `EX-GP-052`, `EX-GP-053`, `EX-LVL-002`, `EX-LVL-004`, `EX-EDIT-001`,
`EX-EDIT-002`, `EX-EDIT-003`.
