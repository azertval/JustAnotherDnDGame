# TACHE-04 — Documentation et vérification {#lot-29-tache-04-documentation-verification}

**Lot :** [LOT-29](epic.md) · **Emplacement :** `Documentation` · **Statut :** ✅

## Contexte
Dernière tâche du lot : aligner la documentation sur le comportement livré par TACHE-01/02/03 et
lever le statut « souhaité » d'`EX-CTRL-012`.

## Travail à réaliser
- **`Documentation/Specification/controles.md`** : `EX-CTRL-012` perd son marqueur `⚠️ souhaité` ;
  courte note renvoyant vers ce lot pour le détail des actions couvertes (jeu) et le sous-ensemble
  éditeur (hors tableau des actions de jeu, qui reste inchangé).
- **`Documentation/Lot/lots.md`** : ajoute `@subpage lot-29`.
- **`CHANGELOG.md`** : entrée `[Non publié]` décrivant le lot (deux sous-menus, persistance
  JSON, portée « actions clés seulement »).
- **`Documentation/Guide/guide-entrees.md`** (écart par rapport au périmètre initial de cette
  tâche, mémoire projet « Doc suit les lots ») : plusieurs passages décrivaient le remappage comme
  un bénéfice **hypothétique** de l'indirection action/touche — désormais réel, mis à jour
  (signature de `toPlayerInput`, filet de sécurité manette, nouvelle section dédiée au
  remappage).
- **Doxygen**, **Cahier de test**, **lint des exigences** : mêmes vérifications que les lots
  précédents — `python scripts/lint_exigences.py` et `python scripts/generate_cahier_test.py`
  **doivent être lancés par l'utilisateur** (pas de Python 3 dans ce sandbox, cf. mémoire projet).

## Fichiers impactés
- `Documentation/Specification/controles.md`.
- `Documentation/Lot/lots.md`.
- `Documentation/Guide/guide-entrees.md`.
- `Documentation/CahierTest.md` (régénéré par l'utilisateur).
- `CHANGELOG.md`.

## Tests (obligatoires)
- `python scripts/lint_exigences.py` retourne un code de sortie `0` (lancé par l'utilisateur).
- `python scripts/generate_cahier_test.py --check` confirme que le Cahier de test est à jour
  (lancé par l'utilisateur).
- `doxygen Doxyfile` (depuis `Documentation/`) se termine avec un code de sortie `0` et **aucune**
  ligne de sortie.
- Build complet `/W4 /WX` et suite de tests complète, sans régression.

## Points d'attention
- Aucun nouvel identifiant `EX-…` : ce lot **implémente** une exigence déjà existante
  (`EX-CTRL-012`), il n'en crée pas de nouvelle.

## Définition de fait (DoD)
- Documentation cohérente avec le comportement livré ; `epic.md` marqué **terminé**, chaque tâche
  marquée ✅.

## Exigences
`EX-CTRL-012` (levée de son statut souhaité).
