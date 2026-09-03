# LOT-28 — Arrondis concaves {#lot-28}

> Statut : **✅ terminé**. Quatre nouvelles tuiles, `ConcaveUpRight`/`ConcaveUpLeft`/
> `ConcaveDownRight`/`ConcaveDownLeft` (`EX-GP-007`) — une **seconde famille** de quart de cercle,
> **concave** plutôt que **convexe** (`RoundedUpRight`/`RoundedUpLeft` et leurs variantes de
> plafond, `EX-GP-004`/`EX-GP-006`) : le bloc arrondi « dans l'autre sens ».

## Objectif
L'arrondi existant (`EX-GP-004`) est un quart de cercle **convexe** : le centre du cercle est du
côté **creux**, la matière bombe vers l'extérieur (tangente verticale du côté creux, horizontale du
côté plein) — utile pour arrondir un coin **saillant** (bosse, sommet de pente). Il manque son
inverse : un quart de cercle **concave** (centre du cercle du côté **plein**), utile pour un
raccord **en creux** entre deux surfaces perpendiculaires (transition sol→mur en douceur, comme un
quart de tuyau/vallée) — tangente **horizontale** du côté creux, **verticale** du côté plein,
exactement l'inverse de l'arrondi convexe. Ce lot ajoute cette seconde famille en réutilisant
**entièrement** l'infrastructure de suivi de surface posée par `LOT-22`/`LOT-23` (sol) et `LOT-26`
(plafond) — seule la **formule** de hauteur change, comme `LOT-23` l'avait déjà fait pour
l'arrondi convexe par rapport à la pente linéaire.

## Périmètre

### Inclus
- Quatre nouvelles tuiles : `ConcaveUpRight`, `ConcaveUpLeft` (sol) et `ConcaveDownRight`,
  `ConcaveDownLeft` (plafond, miroir vertical des deux premières) — sol **et** plafond dans un seul
  lot (contrairement à `LOT-23`→`LOT-26`, séquencés séparément), le plafond ne demandant ici
  **aucune** formule nouvelle (`core::ceilingSlopeHeight` générique, déjà en place depuis
  `LOT-26`, n'a qu'à apprendre le mapping vers ces deux nouveaux types de sol).
- **Formule concave** : même rayon (une case) et mêmes valeurs aux bords que l'arrondi convexe de
  même orientation, mais centre du cercle au coin **haut** du côté creux plutôt qu'au coin **bas**
  du côté plein (voir décisions de cadrage pour le détail).
- **Collision par suivi**, réutilisant tel quel `core::resolveSlopeFollow`/
  `core::resolveCeilingSlopeFollow` (sol/plafond) — jamais solides pour la grille classique
  (`core::isSolid`), suivies en marchant pour les variantes de sol (`core::isFollowableSurface`),
  jamais pour les variantes de plafond (bloquent seulement un saut qui les franchit par en dessous).
- Rendu (silhouette réelle, en gris — même matériau qu'un `Solid`, cohérent avec l'arrondi convexe
  et les pentes), nouveau sous-groupe **Concave** dans la palette de l'éditeur (à côté du
  sous-groupe **Arrondi** existant, sous la catégorie **Tuile**), sérialisation JSON
  (`concaveUpRight`/`concaveUpLeft`/`concaveDownRight`/`concaveDownLeft`).

### Exclus (hors périmètre de ce lot)
- **Angle autre que 45°** ou **rayon différent** — hors périmètre, comme pour `LOT-22`/`LOT-23`.
- **Bloc poussable en forme d'arrondi concave** — une mécanique à part, non demandée ici (comme pour
  `LOT-23`/`LOT-26` : un bloc poussable traite toute case de pente/arrondi comme un obstacle
  simple, `EX-GP-022`, y compris cette nouvelle famille — aucun changement requis côté
  `BlockController`, qui ne distingue déjà pas les familles de courbe).
- **Niveau démo dédié** (`LOT-25`) — comme pour l'arrondi de plafond (`LOT-26`), ces tuiles sont un
  outil d'édition/décor ; les variantes de **sol**, elles, sont bien praticables en marchant
  (comme tout arrondi convexe), mais n'introduisent aucune mécanique de gameplay nouvelle à prouver
  isolément par un niveau démo — couvertes par les tests unitaires/intégration décrits ci-dessous.

## Décisions de cadrage
- **Formule concave, dérivée par raisonnement géométrique puis vérifiée par tangentes** :
  pour une case `[0, 1] × [0, 1]` (repère local de `core::slopeSurfaceHeight`, hauteur mesurée
  depuis le **haut**), l'arrondi convexe `RoundedUpRight` a pour centre de cercle le coin **bas**
  du côté creux (`(0, 1)`, coin bas-gauche) : `h(x) = 1 - sqrt(1 - (1-x)²)`, tangente **verticale**
  en `x=0` (côté creux), **horizontale** en `x=1` (côté plein). `ConcaveUpRight` **inverse** ce
  choix de centre : coin **haut** du côté creux (`(0, 0)`, coin haut-gauche) plutôt que coin bas —
  `h(x) = sqrt(1 - x²)`, tangente **horizontale** en `x=0` (raccord lisse avec un sol plat en
  contrebas), **verticale** en `x=1` (raccord lisse avec un mur), l'inverse exact du convexe. Mêmes
  valeurs aux deux bords que `RoundedUpRight` (`1` à `x=0`, `0` à `x=1`) — seule la courbe entre les
  deux diffère. `ConcaveUpLeft` est le symétrique (`h(x) = sqrt(1 - (1-x)²)`, centre en `(1, 0)`,
  coin haut-droit). Confirmé avec le demandeur via un aperçu ASCII (convexe bombé vs. concave
  creux) avant toute implémentation.
- **Aucune nouvelle fonction de résolution** (décision initiale, invalidée en cours de lot — voir
  ci-dessous) : le plan prévoyait `core::resolveSlopeFollow`/`core::resolveCeilingSlopeFollow`
  inchangées (comme `LOT-23` l'avait démontré pour l'arrondi convexe vis-à-vis de `LOT-22`), seuls
  `core::slopeSurfaceHeight` (deux nouveaux `case`) et le mapping interne de
  `core::ceilingSlopeHeight` (deux nouvelles entrées miroir) devant changer.
- **Écart de cadrage majeur, en trois volets successifs** (voir TACHE-01, section « Points
  d'attention », pour le détail complet de chacun) : des essais manuels en jeu ont révélé que
  `resolveSlopeFollow`/`resolveCeilingSlopeFollow` sélectionnaient la case à consulter uniquement
  par le **centre** de la boîte du personnage, ignorant sa largeur — pour la dernière fraction de
  la largeur de toute case pente/arrondi/concave, ce centre atterrit déjà dans la case **voisine**,
  sans filet si celle-ci est non solide (typiquement deux concaves posés côte à côte, l'arc/la
  voûte étant justement l'usage visé par ce lot). Reproduit sur `SlopeDownRight` (`LOT-26`, pente
  linéaire) : **défaut antérieur à ce lot**, jamais exposé faute d'avoir déjà chaîné deux tuiles non
  solides. Corrigé en élargissant la sélection de colonne aux colonnes réellement couvertes par la
  largeur de la boîte (colonne centrale inchangée, colonnes supplémentaires échantillonnées à leur
  propre bord de boîte) — la signature de `ce serait le signe que l'abstraction... était mal choisie
  »` anticipée ci-dessus s'est donc concrétisée : l'abstraction à sélection par centre seul, posée
  par `LOT-22`, avait cette lacune latente. Un second volet, plus profond, a suivi : ce premier
  correctif ne considérait que la boîte du pas précédent, insuffisant quand marcher tout en sautant
  fait sortir la boîte d'une colonne pertinente sur plusieurs pas consécutifs avant que le seuil de
  blocage n'y soit atteint — corrigé par une mémoire de l'étendue horizontale couverte depuis le
  début de la montée courante (`Player::ascentSweepMinX`/`ascentSweepMaxX`). Un troisième volet,
  distinct des deux premiers, a suivi leur correction : un saut bloqué près du bord fin (silhouette
  quasi vide) d'un arrondi concave de plafond se retrouvait téléporté au-dessus du plafond un pas
  après le blocage — corrigé en restreignant le calage sur face du haut de plafond aux cas où le
  bord bas était déjà au-dessus de la case avant le pas. Les trois correctifs restent strictement
  additifs (comportement inchangé par construction pour les cas déjà couverts) et vérifiés sans
  régression sur la suite de tests complète.
- **Sol et plafond dans le même lot** (contrairement à `LOT-23`→`LOT-26`, séquencés séparément) :
  choix délibéré avec le demandeur — le plafond concave ne coûte ici qu'un mapping de deux lignes
  dans `ceilingSlopeHeight` (infrastructure déjà générique depuis `LOT-26`), pas une nouvelle passe
  de physique ; séparer en deux lots n'apporterait aucune réduction de risque supplémentaire.
- **Rendu en gris uniforme**, pas une couleur distincte par variante — cohérent avec la décision
  déjà prise pour pentes/arrondis convexes et leurs variantes de plafond.
- **Aucun agrandissement de la grille de tuiles de l'atlas procédural** : `TextureAtlas::
  TILES_PER_SIDE` reste à `5` (25 cases). L'agrandissement `4→5` fait par `LOT-26` avait laissé
  **six** cases réellement libres (une seule silhouette de plus aurait suffi, mais un agrandissement
  de grille se fait toujours par lot entier) — sept cases semblaient libres à première vue, mais
  l'une d'elles (`(4,4)`) est en réalité réservée au damier de transparence
  (`TextureAtlas::transparentTileIndex`, écart constaté en cours de lot, voir TACHE-02/TACHE-03) ;
  les quatre nouvelles silhouettes de ce lot logent dans quatre des six cases authentiquement
  libres (couleur noire par convention, `tileColor`), sans toucher au jeu de couleurs existant ni à
  aucune couleur de tuile déjà en place.
- **Nouveau sous-groupe de palette « Concave »**, frère de « Arrondi » sous la catégorie « Tuile »
  (`LOT-27`) — pas une fusion dans le sous-groupe « Arrondi » existant : les deux familles de
  courbe (convexe/concave) sont des choix de forme distincts pour le niveau, une entrée de palette
  par famille reste plus lisible qu'un sous-groupe mélangeant les deux.

## Exigences couvertes
- `EX-GP-007` — nouvelle exigence.

## Découpage

> État : ✅ fait · 🔄 en cours · ⬜ non commencé.

| Tâche | Intitulé | Emplacement | État |
|-------|----------|-------------|:----:|
| [TACHE-01](tache-01-modele-physique-concave.md) | Modèle de tuile et formule de courbe concave | `Core/Levels`, `Core/Physics` | ✅ |
| [TACHE-02](tache-02-editeur-rendu.md) | Éditeur et rendu | `HMI/Editor`, `HMI/Graphics` | ✅ |
| [TACHE-03](tache-03-documentation-verification.md) | Documentation et vérification | `Documentation` | ✅ |

## Critères d'acceptation du lot
1. Le personnage suit une tuile `ConcaveUpRight`/`ConcaveUpLeft` en marchant dessus, avec une courbe
   visuellement et physiquement distincte de l'arrondi convexe existant (courbure inversée).
2. Un saut qui franchit une tuile `ConcaveDownRight`/`ConcaveDownLeft` par en dessous est bloqué
   précisément selon sa silhouette ; un personnage qui tombe sur sa face du haut (plate) s'y pose
   normalement.
3. **Aucune régression** sur la physique existante (sol, pentes/arrondis convexes de sol et de
   plafond, murs, sauts, blocs poussables) — même suite de tests, inchangée, reste verte.
4. Placeables depuis la palette de l'éditeur (nouveau sous-groupe « Concave »), sérialisables
   (JSON), rendues avec leur silhouette réelle en gris, sans décaler aucune couleur de tuile
   existante dans l'atlas.
5. Logique nouvelle **couverte par des tests** dédiés (formule de courbe aux bords et au centre,
   classification statique, mapping miroir plafond, aller-retour JSON, physique de suivi/blocage).
   Build `/W4 /WX` sans avertissement, Doxygen et lint des exigences verts.

## Dépendances
- **Dépend de `LOT-22`/`LOT-23`** (infrastructure de suivi de surface, sol) et de `LOT-26`
  (infrastructure de suivi de silhouette, plafond, `core::ceilingSlopeHeight` déjà générique) — ne
  doit pas être commencé avant que ces trois lots soient terminés (ils le sont).
- Étend `TileType` (`LOT-03`/`LOT-07`).

## Navigation des tâches
- @subpage lot-28-tache-01-modele-physique-concave
- @subpage lot-28-tache-02-editeur-rendu
- @subpage lot-28-tache-03-documentation-verification
