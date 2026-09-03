# TACHE-03 — Interpolation de rendu (EX-ARCH-031) {#lot-33-tache-03-interpolation-rendu}

**Lot :** [LOT-33](epic.md) · **Modules :** `Source/HMI/Graphics`, `Source/HMI/Interface` · **Statut :** terminé

## Contexte
La simulation avance par **pas fixes** de 1/60 s (déterminisme, `EX-NFR-002`) tandis que le rendu
redessine une fois par frame réelle, souvent plus fréquente (écran 120/144 Hz). Sans interpolation,
une entité en mouvement continu est figée à sa **dernière position simulée** pendant plusieurs
frames de rendu, puis « saute » d'un coup au pas suivant : un *judder* en marches d'escalier, visible
même à framerate élevé. L'architecture prévoyait pourtant **dès le départ** un facteur
d'interpolation `[0, 1]` fourni au rendu (`EX-ARCH-031`), exposé par
`core::FixedTimestep::interpolationAlpha` — mais aucun code ne l'exploitait. Cette tâche le
concrétise, en restant une préoccupation **strictement `HMI`** : `Core` n'est pas modifié.

## Travail à réaliser
- **Composant de présentation** `hmi::PreviousPosition` (`HMI/Graphics/PreviousPosition.h`) :
  position de l'entité au pas précédent. Rangé dans le `core::World` (générique sur le type de
  composant) mais **écrit et lu par `HMI` uniquement** — `Core` l'ignore.
- **Instantané par pas** (`GameScreen::snapshotPreviousPositions`) : au **début** de chaque pas fixe
  (`update`), recopier la position **courante** (`core::Transform`) de chaque entité mobile vers son
  `PreviousPosition`, avant que le pas ne la modifie. Après le pas, `Transform` porte la nouvelle
  position et `PreviousPosition` l'ancienne.
- **Attribution du composant** aux seules entités mobiles, à la (re)construction de la scène :
  personnage (`spawnPlayer`), dangers mobiles (`_moverEntities`), blocs poussables
  (`_blockEntities`) — initialisé à leur position de départ (aucun glissement parasite à la première
  frame après (re)chargement). Les tuiles fixes, portes et dangers à état n'en reçoivent pas.
- **Facteur d'interpolation au rendu** : `RenderContext` gagne un champ `interpolationAlpha`, rempli
  par la boucle (`main.cpp`) depuis `FixedTimestep::interpolationAlpha`. `GameScreen::render` le
  transmet à `SpriteRenderer::render`.
- **Dessin interpolé** (`SpriteRenderer::render`) : pour chaque entité, si elle porte un
  `PreviousPosition`, dessiner à `lerp(précédente, courante, alpha)` ; sinon à la position courante.
  Lire cette seconde pool pendant l'itération de la vue `Transform`+`Sprite` est sûr (aucune pool
  n'est modifiée, la vue reste valide).
- **Non-interpolation de la caméra** : inutile, elle bascule par coupure nette (`LOT-32`).

## Fichiers impactés
- `Source/HMI/Graphics/PreviousPosition.h` (nouveau).
- `Source/HMI/Graphics/SpriteRenderer.h`, `Source/HMI/Graphics/SpriteRenderer.cpp` : paramètre
  `interpolationAlpha`, lerp par entité.
- `Source/HMI/Interface/RenderContext.h` : champ `interpolationAlpha`.
- `Source/HMI/Interface/GameScreen.h`, `Source/HMI/Interface/GameScreen.cpp` :
  `snapshotPreviousPositions`, attribution du composant, passage de l'alpha au rendu.
- `Source/HMI/main.cpp` : `RenderContext::interpolationAlpha` alimenté depuis le cadenceur.

## Tests (obligatoires)
- `core::FixedTimestep::interpolationAlpha` : couvert (fraction de pas restante après consommation).
- Le lerp par entité est du code de rendu (dépendance D3D11) : vérifié **visuellement** — le
  mouvement du personnage est lisse à haut framerate, sans marche d'escalier, et une (re)apparition
  ne « glisse » pas depuis l'origine.
- Non-régression : la suite (unitaires + intégration + **système**) reste verte — l'interpolation
  n'affecte **que** l'affichage, jamais les positions simulées lues par la logique de fin de niveau.

## Définition de fait (DoD)
- Mouvement lisse à 120/144 Hz, simulation inchangée (déterminisme préservé), aucun glissement à la
  (re)apparition.
- Compile `/W4 /WX`, formaté, API documentée `.h` + `.cpp`.

## Exigences
`EX-ARCH-031`, `EX-REN-021`, `EX-ARCH-030`, `EX-ARCH-012`, `EX-NFR-010`, `EX-NFR-002`.
