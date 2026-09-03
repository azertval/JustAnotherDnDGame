# TACHE-03 — Caméra : englober tout le niveau (éditeur et jeu) {#lot-16-tache-03-camera-niveau-entier}

**Lot :** [LOT-16](epic.md) · **Emplacement :** `HMI/Interface` · **Statut :** fait

## Contexte
`EditorScreen::renderGrid` (cadrage automatique **et** borne minimale du zoom manuel, LOT-15) et
`GameScreen::render` calculent chacun un facteur d'ajustement à la fenêtre, puis appliquent
`std::max(1.0f, std::floor(ajustement))` — un plancher qui empêche de descendre sous le zoom ×1.
Pour un niveau plus grand que la fenêtre (largeur ou hauteur × 16 px/case > taille fenêtre), cet
ajustement voudrait descendre sous 1 mais en est empêché : une partie de la grille reste **hors
champ**, dans l'éditeur comme en jeu, sans aucun moyen de la voir (l'éditeur a un pan/zoom manuel
depuis LOT-15, mais son zoom minimal hérite du même plancher ; le jeu n'a ni pan ni zoom manuel).

## Travail à réaliser
Même correction à trois emplacements, toujours la même logique : zoom **entier** (via
`std::floor`) tant que l'ajustement à la fenêtre est `≥ 1` (comportement inchangé pour tout niveau
tenant déjà dans la fenêtre — non-régression) ; zoom **fractionnaire** (valeur brute, sans
`floor`) uniquement lorsque l'ajustement est `< 1`, pour que le niveau entier tienne malgré tout à
l'écran :

```cpp
const float rawFit = std::min(fitX, fitY) * margin;
const float zoom = rawFit >= 1.0f ? std::floor(rawFit) : rawFit;
```

- **`EditorScreen::renderGrid`** (cadrage automatique, `!_manualCamera`) : remplacer le calcul de
  `_cameraZoom` par cette règle.
- **`EditorScreen` — bornes du zoom manuel** (molette, LOT-15) : `minZoom` (calculé sur la même
  formule que le cadrage automatique) doit suivre la même règle — sinon zoomer à la molette ne
  permettrait toujours pas de redescendre sous ×1 pour un grand niveau, malgré la correction du
  cadrage automatique.
- **`GameScreen::render`** : même remplacement pour le calcul de zoom d'ajustement au niveau
  joué.

## Fichiers impactés
- `Source/HMI/Interface/EditorScreen.cpp` (`renderGrid`, calcul de `minZoom` dans la gestion de la
  molette).
- `Source/HMI/Interface/GameScreen.cpp` (`render`).

## Tests (obligatoires)
- Non testable automatiquement (dépendance rendu D3D11/`Camera2D` utilisé via `EditorScreen`/
  `GameScreen`, non compilés dans `UnitTests`) : vérifié par relecture et essai manuel — charger
  (ou créer via TACHE-02) un niveau nettement plus grand que la fenêtre et constater qu'il est
  **entièrement visible** à l'ouverture, dans l'éditeur et en essai immédiat (`P`).
- Non-régression : un niveau existant (`demo.json` à `demo4.json`, 12×8 à 14×8) s'affiche avec
  **exactement** le même zoom entier qu'avant ce lot, dans l'éditeur comme en jeu.

## Points d'attention
- Ne pas introduire de division par zéro ou de zoom négatif/nul : `width`/`height` restent
  toujours `> 0` (garanti par `Core`), donc `fitX`/`fitY` restent strictement positifs — aucune
  garde supplémentaire nécessaire au-delà de la règle ci-dessus.
- `EX-ARCH-022` dit « zoom **de préférence** en facteurs entiers » — cette tâche ne change pas
  cette préférence par défaut (zoom `≥ 1` reste entier), elle ajoute seulement l'exception déjà
  anticipée par le mot « préférence » pour le cas où l'objectif « aucune zone invisible » l'exige.
- `Camera2D` (LOT-05) n'a besoin d'aucune modification : `setZoom` accepte déjà n'importe quel
  facteur positif, la limitation était entièrement dans le calcul appelant.

## Définition de fait (DoD)
- Cadrage automatique corrigé aux trois emplacements ; build `/W4 /WX` ; Doxygen à jour ; vérifié
  manuellement (grand niveau entièrement visible, petits niveaux inchangés).

## Exigences
`EX-REN-013` (corrigée), `EX-EDIT-013` (cadrage automatique de l'éditeur).
