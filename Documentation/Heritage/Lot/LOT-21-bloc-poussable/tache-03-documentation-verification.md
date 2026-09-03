# TACHE-03 — Documentation et vérification {#lot-21-tache-03-documentation-verification}

**Lot :** [LOT-21](epic.md) · **Emplacement :** `Documentation` · **Statut :** fait

## Contexte
Dernière tâche du lot : aligner la documentation sur ce qui a été livré, comme pour chaque lot
précédent.

## Travail à réaliser
- **`Documentation/Specification/niveaux.md`** : ajoute `block` à la liste des types de tuiles du
  format JSON.
- **`Documentation/Guide/guide-niveaux.md`** : ligne `Block` dans le tableau `core::TileType` ;
  nouvelle section « Blocs poussables » (principe, poussée avant la physique, chute discrète,
  effacement de la position d'origine dans `collisionMap`) ; note sur la non-interaction
  bloc/plaque de pression dans le paragraphe existant sur le poids.
- **`Documentation/Manuel/jouer.md`** : ajoute le bloc à la liste des mécanismes rencontrés en jeu.
- **`Documentation/Manuel/partager-un-niveau.md`** : nouvelle ligne dans le tableau des actions de
  l'éditeur (« Placer un bloc poussable »).
- **`Documentation/Lot/lots.md`** : ajoute `@subpage lot-21`.
- **`CHANGELOG.md`** : entrée `[Non publié]` décrivant LOT-21 (`EX-GP-022`).
- **Doxygen** : génération locale complète (`doxygen Doxyfile` depuis `Documentation/`) avant de
  pousser.
- **`scripts/generate_cahier_test.py`** : relancé pour intégrer les nouveaux cas de test au
  Cahier de test (`Documentation/CahierTest.md`).
- **`scripts/lint_exigences.py`** : doit rester vert (`EX-GP-022` déjà déclarée, aucune nouvelle
  référence orpheline).

## Fichiers impactés
- `Documentation/Specification/niveaux.md`.
- `Documentation/Guide/guide-niveaux.md`.
- `Documentation/Manuel/jouer.md`, `partager-un-niveau.md`.
- `Documentation/Lot/lots.md`.
- `Documentation/CahierTest.md` (régénéré).
- `CHANGELOG.md`.

## Tests (obligatoires)
- `python scripts/lint_exigences.py` retourne un code de sortie `0`.
- `python scripts/generate_cahier_test.py --check` confirme que le Cahier de test est à jour.
- `doxygen Doxyfile` (depuis `Documentation/`) se termine avec un code de sortie `0` et **aucune**
  ligne de sortie.

## Points d'attention
- Cette tâche clôt le lot : vérifier que les critères d'acceptation d'`epic.md` sont tous
  effectivement remplis avant de le marquer terminé (statut + cases ✅ du tableau des tâches).
- La non-interaction bloc/plaque de pression (périmètre exclu de l'épic) doit rester **visible**
  dans `guide-niveaux.md`, pas seulement dans l'épic — un lecteur du guide qui n'a pas ouvert
  `Documentation/Lot/` doit comprendre cette limite sans avoir à lire l'historique des lots.

## Définition de fait (DoD)
- Documentation cohérente avec le code livré ; lint des exigences, Cahier de test et génération
  Doxygen locale verts ; `CHANGELOG.md` à jour ; `epic.md` marqué **terminé**, chaque tâche
  marquée ✅.

## Exigences
Aucune exigence propre — tâche de cohérence documentaire pour l'ensemble du lot.
