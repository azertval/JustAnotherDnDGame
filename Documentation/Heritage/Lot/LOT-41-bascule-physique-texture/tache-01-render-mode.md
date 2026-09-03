# TACHE-01 — *RenderMode* et câblage du viewport {#lot-41-tache-01-render-mode}

**Lot :** [LOT-41](epic.md) · **Emplacement :** `Source/HMI/Game`, `Source/HMI/Graphics` · **Statut :** fait

## Contexte
Le rendu actuel n'a qu'un mode : la couleur plate par type de tuile, résolue par
`hmi::regionForTile` (`TileVisuals`). À partir de LOT-42, un second mode coexiste — l'habillage par
textures. Les deux doivent être disponibles **partout** où une scène est dessinée : en édition
(`hmi::DraftRenderer`), en essai et en jeu réel (`hmi::GameSession`).

Le point d'entrée commun existe déjà : `hmi::GameViewport` est le widget unique derrière ces trois
contextes (`EX-IHM-002`). C'est donc lui qui porte le mode, et non chaque appelant.

## Travail à réaliser
- ***RenderMode*** (`Source/HMI/Graphics/RenderMode.h`) : `enum class RenderMode { Physique,
  Texture };`. Deux valeurs, pas de troisième — le contrôle fin par calque relève de LOT-51.
- **`GameViewport`** détient le mode courant, l'expose en lecture (`renderMode()`) et en écriture
  (`setRenderMode`), et le transmet à chaque appel de rendu :
  - `DraftRenderer::render(...)` gagne le mode en paramètre ;
  - `GameSession::render(...)` de même, qui le relaie à la composition de la scène.
- **Point de résolution unique** : la fonction qui décide de l'apparence d'une tuile reçoit le mode
  et n'a, à ce stade, que deux branches — `Physique` → `hmi::regionForTile` (comportement actuel,
  inchangé) ; `Texture` → la texture de repli en damier magenta (LOT-40, TACHE-03), puisqu'aucun
  skin n'existe encore. Les branches suivantes (skin, surcharge) s'y ajouteront sans changer la
  forme de ce résolveur.

## Fichiers impactés
- `Source/HMI/Graphics/RenderMode.h` (nouveau).
- `Source/HMI/Graphics/DraftRenderer.{h,cpp}`, `Source/HMI/Graphics/SpriteRenderer.{h,cpp}`.
- `Source/HMI/Game/GameViewport.{h,cpp}`, `Source/HMI/Game/GameSession.{h,cpp}`.

## Tests (obligatoires)
- Le résolveur d'apparence est une fonction **pure** prenant le mode : mode `Physique` → région
  d'atlas attendue pour chaque `core::TileType` ; mode `Texture` → repli. Testé sans GPU.
- Non-régression : en mode `Physique`, la liste des primitives composées pour une scène de référence
  est identique à celle d'avant le lot (*QuadRecorder*, LOT-40).

## Points d'attention
- Le mode est une donnée de **présentation** : `Core` ne doit en aucun cas le connaître.
  `core::buildLevelScene` continue de recevoir une fonction de résolution **injectée** depuis `HMI`.
- Ne pas dupliquer le résolveur entre `GameSession` et `DraftRenderer` : c'est le même point d'appel,
  c'est ce qui garantit que l'éditeur montre exactement ce que le jeu affichera.
- Changer de mode ne doit **pas** reconstruire la scène ECS : seule la résolution d'apparence change.

## Définition de fait (DoD)
- Le mode traverse le viewport jusqu'aux deux renderers ; le mode `Physique` est inchangé ; le
  résolveur est unique et testé sans GPU ; `/W4 /WX` propre.

## Exigences
`EX-REN-046` (bascule Physique/Texture) ; réutilise `EX-IHM-002` (viewport unique), `EX-REN-043`
(calques), `EX-NFR-040` (repli), `EX-NFR-004` (vérification sans GPU).
