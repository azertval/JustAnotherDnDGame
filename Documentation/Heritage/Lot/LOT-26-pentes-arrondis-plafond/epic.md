# LOT-26 — Pentes et arrondis de plafond {#lot-26}

> Statut : **terminé**. Quatre nouvelles tuiles, `SlopeDownRight`/`SlopeDownLeft`/
> `RoundedDownRight`/`RoundedDownLeft` (`EX-GP-006`) — miroir vertical des pentes/arrondis de sol
> (`EX-GP-003`/`EX-GP-004`, `LOT-22`/`LOT-23`) : matière pleine en haut de la case plutôt qu'en
> bas, un mur incliné/courbe qui bloque un saut par le dessus, jamais une surface qu'on parcourt.

## Objectif
L'éditeur ne permettait de poser des pentes/arrondis qu'au **sol** (`EX-GP-003`/`EX-GP-004`) : une
silhouette inclinée/courbe donnant lieu à un **plafond** (grotte, couloir symétrique, plafond
mansardé) n'avait pas d'équivalent. Ce lot ajoute cette variante en **réutilisant** l'infrastructure
posée par `LOT-22`/`LOT-23` (fonction de hauteur pure, passe de suivi après le balayage classique)
plutôt qu'en dupliquant une deuxième famille de formules ou de mécanique.

## Périmètre

### Inclus
- Quatre nouvelles tuiles : `SlopeDownRight`, `SlopeDownLeft`, `RoundedDownRight`,
  `RoundedDownLeft` — miroir vertical exact de `SlopeUpRight`/`SlopeUpLeft`/`RoundedUpRight`/
  `RoundedUpLeft` (même profil horizontal, silhouette retournée haut/bas).
- **Collision par suivi**, miroir de celle du sol : ni solides pour la grille classique
  (`core::isSolid`), ni suivies en marchant (`core::isFollowableSurface` reste `false`) — une
  passe dédiée (`core::resolveCeilingSlopeFollow`) bloque précisément un **saut** qui franchirait
  leur silhouette, sans jamais faire « marcher » le personnage dessus.
- Rendu (silhouette réelle, en gris — même matériau qu'un `Solid`, pas une couleur distincte par
  variante, cohérent avec la simplification déjà retenue pour les variantes de sol), palette de
  l'éditeur (19 types désormais), sérialisation JSON (`slopeDownRight`/`slopeDownLeft`/
  `roundedDownRight`/`roundedDownLeft`).

### Exclus (hors périmètre de ce lot)
- **Angle autre que 45°** ou **rayon différent** — hors périmètre, comme pour `LOT-22`/`LOT-23`.
- **Bloc poussable en forme de pente/arrondi** — une pente/arrondi qui se pousserait comme un
  `Block` est une mécanique à part, non demandée ici.
- **Niveau démo dédié** (`LOT-25`) — ces tuiles sont un outil d'édition/décor de plafond, pas une
  mécanique de gameplay nouvelle à prouver isolément par un niveau démo (contrairement aux pentes/
  arrondis de sol, qui rendent une progression **nécessaire** — un plafond incliné bloque un saut,
  ce que n'importe quel `Solid` fait déjà, seule sa forme diffère) ; couvertes par les tests
  unitaires/intégration décrits ci-dessous.

## Décisions de cadrage
- **Choix initial réévalué en cours de lot** : la première implémentation rendait ces tuiles
  **solides** (`core::isSolid`), avec une silhouette purement **visuelle** (collision = case pleine
  approximative). Retour en arrière complet suite à la demande explicite d'une physique **fidèle**
  à la silhouette (« ajoutons la physique qui suit le bloc ») : la collision réelle suit désormais
  le profil incliné/courbe au pixel près, comme les variantes de sol — cohérent avec le principe
  déjà établi ailleurs dans le projet (« l'affichage doit correspondre à la hitbox »), qu'une
  approximation en case pleine aurait contredit.
- **`core::resolveCeilingSlopeFollow` est un miroir exact de `core::resolveSlopeFollow`**, pas une
  nouvelle famille de règles : bord **haut** plutôt que bas, déclenché en **montant**
  (`velocityY < 0`, saut) plutôt qu'en tombant, silhouette lue via `core::ceilingSlopeHeight`
  (`1 - core::slopeSurfaceHeight` de la variante de sol miroir — aucune formule physique dupliquée).
- **Jamais suivies en marchant** (`core::isFollowableSurface` reste `false` pour les quatre
  types) : contrairement à une pente de sol, on ne « marche » jamais sous un plafond (pas de
  déplacement latéral calé dessus) — seul le fait de ne pas le traverser en sautant compte.
- **Rendu en gris uniforme**, pas une couleur distincte par variante : décision reprise de la
  correction apportée aux pentes/arrondis de sol dans la même conversation (cohérence de « matériau
  de plateforme » plutôt qu'une famille de teintes).
- **Grille de tuiles de l'atlas procédural agrandie** (`TextureAtlas::TILES_PER_SIDE` `4→5`,
  16→25 cases) pour loger les quatre nouvelles silhouettes ; le jeu de couleurs (`tileColor`) a été
  explicitement recalé pour qu'un simple agrandissement de grille ne décale **aucune** couleur des
  tuiles existantes (l'index linéaire d'une case dépend de la largeur de la grille).
- **Gap découvert après la première passe de revue** : la physique ne gérait que le franchissement
  **par en dessous** (saut bloqué contre la silhouette). Un personnage tombant sur le **dessus**
  d'une pente/arrondi de plafond (accessible depuis une zone praticable au-dessus, ex. un couloir
  entre deux niveaux) tombait au travers, faute de toute résolution pour ce cas — ni solide
  (`isSolid`), ni suivie par aucune passe existante. Corrigé en étendant `slopeSurfaceHeight`
  (physique de **sol**) pour reconnaître aussi ces quatre types, avec une hauteur **constante 0**
  (leur face du haut est toujours plate, au sommet de la case, quel que soit `localX`) : `
  resolveSlopeFollow` les prend alors en charge tel quel, sans aucun code de résolution
  supplémentaire — un personnage qui tombe dessus s'y pose désormais normalement.

## Exigences couvertes
- `EX-GP-006` — nouvelle exigence, implémentée.

## Découpage

> État : ✅ fait · 🔄 en cours · ⬜ non commencé.

| Tâche | Intitulé | Emplacement | État |
|-------|----------|-------------|:----:|
| [TACHE-01](tache-01-modele-physique-plafond.md) | Modèle de tuile et physique de suivi | `Core/Levels`, `Core/Physics`, `Core/Ecs/Systems` | ✅ |
| [TACHE-02](tache-02-editeur-rendu.md) | Éditeur et rendu | `HMI/Editor`, `HMI/Graphics` | ✅ |
| [TACHE-03](tache-03-documentation-verification.md) | Documentation et vérification | `Documentation` | ✅ |

## Critères d'acceptation du lot
1. Un saut qui franchit une pente/arrondi de plafond **par en dessous** est bloqué précisément
   selon sa silhouette (bord fin = monte plus haut, bord épais = bloqué plus tôt) — pas comme un
   carré plein uniforme.
2. Le personnage ne peut **jamais** franchir la silhouette en sautant, quelle que soit la vitesse
   d'ascension (chevauchement de plusieurs lignes en un seul pas inclus, comme pour le sol).
3. Un personnage qui tombe sur le **dessus** d'une pente/arrondi de plafond s'y pose normalement
   (face du haut plate), sans tomber au travers.
4. Aucune régression sur la physique existante (sol, pentes/arrondis de sol, murs, sauts) : tous
   les tests existants restent verts sans modification.
5. Placeables depuis la palette de l'éditeur, sérialisables (JSON), rendues avec leur silhouette
   réelle en gris.
6. Logique nouvelle **couverte par des tests** dédiés (géométrie miroir, classification statique,
   aller-retour JSON, physique de blocage par en dessous **et** de support par au-dessus). Build
   `/W4 /WX` sans avertissement, Doxygen et lint des exigences verts.

## Dépendances
- Étend `TileType` (`LOT-03`/`LOT-07`) et réutilise directement l'infrastructure de suivi de
  surface posée par `LOT-22` (`core::slopeSurfaceHeight`, `core::resolveSlopeFollow`) et étendue
  par `LOT-23` (arrondis) — aucune modification de ces fonctions existantes (hormis l'extension du
  `switch` de `slopeSurfaceHeight` pour la face du haut, voir décisions de cadrage), seulement de
  nouvelles fonctions miroir qui les appellent.

## Navigation des tâches
- @subpage lot-26-tache-01-modele-physique-plafond
- @subpage lot-26-tache-02-editeur-rendu
- @subpage lot-26-tache-03-documentation-verification
