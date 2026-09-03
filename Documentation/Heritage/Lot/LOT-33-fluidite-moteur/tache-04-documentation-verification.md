# TACHE-04 — Documentation, spécifications et vérification {#lot-33-tache-04-documentation-verification}

**Lot :** [LOT-33](epic.md) · **Module :** `Documentation` · **Statut :** en cours

## Contexte
Les tâches 01 à 03 changent le comportement de la boucle, des entrées et du rendu décrits dans les
guides et référencent une exigence nouvelle (`EX-REN-004`) tout en en concrétisant une existante
(`EX-ARCH-031`). Les spécifications et guides doivent rester la **source de vérité** : ils sont mis
à jour en conséquence, et la nouvelle exigence est déclarée par une ancre unique (lint CI).

## Travail à réaliser
- **Spécifications** :
  - `rendu-technique.md` : nouvelle `EX-REN-004` (modèle de présentation flip, faible latence).
  - `architecture.md` : `EX-ARCH-031` annotée « concrétisée en `LOT-33` » (interpolation désormais
    exploitée par le rendu).
  - `controles.md` : préciser sur `EX-CTRL-020`/`EX-CTRL-021` que la conformité vaut désormais à
    **tout framerate** de rendu (fronts consommés par pas, non par frame — `LOT-33`).
- **Guides** :
  - `guide-boucle.md` : la section `interpolationAlpha` ne dit plus « exposé mais pas exploité » ;
    décrit le découplage fronts d'entrée ↔ pas (correction des entrées perdues).
  - `guide-rendu.md` : flip model (`GraphicsDevice`), interpolation dans `SpriteRenderer`
    (`hmi::PreviousPosition`, facteur d'interpolation).
  - `guide-entrees.md` : cycle d'une frame mis à jour (échantillonnage par frame, avancée des
    fronts par pas consommé, `beginInputFrame`), relâchement à la perte de focus, throttling manette.
- **Navigation Doxygen** : `Documentation/Lot/lots.md` (ajout `@subpage lot-33`), l'epic référence
  ses tâches par `@subpage`.
- **CHANGELOG** : entrée `LOT-33` dans *Non publié*.
- **Vérification** : build `/W4 /WX` sans avertissement, `ctest` 100 %, `lint_exigences.py` vert
  (ancre `EX-REN-004` unique, aucune référence orpheline), Doxygen sans avertissement, vérification
  visuelle du rendu (fluidité, flip model, entrées nerveuses) — dépendances D3D11/Win32.

## Fichiers impactés
- `Documentation/Specification/rendu-technique.md`, `architecture.md`, `controles.md`.
- `Documentation/Guide/guide-boucle.md`, `guide-rendu.md`, `guide-entrees.md`.
- `Documentation/Lot/lots.md`, `Documentation/Lot/LOT-33-fluidite-moteur/*`.
- `CHANGELOG.md`.

## Définition de fait (DoD)
- Spécifications et guides cohérents avec le code livré ; `EX-REN-004` déclarée et référencée.
- `lint_exigences.py`, Doxygen, build `/W4 /WX` et `ctest` verts.

## Exigences
`EX-REN-004`, `EX-ARCH-031`, `EX-CTRL-020`, `EX-CTRL-021`, `EX-NFR-012`, `EX-NFR-013`.
