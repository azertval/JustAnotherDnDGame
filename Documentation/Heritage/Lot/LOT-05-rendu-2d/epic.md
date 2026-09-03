# LOT-05 — Rendu 2D : atlas, sprites & caméra {#lot-05}

> Statut : **terminé**. L'exécutable affiche une scène ECS (grille de tuiles + sprite mobile) rendue en Direct3D 11 via la caméra 2D ; le `World` est cadencé à pas fixe et le rendu, découplé, lit les composants sans les muter.

## Objectif
Afficher à l'écran des entités de l'ECS (`Transform` + `Sprite`) via un pipeline
**Direct3D 11** de quads texturés, avec une **caméra 2D** monde → écran, et **brancher
le `World`** dans la boucle à pas de temps fixe. À l'issue du lot : l'exécutable affiche
une petite scène (grille de tuiles + un sprite) issue de l'ECS — la **première image
réelle** du jeu.

## Périmètre

### Inclus
- Composant `Sprite` (données pures dans `Core`) : région d'atlas + couche.
- Pipeline de **quads texturés** dans `HMI/Graphics` : shaders HLSL compilés à
  l'exécution (D3DCompile), *blending* alpha (transparence), échantillonnage **nearest**
  (pixel art).
- **Atlas de textures procédural** (généré en mémoire) : débloque le rendu sans asset.
- **Caméra 2D** : conversion monde → écran (16 px/unité, origine haut-gauche, Y-bas,
  zoom en facteurs entiers).
- **Système de rendu** (dans `HMI`) itérant `world.view<Transform, Sprite>()`, avec tri
  par **couche**.
- **Câblage du `World`** dans `main` : création, enregistrement des systèmes,
  `world.update(fixedDelta)` par pas fixe ; rendu découplé après la simulation.
- Scène de **démonstration** codée en dur (grille de tuiles + un sprite).

### Exclus (lots ultérieurs)
- **Chargement de niveaux depuis fichier** (lot dédié suivant ; ici la scène est codée en dur).
- Animation par séquence d'images (`EX-REN-012`).
- Caméra qui **suit** une cible et **bornage** au niveau (`EX-REN-013`) — ici caméra basique.
- Menu, pause, fin de niveau, texte (`EX-REN-030` à `EX-REN-032`) ; audio (`EX-REN-040`).

## Décisions de cadrage
- **Périmètre** : rendu **fondamental** (sprites + atlas + caméra basique + couches +
  câblage `World`), le reste différé.
- **Atlas** : **procédural** (généré en code), aucun asset ni chargeur d'image requis ;
  le chargement d'un vrai PNG viendra avec les graphismes.
- **Shaders** : **compilés à l'exécution** via `D3DCompile` (itération rapide en dev).

## Exigences couvertes
- `EX-REN-010` (grille de tuiles depuis un atlas), `EX-REN-011` (sprites + transparence),
  `EX-REN-014` (ordre de dessin par couches).
- `EX-REN-021` / `EX-REN-022` (logique à pas fixe, rendu découplé ; V-Sync).
- `EX-ARCH-012` (le rendu **lit** les composants sans les muter), `EX-ARCH-020` /
  `EX-ARCH-021` (unités monde, 16 px/unité), `EX-ARCH-022` (pixel art *nearest*).

## Découpage

> État : ✅ fait · 🔄 en cours · ⬜ non commencé.

| Tâche | Intitulé | Emplacement | État |
|-------|----------|-------------|:----:|
| [TACHE-01](tache-01-composant-sprite.md) | Composant `Sprite` (données pures) | `Core/Ecs/Components` | ✅ Fait |
| [TACHE-02](tache-02-pipeline-quads-textures.md) | Pipeline de quads texturés (HLSL, blend, nearest) | `HMI/Graphics` | ✅ Fait |
| [TACHE-03](tache-03-atlas-procedural.md) | Atlas de textures procédural | `HMI/Graphics` | ✅ Fait |
| [TACHE-04](tache-04-camera-2d.md) | Caméra 2D (monde → écran) | `HMI/Graphics` | ✅ Fait |
| [TACHE-05](tache-05-systeme-rendu-sprites.md) | Système de rendu des sprites (ECS → écran) | `HMI` | ✅ Fait |
| [TACHE-06](tache-06-cablage-world-demo.md) | Câblage du `World` dans la boucle + scène de démo | `HMI/main.cpp` | ✅ Fait |

## Critères d'acceptation du lot
1. L'exécutable affiche une **grille de tuiles** et **un sprite** (avec transparence),
   positionnés d'après des composants `Transform` de l'ECS.
2. La **caméra 2D** convertit correctement monde → écran (16 px/unité, Y-bas) ; déplacer
   la caméra déplace la scène de façon cohérente.
3. Les sprites sont dessinés dans l'ordre des **couches**.
4. Le `World` est **exécuté à pas fixe** dans la boucle (systèmes appelés) ; le rendu lit
   les composants **sans les muter**.
5. Build `/W4 /WX` sans avertissement, tests verts (logique testable de `Core`),
   documentation Doxygen à jour, `CHANGELOG.md` mis à jour.

## Dépendances
- Réutilise l'ECS (`World`, `Transform`, vues) de [LOT-03](@ref lot-03) et le cadenceur
  `FixedTimestep` de [LOT-01](@ref lot-01).
- S'appuie sur `hmi::GraphicsDevice` et `hmi::Window` (LOT-01) pour le device et la surface.

## Navigation des tâches
- @subpage lot-05-tache-01-composant-sprite
- @subpage lot-05-tache-02-pipeline-quads-textures
- @subpage lot-05-tache-03-atlas-procedural
- @subpage lot-05-tache-04-camera-2d
- @subpage lot-05-tache-05-systeme-rendu-sprites
- @subpage lot-05-tache-06-cablage-world-demo
