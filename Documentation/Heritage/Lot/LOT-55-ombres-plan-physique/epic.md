# LOT-55 — Ombres du plan physique {#lot-55}

> Statut : **fait**. Prérequis : [LOT-41](@ref lot-41) (bascule). Bénéficie de
> [LOT-44](@ref lot-44) (fond) et [LOT-49](@ref lot-49) (décors). Dernier lot du programme
> d'habillage.

## Objectif
Aider visuellement le level designer — et, en mode Texture, le joueur — à distinguer ce qui est
**physique** (solide, collidable) de ce qui est **décor** : les tuiles solides projettent une ombre
douce sur ce qui se trouve derrière elles, purement visuelle.

C'est le complément du calque de premier plan (LOT-49) : le premier plan dit « ceci passe devant
vous, donc ne vous porte pas » ; l'ombre dit « ceci est en relief sur le décor, donc vous porte ».

Placé en dernier parce que c'est du polissage : il n'est prérequis d'aucun autre lot et n'apporte
rien tant que le reste de l'habillage n'existe pas.

## Périmètre

### Inclus
- Pour chaque tuile solide (`core::isSolid`/`TileMap::isSolid`, déjà existants dans `Core`),
  dessiner un quad sombre semi-transparent légèrement décalé, sur le calque *Shadow* de
  *RenderLayer* (réservé en LOT-40), **avant** le rendu de la tuile elle-même — visible uniquement
  là où le fond ou un décor d'arrière-plan est visible en dessous.
- **Ombres à la forme réelle** pour les douze **silhouettes** (pentes et arrondis, sol et plafond) et
  pour les **blocs réduits** (`BlockHalf`, `BlockQuarter`). Les silhouettes ne sont jamais solides au
  sens de `core::isSolid` — elles n'auraient donc aucune ombre — et les blocs réduits n'occupent
  qu'une fraction de leur case. L'ombre réutilise le masque de silhouette du `LOT-42` et
  `core::tileVisualScale`, **sans aucun nouveau prédicat de solidité dans `Core`**.
- Actif **uniquement** en mode Texture (LOT-41). Sans effet en mode Physique (déjà sans ambiguïté
  par la couleur plate) et **aucun** effet sur le gameplay (`EX-ARCH-012`).
- Dégradation propre : un niveau sans fond ni décor d'arrière-plan n'a simplement aucune surface
  visible pour recevoir l'ombre (pas d'erreur, pas de cas particulier à gérer).
- Les quads d'ombre passent par le culling comme tout le reste (LOT-40, TACHE-05).

### Exclus (hors périmètre de ce lot)
- Éclairage dynamique, ombres portées entre objets, tout système d'éclairage général — ce lot reste
  une silhouette sombre décalée, pas un moteur d'ombres.
- Ombres projetées par le personnage, les objets interactifs ou les décors.
- Configuration par le designer (intensité, angle, portée) : valeurs fixes, constantes nommées.
- Tout nouveau prédicat de solidité dans `Core` (cf. décisions de cadrage).

## Décisions de cadrage
- **Purement `HMI`, zéro nouvelle surface `Core`** : réutilise `core::isSolid`/`TileMap::isSolid`,
  `core::tileVisualScale` et la géométrie de silhouette (`core::slopeSurfaceHeight`,
  `ceilingSlopeHeight`, `isCeilingSlope`) — tous existants et publics.
- **Pas de prédicat de solidité partielle.** L'absence d'ombre sur les pentes appelle naturellement
  un `isSolidHalf` dans `Core` ; c'est écarté. Une ombre est la projection d'une **forme**, pas d'un
  degré de solidité : un booléen dirait qu'une pente est à moitié solide sans dire **où**. Or la
  forme est déjà exposée par les fonctions pures de géométrie de `Core`, dont *hmi::slopeShapePixel*
  se sert depuis `LOT-39` et que `LOT-42` factorise en masque réutilisable. Ajouter un prédicat de
  simulation pour un besoin purement visuel violerait `EX-ARCH-012` pour une information moins riche
  que celle déjà disponible.
- **Traité ici plutôt que dans un lot dédié.** La forme des silhouettes est acquise (masque de
  `LOT-42`) et celle des blocs réduits tient en une échelle : c'est une tâche, pas un lot. Le seul
  changement de nature est que l'ombre devient une **texture masquée** plutôt qu'un quad uni — ce qui
  fait dépendre ce lot de `LOT-42`, très en amont dans la séquence.
- **Calque *Shadow* déjà réservé en LOT-40** — ce lot est le premier et le seul à l'activer.
- **Placé en dernier** : aucun lot n'en dépend, et son intérêt est nul tant qu'il n'y a rien
  derrière les tuiles.
- **Valeurs fixes** : ouvrir le réglage au designer supposerait de le stocker (où ? par niveau ? par
  jeu de skins ?) pour un bénéfice non établi.

## Exigences couvertes
- Nouvelle : `EX-REN-045` (ombres du plan physique, mode Texture, purement visuel).
- Réutilisées : `EX-REN-043` (calques), `EX-REN-044` (fond), `EX-REN-046` (bascule), `EX-DEC-002`
  (couches de décor), `EX-EDIT-042` (masque de silhouette du `LOT-42`), `EX-GP-003`/`EX-GP-004`/
  `EX-GP-006`/`EX-GP-007` (silhouettes), `EX-GP-005` (blocs réduits), `EX-ARCH-012` (rendu non
  mutant, sans effet gameplay), `EX-NFR-005` (culling), `EX-NFR-004` (vérification sans GPU).

## Découpage

> État : ✅ fait · 🔄 en cours · ⬜ non commencé.

| Tâche | Intitulé | Emplacement | État |
|-------|----------|-------------|:----:|
| [TACHE-01](tache-01-quad-ombre.md) | Génération du quad d'ombre par tuile solide sur le calque *Shadow* | `Source/HMI/Graphics` | ✅ |
| [TACHE-03](tache-03-ombres-silhouettes.md) | Ombres à la forme réelle : silhouettes (pentes, arrondis) et blocs réduits | `Source/HMI/Graphics` | ✅ |
| [TACHE-02](tache-02-verification-documentation.md) | Vérification finale du programme + documentation | `Source/Test`, `Documentation` | ✅ |

> `TACHE-03` s'exécute **avant** `TACHE-02` (vérification finale) ; les numéros sont des
> identifiants, pas un ordre.

## Critères d'acceptation du lot
1. Les tuiles solides projettent une ombre visible sur le fond et sur les décors d'arrière-plan, en
   mode Texture uniquement.
2. Les pentes et arrondis projettent une ombre **à leur silhouette**, les blocs réduits une ombre
   **à leur taille réelle** ; aucun nouveau prédicat de solidité n'a été ajouté à `Core`.
3. Aucun quad d'ombre n'est émis en mode Physique — asserté via le *QuadRecorder*.
4. Le gameplay est inchangé (collisions identiques, tests existants inchangés).
5. Un niveau sans fond ni décor d'arrière-plan ne provoque aucune erreur ni artefact visuel.
6. Build `/W4 /WX`, Doxygen, lint verts ; vérification manuelle.

## Dépendances
Bâtit sur [LOT-41](@ref lot-41) (bascule), [LOT-40](@ref lot-40) (calque *Shadow*, culling,
*QuadRecorder*) et [LOT-42](@ref lot-42) (masque de silhouette réutilisé par les ombres de pentes).
Bénéficie de [LOT-44](@ref lot-44) (fond) et [LOT-49](@ref lot-49) (décors) sans en dépendre
strictement. Aucun lot ne dépend de celui-ci.

## Navigation des tâches
- @subpage lot-55-tache-01-quad-ombre
- @subpage lot-55-tache-03-ombres-silhouettes
- @subpage lot-55-tache-02-verification-documentation
