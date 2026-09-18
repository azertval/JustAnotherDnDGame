# Cartes & format {#spec-niveaux}

> Statut : **livré**. Format JSON versionné (version 3), chargement, validation, couches, entités
> et pièces assignées par case ; le Colisée et deux quartiers de la Capitale sont livrés dans ce
> format. Dépend de [`gameplay.md`](gameplay.md).

## 1. Représentation des cartes
- \anchor EX-LVL-001 **EX-LVL-001** — Une carte doit être décrite par un **fichier de données**
  externe (pas en dur dans le code), placé dans `Source/Elements/Levels`.
- \anchor EX-LVL-002 **EX-LVL-002** — Le format doit décrire au minimum : dimensions de la grille,
  type de chaque tuile, position d'entrée, couches visibles et entités.
- \anchor EX-LVL-003 **EX-LVL-003** — Le format retenu est un **JSON structuré orienté objets** : une
  carte est un objet JSON portant ses **métadonnées** (nom, dimensions) et une **liste de tuiles**,
  chaque tuile étant un **objet** `{x, y, type, …}` (les cases vides sont omises) pouvant porter des
  **champs propres** (la pièce assignée à la case, `"texture"`). Choisi pour un format
  **extensible** (données riches par tuile, *round-trip* d'éditeur direct), au prix d'une lisibilité
  « à l'œil » moindre qu'une grille ASCII — l'édition passe par l'**éditeur**, pas par le texte brut.
- \anchor EX-LVL-005 **EX-LVL-005** — Le fichier de carte doit porter un **numéro de version de
  format**, afin qu'une évolution non rétrocompatible soit **détectée** plutôt que subie. Un fichier
  **sans** numéro de version est lu comme la version initiale, sans erreur ni avertissement ; une
  version supérieure à celle gérée est refusée avec un message explicite.
- \anchor EX-LVL-004 **EX-LVL-004** — Le chargement d'une carte doit **valider** les données
  (positions des tuiles et des entités **dans les bornes** `width × height`, une seule tuile par case,
  **une et une seule entrée**, types de tuile connus) et signaler une erreur exploitable en cas de
  fichier invalide (cf. politique d'erreurs des conventions).
- \anchor EX-LVL-016 **EX-LVL-016** — Une carte doit porter **N couches de tuiles typées** plutôt
  qu'une grille unique : un RPG en vue de dessus superpose un **sol** (herbe, dalle, eau), un
  **décor** (arbre, tonneau, tapis) et une **collision** — masque indépendant du visuel, un tapis se
  traverse et un tonneau non. La grille de **collision** d'une carte est son tableau racine `tiles`,
  celui qui porte déjà l'entrée : le tableau `layers` ne décrit que les couches **visibles**, et une
  couche de rôle `collision` qui y serait déclarée est **refusée** — deux grilles à tenir d'accord se
  désynchronisent, et c'est celle qu'on ne voit pas qui gagne. Au chargement, la grille racine est
  **promue** en couche de tête, pour que tout consommateur boucle sur les couches sans cas
  particulier. Un fichier **sans** tableau `layers` se charge, sa grille promue en couche unique dite
  *legacy*, à la fois décor et collision. Concrétisé en `LOT-04`.
- \anchor EX-LVL-017 **EX-LVL-017** — Une carte doit porter une **liste d'entités** — PNJ, coffres,
  panneaux, portails, rencontres — distincte de ses grilles : une entité est un **objet** à type
  libre, placé sur une case et porteur de ses propres données, là où une grille ne retient qu'un
  type par case. Le type n'est **pas** interprété au chargement — c'est le gameplay qui lui donne un
  sens — mais la position est validée comme celle d'une tuile (`EX-LVL-004`). Concrétisé en
  `LOT-04`.
- \anchor EX-LVL-018 **EX-LVL-018** — Une couche et une entité doivent pouvoir porter un
  **dictionnaire de propriétés libres**, et tout champ **inconnu** du chargeur doit y être rangé :
  ignoré sans erreur à la lecture, et **réémis** à l'écriture. Sans quoi le moindre besoin découvert
  plus tard — terrain difficile, couverture, hauteur, dialogue d'un PNJ — imposerait une nouvelle
  version de format et la migration de tout le contenu déjà produit. Concrétisé en `LOT-04`.

### Format retenu (JSON, liste de tuiles-objets)

Types de tuiles : `entry` (entrée, point d'arrivée par défaut), `solid` (matière pleine), et le
terrain du RPG (`EX-EXP-005`) — `grass`, `dirt`, `sand`, `water`, `deepWater` (sols ; l'eau profonde
bloque), `wall`, `cliff` (obstacles), `bridge`, `stairs` (passages). Une case **vide** n'est pas
listée (absence = vide).

Une tuile de la grille racine peut porter `"texture"` : la pièce de la planche du lieu
(`EX-VIS-008`) dessinée sur cette case, prioritaire sur la table d'apparence de son type
(`TileTextureOverride`).

```json
{
  "version": 3,
  "name": "Village",
  "width": 12,
  "height": 8,
  "tiles": [
    { "x": 1, "y": 1, "type": "entry" },
    { "x": 4, "y": 4, "type": "wall", "texture": "puits" }
  ],
  "layers": [
    { "name": "sol", "kind": "ground", "tiles": [{ "x": 4, "y": 4, "type": "dirt" }],
      "scene": "village" },
    { "name": "decor", "kind": "decor", "tiles": [{ "x": 5, "y": 4, "type": "wall" }],
      "difficultTerrain": true }
  ],
  "entities": [
    { "type": "npc", "x": 6, "y": 3, "dialogue": "bonjour" },
    { "type": "chest", "x": 2, "y": 7 }
  ]
}
```
Rôles de couche reconnus : `ground`, `decor`, et `legacy` (rôle de la grille racine promue, jamais
écrit) ; un rôle inconnu retombe sur `ground` plutôt que de faire échouer la carte (`EX-NFR-040`),
et `collision` déclaré est refusé (`EX-LVL-016`). La propriété de couche `scene` nomme le **lieu**
dont la carte porte les planches (`Assets/Scene/<lieu>/`).

**Familles d'entités posées par l'éditeur** (`LOT-11`, `EX-EDIT-050`). Le chargeur ne connaît aucun
type d'entité (`EX-NFR-040`) ; l'éditeur, lui, sait poser et renseigner ceux que le gameplay lit,
rassemblés dans `core::knownEntityKinds` (`Source/Core/World/EntityKinds.h`) :

| `type` | Propriétés | Lue par |
|---|---|---|
| `chest` | — | `core::knownInteractableKinds` (`LOT-10`) |
| `sign` | — | `core::knownInteractableKinds` (`LOT-10`) |
| `npc` | `dialogue`, `figure` (figurine de l'atelier), `guards` (quartier gardé) | `core::dialogueTriggerFor` (`LOT-15`), rendu du lieu |
| `encounter` | `encounterId` (requis), `respawns` (booléen) | `core::encounterTriggerFor` (`LOT-18`) |
| `portal` | `targetMap`, `arrival` — requis ; `requiresFlag` | graphe du monde (`LOT-09`) |
| `spawnPoint` | `name` (requis, unique dans la carte) | graphe du monde (`LOT-09`) |
| `combatZone` | `name`, `width`, `height` — requis | découpe de la grille de combat (`LOT-09`) |
| `cityBlock` | `name`, `width`, `height` — requis | plan de ville (`LOT-96`) |
| `arenaEntry` | `side` (`allies` ou `enemies`), `rank` (entier) | `core::arenaEntryPoints` (`LOT-50`) |

L'**identifiant d'une carte** est le chemin de son fichier sous `Source/Elements/Levels/`, sans
extension (`coliseum`, `capital/martpart`). Un portail désigne sa destination par `(carte, point
d'arrivée nommé)`, jamais par des coordonnées, qui se désynchroniseraient au premier
redimensionnement de la carte cible (`EX-EDIT-052`) :
```json
{ "type": "portal", "x": 11, "y": 4, "targetMap": "foret", "arrival": "lisiere-est" }
{ "type": "spawnPoint", "x": 1, "y": 4, "name": "porte-ouest" }
```

Coordonnées `x` = colonne, `y` = ligne, origine **haut-gauche** ; toute tuile hors des bornes
`width × height` est invalide.

## 2. Conception (lignes directrices)
- Chaque carte doit être **franchissable** : aucune zone jouable ne doit être inatteignable.
- Aucune situation sans issue : un portail mène toujours quelque part, et l'on peut revenir.
- Toute carte doit être un **terrain tactique valide** (`EX-EDIT-054`) : le combat se joue dessus.

## Exigences retirées {#lvl-retirees}

> Ancres conservées, jamais renumérotées : les lots livrés s'y réfèrent. Les six premières
> décrivaient une **campagne** de tableaux ordonnés ; le jeu est un bac à sable, relié par le
> **graphe de cartes** du `LOT-09` et retenu par la **sauvegarde** du `LOT-17`.

- \anchor EX-LVL-010 **EX-LVL-010** *(retirée en `LOT-67`)* — ordre de chargement des niveaux.
- \anchor EX-LVL-011 **EX-LVL-011** *(retirée en `LOT-67`)* — enchaînement automatique des niveaux.
- \anchor EX-LVL-012 **EX-LVL-012** *(retirée en `LOT-67`)* — niveaux de démonstration à difficulté
  croissante.
- \anchor EX-LVL-013 **EX-LVL-013** *(retirée en `LOT-67`)* — séquence de niveaux en donnée de
  contenu.
- \anchor EX-LVL-014 **EX-LVL-014** *(retirée en `LOT-67`, remplacée par la sauvegarde du `LOT-17`)*
  — progression par tableau.
- \anchor EX-LVL-015 **EX-LVL-015** *(retirée en `LOT-67`, reprise par le `LOT-49`)* — couverture de
  toutes les mécaniques par le contenu livré.
- \anchor EX-LVL-006 **EX-LVL-006** *(retirée au `LOT-88`)* — mode de cadrage de caméra par niveau.
- \anchor EX-LVL-007 **EX-LVL-007** *(retirée au `LOT-88`)* — zones de caméra dessinées à la main.
- \anchor EX-LVL-008 **EX-LVL-008** *(retirée au `LOT-88`)* — route des plateformes mobiles et
  capacités par niveau.
- \anchor EX-LVL-009 **EX-LVL-009** *(retirée au `LOT-88`)* — liste des plans picturaux et
  parallaxe.

## Traçabilité
Le chargement et la validation relèvent de `Source/Core` (`core::LevelLoader`,
`core::LevelWriter`) ; les fichiers de cartes sont dans `Source/Elements/Levels`. Types de tuiles :
[`gameplay.md`](gameplay.md) ; exploration : [`exploration.md`](exploration.md).
