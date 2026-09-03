# LOT-32 — Niveaux à salles (façon Celeste) {#lot-32}

> Statut : **terminé**.

## Objectif
`LOT-16` a rendu les grands niveaux **visibles** (caméra qui zoome pour englober le niveau
entier), mais au prix d'un zoom qui rapetisse d'autant plus que le niveau grandit — acceptable
jusqu'à un plafond raisonnable (`EX-EDIT-017`), pas au-delà. Pour de **très grands** niveaux, ce
lot introduit une seconde stratégie de cadrage, façon *Celeste* : le niveau reste une **grille de
tuiles unique** (aucun changement de format, aucune limite dans `Core`), mais elle est
**partitionnée** en un tableau de **salles** de taille fixe pour la caméra. En jeu, la caméra ne
cadre plus tout le niveau mais **une salle à la fois**, au zoom pixel art natif — elle bascule
**nettement** sur la salle voisine quand le personnage en franchit la frontière, sans jamais
rapetisser le rendu quelle que soit la taille totale du niveau. Le niveau dans son ensemble garde
**une seule entrée et une seule sortie** (invariant `Core` inchangé, `EX-LVL-004`) ; une salle
« intérieure » peut en revanche donner sur **plusieurs salles voisines** (haut/bas/gauche/droite),
dès lors qu'un couloir reste ouvert sur le bord correspondant — aucune nouvelle tuile ni notion de
porte inter-salle : la géométrie déjà peinte suffit.

## Périmètre

### Inclus
- **Partition en salles** (`RoomGrid`, nouvelle logique pure) : découpe une grille de tuiles en un
  tableau de rectangles de taille fixe (constante de code, dernière ligne/colonne éventuellement
  plus petite si la taille du niveau n'est pas un multiple exact — jamais rognée, toujours
  entièrement visible). Détermine la salle contenant une position donnée.
- **Caméra par salle en jeu** (`GameScreen`) : remplace le cadrage « niveau entier » (`LOT-16`) par
  un cadrage sur la **salle courante** du personnage — zoom via `Camera2D::fitZoom` appliqué au
  rectangle de la salle (au lieu du niveau entier), centre recalculé **uniquement** quand la salle
  change (coupure nette, pas de scroll progressif). Un niveau qui tient dans une seule salle se
  comporte **à l'identique** d'avant ce lot (une seule salle = le niveau entier).
- **Repère visuel dans l'éditeur** (`EditorScreen`) : lignes de grille délimitant les salles,
  superposées à la grille de tuiles, pour guider l'alignement des couloirs aux frontières de
  salles. La caméra de l'éditeur (pan/zoom manuel, `LOT-15`/`LOT-16`) **ne change pas** : un
  level designer continue de voir (et d'éditer) le niveau entier pendant la conception.
- **Niveau de démonstration** : au moins `2×2` salles, avec une salle intermédiaire ouverte sur
  **plusieurs** voisines, intégré à la séquence de jeu.
- Documentation (guide de rendu/éditeur) et mise à jour des spécifications concernées.

### Exclus (hors périmètre de ce lot)
- **Salles comme entités séparées** (fichiers/`TileMap` distincts reliés par une tuile
  « connecteur » explicite) — écarté au profit d'une grille unique partitionnée : aucun nouveau
  format de fichier, aucune nouvelle tuile, aucune résolution de liaison au chargement (cf.
  décisions de cadrage). Un lot futur pourrait revisiter ce choix si le besoin de salles
  *réellement* indépendantes (rejouables séparément, générées procéduralement, etc.) apparaît.
- **Caméra suiveuse en continu, bornée à la salle** (zone morte/marge) — même exclusion que
  `LOT-16` : plus de complexité (zone morte, bornage, risque de saccade) pour un gain non demandé ;
  la coupure nette façon *Celeste* est le comportement retenu.
- **Salles de taille variable définies niveau par niveau** — une seule constante de taille de salle
  pour tout le jeu (façon écran fixe), pas un champ éditable par niveau : cohérent avec le plafond
  de taille de `LOT-16` (`EX-EDIT-017`, « configurable au niveau du code, pas une limite arbitraire
  exposée »).
- **Transitions animées** (fondu, glissement) entre salles — coupure nette uniquement ; une
  transition animée est un raffinement de *feel* pour un lot ultérieur si souhaité.
- **Édition du découpage en salles** (déplacer/redimensionner une salle indépendamment de la
  grille) — les salles sont une **projection** dérivée de la taille du niveau et de la constante de
  taille de salle, pas un objet éditable ; agrandir/rétrécir le niveau (`EX-EDIT-017`, déjà livré)
  suffit à ajouter ou retirer des salles.
- **Décors, pipeline photo → pixel art** — inchangé, toujours hors périmètre (`EX-DEC-*`).

## Décisions de cadrage
- **Une grille unique partitionnée, pas des salles séparées reliées par connecteurs** : tranché
  avec le demandeur. `Core` n'a aucune limite de taille depuis `LOT-16` et l'invariant « une entrée,
  une sortie » (`EX-LVL-004`) porte déjà sur le niveau entier — le réutiliser tel quel évite tout
  nouveau format, toute nouvelle tuile, toute résolution de liaison inter-salles. Une salle a
  « plusieurs entrées/sorties » simplement parce qu'un couloir reste ouvert sur plusieurs bords vers
  des salles voisines dans la même grille — propriété **géométrique**, pas un mécanisme à modéliser.
- **Caméra à coupure nette par salle, pas de scroll continu borné** : tranché avec le demandeur,
  même raisonnement que `LOT-16` (caméra suiveuse explicitement écartée) — aucune zone morte ni
  bornage aux limites à concevoir, comportement prévisible façon *Celeste*.
- **La partition en salles vit dans `HMI`, pas dans `Core`** : comme le plafond de taille de
  `LOT-16` (`EX-EDIT-017`), c'est un découpage d'**usage** (cadrage caméra) sans notion de règle de
  jeu — `Core`/`TileMap`/`Level` restent une grille plate, sans notion de salle (`EX-NFR-010`).
- **La caméra de l'éditeur reste « niveau entier »**, seule celle du jeu passe en cadrage par salle.
  Un level designer construit et vérifie la cohérence d'ensemble (couloirs alignés, entrée/sortie
  atteignables) en voyant tout le niveau à la fois ; le cadrage par salle est une expérience de
  **jeu**, pas un besoin d'édition. Un simple quadrillage superposé suffit à visualiser les
  frontières de salles sans changer le pan/zoom existant.
- **`RoomGrid` réutilise `Camera2D::fitZoom`** (LOT-16) tel quel, appliqué au rectangle de la salle
  courante plutôt qu'au niveau entier — aucune duplication de la règle de zoom (entier tant que
  `≥ 1`, fractionnaire seulement si nécessaire).

## Exigences couvertes
- Nouvelle : `EX-REN-015` (cadrage caméra par salle, jeu), `EX-EDIT-023` (repère visuel de salles,
  éditeur).
- Corrigée (portée, pas le sens) : `EX-REN-013` (cadrage « niveau entier » scopé aux niveaux tenant
  dans une seule salle ; au-delà, `EX-REN-015` prend le relais).
- Réutilisées (inchangées) : `EX-LVL-004` (une entrée/une sortie, invariant niveau entier),
  `EX-EDIT-017` (plafond de taille, toujours applicable au niveau entier), `EX-ARCH-022` (zoom
  pixel art de préférence entier), `EX-NFR-010` (logique pure côté `Core`/logique de partition).

## Découpage

> État : ✅ fait · 🔄 en cours · ⬜ non commencé.

| Tâche | Intitulé | Emplacement | État |
|-------|----------|-------------|:----:|
| [TACHE-01](tache-01-partition-salles.md) | Partition en salles (`RoomGrid`, logique pure) | `HMI/Graphics` | ✅ |
| [TACHE-02](tache-02-camera-salle-jeu.md) | Caméra par salle en jeu (coupure nette) | `HMI/Interface` | ✅ |
| [TACHE-03](tache-03-repere-editeur.md) | Repère visuel de salles dans l'éditeur | `HMI/Interface` | ✅ |
| [TACHE-04](tache-04-demo-documentation-verification.md) | Niveau de démonstration, documentation et vérification | `Source/Elements`, `Documentation` | ✅ |

## Critères d'acceptation du lot
1. Un niveau plus grand qu'une salle se joue avec une caméra au **zoom pixel art natif** (jamais
   rapetissie pour « tout englober ») : elle cadre la salle du personnage, et bascule **nettement**
   sur la salle voisine dès que le personnage en franchit la frontière, dans les quatre directions.
2. Un niveau qui tient dans une seule salle se comporte **à l'identique** d'avant ce lot (même zoom
   entier, même cadrage) — non-régression sur tous les niveaux livrés à ce jour.
3. Le niveau garde **une seule entrée et une seule sortie** au global (`EX-LVL-004`, inchangé) ;
   aucune nouvelle notion d'entrée/sortie **par salle** n'est introduite dans `Core`.
4. Une salle intermédiaire du niveau de démonstration est accessible depuis **plusieurs** salles
   voisines (au moins deux bords ouverts), sans aucune tuile ni configuration dédiée au-delà de la
   géométrie peinte.
5. Dans l'éditeur, le quadrillage des salles est visible en superposition de la grille de tuiles,
   sans changer le comportement de pan/zoom existant (`EX-EDIT-013`).
6. Logique nouvelle **couverte par des tests** (`ctest` vert), déterministe, sans GPU pour la
   partition (`RoomGrid`) ; vérification visuelle pour l'intégration caméra/éditeur (dépendance
   D3D11). Build `/W4 /WX` sans avertissement, Doxygen (binaire CI 1.9.8 vérifié localement avant
   push) et lint des exigences verts.

## Dépendances
- Étend `hmi::Camera2D::fitZoom` (`LOT-16`) et le cadrage caméra de `GameScreen` (`LOT-05`/`09`),
  sans modifier `core::TileMap`/`core::Level`/`core::LevelDraft` (`LOT-07`/`14`) ni l'invariant
  entrée/sortie (`EX-LVL-004`). Le repère visuel de l'éditeur s'appuie sur `EditorScreen::renderGrid`
  (`LOT-14`/`15`/`16`).

## Navigation des tâches
- @subpage lot-32-tache-01-partition-salles
- @subpage lot-32-tache-02-camera-salle-jeu
- @subpage lot-32-tache-03-repere-editeur
- @subpage lot-32-tache-04-demo-documentation-verification
