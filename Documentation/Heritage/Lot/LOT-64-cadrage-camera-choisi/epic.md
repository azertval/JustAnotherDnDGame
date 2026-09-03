# LOT-64 — Cadrage de caméra choisi par le level designer {#lot-64}

> Statut : **fait** (vérification automatisée : build `/W4 /WX`, 1114 tests `ctest` à 100 %, lint
> d'exigences, cahier de test régénéré, Doxygen vert sur la version épinglée par la CI ; l'essai
> manuel des trois modes et des zones de caméra reste à faire avant fusion, voir tache-04). Prérequis :
> [LOT-32](@ref lot-32) (partition en salles),
> [LOT-16](@ref lot-16) (caméra niveau entier), [LOT-57](@ref lot-57) (architecture de
> l'information de l'éditeur).

## Objectif
Rendre au level designer le **choix du cadrage**, aujourd'hui entièrement décidé par le moteur.

Le comportement actuel est une règle unique, en dur, et invisible depuis l'éditeur : si le niveau
tient dans une **salle** (`RoomGrid::ROOM_WIDTH_TILES` × `ROOM_HEIGHT_TILES`, soit 24 × 14 tuiles,
deux **constantes de compilation**), la caméra cadre le niveau entier (`LOT-16`) ; au-delà, elle
bascule salle par salle avec une coupure nette (`EX-REN-015`, `LOT-32`). Le designer ne choisit
rien : il **subit** une bascule de comportement au moment où son niveau franchit 24 tuiles de large.

Deux conséquences.

1. **Le moteur n'a aucune caméra de suivi.** C'est le cadrage le plus répandu du genre — la caméra
   accompagne le personnage, avec une zone morte et une anticipation — et il n'existe nulle part.
   Un niveau large et bas (une course, une poursuite) n'a aujourd'hui qu'un mauvais choix : subir
   des coupures de salle qui n'ont aucun sens sur un couloir horizontal.
2. **Le cadrage n'est pas une propriété du niveau.** Il devrait l'être : c'est une décision de
   **conception**, au même titre que le placement des dangers. Un tableau de puzzle veut voir la
   salle entière ; un tableau d'adresse veut suivre le personnage.

## Périmètre

### Inclus
- **Mode de cadrage porté par le niveau** (`EX-LVL-006`), donnée sérialisée et validée comme le
  reste du format.
- **Trois modes** : *niveau entier*, *par salle* (comportement actuel), *suivi du personnage*.
- **Caméra de suivi** (`EX-REN-016`) : zone morte, anticipation dans le sens du déplacement,
  bornage aux limites du niveau, lissage — le mode qui manque au moteur.
- **Taille de salle réglable par niveau** plutôt que constante de compilation, quand le mode *par
  salle* est retenu.
- **Choix et prévisualisation dans l'éditeur** (`EX-EDIT-028`) : le designer voit le cadrage qu'il
  choisit, sans lancer l'essai.
- **Repli compatible** : un niveau sans mode déclaré conserve **exactement** le comportement
  actuel.

- **Zones de caméra dessinées à la main** (`EX-LVL-007`, `EX-EDIT-029`) en mode *par salle* : le
  designer peut dessiner des rectangles de taille arbitraire directement sur le canevas plutôt que
  de subir la grille automatique uniforme, ce qui permet de **mélanger plusieurs tailles de
  caméra au sein d'un même niveau**. Priorité par ordre d'ajout en cas de chevauchement, repli sur
  le niveau entier si aucune zone ne couvre le personnage.
- **Taille de la caméra de suivi réglable par niveau** (`EX-REN-017`), mêmes champs que la taille
  de salle du mode *par salle*.

### Exclus (hors périmètre de ce lot)
- Transitions animées entre zones de caméra, déclencheurs de caméra (objets posés dans le niveau
  qui changeraient le cadrage à l'exécution) : les zones restent une **donnée statique** du
  niveau, pas une couche d'objets à éditer ni un système scripté.
- Effets de caméra (secousse, zoom scénarisé) — la secousse relève du [LOT-53](@ref lot-53).
- Caméra libre en jeu, mode photo.

## Décisions de cadrage
- **Le mode est une donnée du niveau, pas un réglage d'application.** Un réglage global ferait du
  cadrage une préférence de joueur alors que c'est une décision de conception : un tableau de
  puzzle conçu pour être vu en entier devient injouable en caméra suivi.
- **Le comportement actuel reste le défaut.** Un niveau existant, sans mode déclaré, se joue
  exactement comme aujourd'hui — quinze tableaux livrés et le test système en dépendent. La
  compatibilité n'est pas une option de repli : c'est le critère d'acceptation numéro un.
- **La caméra de suivi doit être bornée aux limites du niveau.** Une caméra qui déborde montre le
  vide hors grille, ce que ni le fond ni les décors ne couvrent — le défaut le plus visible de
  cette famille.
- **La caméra n'affecte jamais la simulation** (`EX-ARCH-012`). Elle lit la position interpolée pour
  l'affichage ; le zoom pixel art reste **entier** (`EX-ARCH-022`) sous peine de ruiner la netteté
  que tout le projet protège depuis le `LOT-05`.
- **Trois modes, pas un système de caméra scripté.** L'édition de zones reste une simple liste de
  rectangles statiques attachée au niveau (repli sur le niveau entier hors de toute zone, priorité
  par ordre d'ajout) — sans transition animée ni déclencheur. Ouvrir ce dernier point
  transformerait le lot en programme ; le besoin constaté (mélanger des tailles de caméra dans un
  même niveau) ne le demande pas.
- **Les zones de caméra et la taille réglable du mode suivi ont été intégrées en cours de lot**,
  suite à un besoin exprimé par le level designer une fois les trois modes de base livrés :
  mélanger plusieurs tailles de caméra dans un même niveau (mode *par salle*) était impossible tant
  que la seule granularité était une grille uniforme par niveau. Choix délibéré : les intégrer
  **dans ce lot** plutôt que d'ouvrir un nouveau lot après fusion, pour livrer une fonctionnalité de
  cadrage cohérente et complète en une seule fois plutôt qu'en deux passes sur le même code.
- **Ce lot précède [LOT-66](@ref lot-66)** : refondre les niveaux de démonstration avant que le
  choix de cadrage existe obligerait à les reprendre deux fois.

## Exigences couvertes
- Nouvelles : `EX-LVL-006` (le niveau porte son mode de cadrage), `EX-REN-016` (trois modes de
  cadrage, dont la caméra de suivi bornée), `EX-EDIT-028` (choix et prévisualisation dans
  l'éditeur), `EX-LVL-007` (zones de caméra dessinées à la main), `EX-REN-017` (taille de suivi
  réglable), `EX-EDIT-029` (outil de dessin et tableau des zones dans l'éditeur).
- Réutilisées : `EX-REN-013` (caméra 2D, zoom pixel art natif), `EX-REN-015` (cadrage par salle —
  devient l'un des trois modes), `EX-ARCH-012` (rendu sans effet sur la simulation), `EX-ARCH-022`
  (échantillonnage *nearest*, zoom entier), `EX-LVL-004`/`EX-LVL-005` (validation, version de
  format), `EX-NFR-004` (vérification sans GPU), `EX-NFR-040` (erreur récupérable), `EX-EDIT-005`
  (annulation), `EX-EDIT-010` (réutilisation du modèle de `Core`).

## Découpage

> État : ✅ fait · 🔄 en cours · ⬜ non commencé.

| Tâche | Intitulé | Emplacement | État |
|-------|----------|-------------|:----:|
| [TACHE-01](tache-01-mode-cadrage-format.md) | Mode de cadrage dans le format de niveau, validation et repli compatible | `Source/Core/Levels` | ✅ |
| [TACHE-02](tache-02-camera-suivi.md) | Caméra de suivi : zone morte, anticipation, bornage | `Source/HMI/Graphics` | ✅ |
| [TACHE-03](tache-03-choix-previsualisation-editeur.md) | Choix et prévisualisation du cadrage dans l'éditeur | `Source/HMI/Editor` | ✅ |
| [TACHE-04](tache-04-documentation-verification.md) | Documentation et vérification | `Source/Test`, `Documentation` | ✅ |
| [TACHE-05](tache-05-zones-camera-taille-suivi.md) | Zones de caméra dessinées à la main et taille de suivi réglable | `Source/Core/Levels`, `Source/HMI/{Graphics,Editor,Game}` | ✅ |

## Critères d'acceptation du lot
1. Un niveau **sans** mode déclaré se joue exactement comme aujourd'hui — les quinze tableaux
   livrés et le test système sont inchangés.
2. Les trois modes sont sélectionnables par niveau, enregistrés, rechargés et validés.
3. La caméra de suivi accompagne le personnage sans jamais déborder des limites du niveau, et sans
   à-coup à l'inversion du sens de déplacement.
4. Le zoom reste **entier** dans les trois modes : aucun pixel n'est interpolé.
5. Le designer voit le cadrage choisi depuis l'éditeur, sans lancer l'essai.
6. Le cadrage n'a **aucun** effet sur la simulation : tests de gameplay et de franchissabilité
   inchangés.
7. En mode *par salle*, le designer peut dessiner des zones de caméra de tailles différentes dans
   un même niveau ; sans zone dessinée, la grille automatique reste inchangée.
8. La taille de la caméra de suivi est réglable par niveau, avec repli sur la taille par défaut
   actuelle si non déclarée.
9. Build `/W4 /WX`, `ctest` à 100 %, Doxygen et lints verts.

## Dépendances
Bâtit sur [LOT-32](@ref lot-32) (partition en salles, `hmi::RoomGrid`), [LOT-16](@ref lot-16)
(caméra niveau entier) et [LOT-05](@ref lot-05) (`hmi::Camera2D`). S'intègre à l'éditeur livré par
[LOT-57](@ref lot-57). Prérequis de [LOT-66](@ref lot-66), qui choisira un cadrage par tableau.

## Navigation des tâches
- @subpage lot-64-tache-01-mode-cadrage-format
- @subpage lot-64-tache-02-camera-suivi
- @subpage lot-64-tache-03-choix-previsualisation-editeur
- @subpage lot-64-tache-04-documentation-verification
- @subpage lot-64-tache-05-zones-camera-taille-suivi
