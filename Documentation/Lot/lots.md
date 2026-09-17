# Lots {#lots}

Le travail est découpé en **lots** (un incrément livrable par lot). Un lot **démarré** reçoit son
sous-dossier `LOT-XX-nom/` avec un `epic.md` (objectif, périmètre, critères d'acceptation) ; les
lots **à venir** vivent dans la [feuille de route](@ref roadmap), qui est leur unique source de
vérité. Les lots référencent les [spécifications](@ref specifications) via les identifiants
d'exigences `EX-…`.

> **Pourquoi les lots à venir n'ont pas de dossier.** Ils en avaient un, vide de tout travail et
> écrit contre des familles d'exigences qui n'existent pas encore. Deux documents décrivant le même
> programme divergent — ils avaient déjà commencé. Un dossier se crée désormais **au démarrage** du
> lot, pas des années avant.

Contrairement aux spécifications, les lots **conservent** leur numéro (`LOT-XX`) :
c'est un identifiant stable, jamais réordonné.

> **Numérotation repartie à `LOT-01`, et les deux plages se recouvrent désormais.** Ce dépôt est
> dérivé de `ProjectGaming` (jeu de plateforme livré en `0.1.3` après 74 lots). Son programme de
> lots est archivé en lecture seule sous `Documentation/Heritage/Lot/`.
>
> On a longtemps pu écrire que les deux ensembles « ne se croisaient jamais ». **Ce n'est plus
> vrai** : le programme atteint `LOT-87` et recouvre entièrement la plage héritée. Un renvoi au
> programme hérité s'écrit donc `LOT-H-XX` — le `LOT-78` a préfixé les 201 renvois des
> spécifications, et `scripts/lint_lots.py` refuse tout `LOT-XX` de spécification qui ne désigne
> pas un lot de ce programme.

## Lots

- @subpage lot-01
- @subpage lot-02
- @subpage lot-03
- @subpage lot-04
- @subpage lot-05
- @subpage lot-06
- @subpage lot-07
- @subpage lot-08
- @subpage lot-77
- @subpage lot-78
- @subpage lot-79
- @subpage lot-30
- @subpage lot-32
- @subpage lot-12
- @subpage lot-43
- @subpage lot-33
- @subpage lot-36
- @subpage lot-13
- @subpage lot-10
- @subpage lot-34
- @subpage lot-66
- @subpage lot-37
- @subpage lot-76
- @subpage lot-67
- @subpage lot-68
- @subpage lot-38
- @subpage lot-14
- @subpage lot-18
- @subpage lot-39
- @subpage lot-86
- @subpage lot-87
- @subpage lot-19
- @subpage lot-20
- @subpage lot-50
- @subpage lot-15
- @subpage lot-21
- @subpage lot-22
- @subpage lot-94
- @subpage lot-09
- @subpage lot-91 — hors feuille de route : chantier long, alimenté au fil de l'eau

## Feuille de route

- @subpage roadmap — le programme complet du bac à sable de Tanares : ce que le corpus
  `Documentation/SourceBook/` permet d'en tirer, les lots `LOT-09` à `LOT-96` par jalon de version,
  et les audits qui ont confronté le tout au dépôt et aux livres.

## Programme

La cible est un **bac à sable** dans l'univers complet de Tanares : treize régions jouables, treize
espèces, seize classes, cent-soixante-seize créatures. Le *vertical slice* du `LOT-27` — « un
personnage explore une carte top-down, parle à un PNJ, déclenche une rencontre, gagne un combat
tactique au d20 » — reste le jalon qui prouve la boucle avant qu'on la généralise.

| Phase | Lots | Objet |
|---|---|---|
| A — Fondation | `LOT-01` → `LOT-05` | Fork et purge, bibliothèque `HmiLib`, agrégat `LevelData`, format de niveau multi-couches, modes de jeu |
| B — Exploration | `LOT-06` → `LOT-11` | Déplacement top-down 8 directions, tri par Y, vocabulaire de tuiles RPG, graphe de cartes, entités et interaction, éditeur |
| C — Noyau RPG | `LOT-12` → `LOT-17` | Dés et jets d20, fiche de personnage, inventaire, PNJ et dialogues, quêtes, sauvegarde |
| D — Combat tactique | `LOT-18` → `LOT-24` | Bascule exploration ↔ combat, grille tactique, initiative, attaques, ligne de vue, IA, IHM de combat |
| E — Contenu et finition | `LOT-25` → `LOT-29` | Sorts, économie, contenu du slice, audio et version `0.0.1` (le slice ; la `0.1.0` est le bac à sable complet), groupe de quatre personnages |
| F — Filière contenu | `LOT-30` → `LOT-84` | Préconditions (spécification RPG, numéros, chargement de données), extraction du corpus, catalogues, monde et peuplement, une classe par lot, refonte IHM, mécaniques manquantes |
| G — Refonte de l'interface | `LOT-85` → `LOT-87` | Décidés hors de la feuille de route : `LOT-85` abandonné, `LOT-86` (Qt Quick, deux exécutables) et `LOT-87` (charte v2) livrés |
| H — Relecture des livres | `LOT-88` → `LOT-90` | Ajoutés au second audit : retrait de l'héritage plateforme des spécifications, dons et objets magiques de Tanares, le plan pénombral |
| I — Chantiers au fil de l'eau | `LOT-91` | Hors feuille de route et hors jalon : l'atelier des PNJ, la méthode qui produit une à une les figurines des 160 fiches du *Character Compendium*, au rythme du budget de génération |
| J — Assets propres au jeu | `LOT-92` → `LOT-96` | Ajoutés le 16 septembre 2026 à la relecture du plan de la Capitale : l'atelier des textures (style de scène par maquette, livré), l'atelier des monstres, les images du corpus hors du dépôt et l'écran « Carte » sur les cartes peintes par l'auteur (`LOT-94`, livré, qui absorbe le `LOT-95`, le plan de la Capitale), les quartiers scindés du `LOT-09` |

La filière (phases F, H et J) compte **39 lots à venir** — cinq numéros (`LOT-31`, `LOT-48`, `LOT-71`, `LOT-73`, `LOT-95`)
ayant été retirés par fusion et n'étant pas réattribués, et dix-huit de ses lots étant livrés, dont
ses trois préconditions (`LOT-77`, `LOT-78`, `LOT-79`). Elle **ne suit pas** les précédentes : elle
s'entrelace avec B à E, chacun de ses lots servant un lot des phases antérieures qui, sans lui, se
bâtirait sur un catalogue fictif. L'ordre d'exécution recommandé, le chemin critique jusqu'au
`LOT-27` et le graphe complet des dépendances sont donnés par la
[feuille de route](@ref roadmap), §6 — et vérifiés en CI par `scripts/lint_lots.py`.

Décisions de cadrage actées avant le `LOT-01` : règles **d20 maison** (compatible SRD dans sa
structure, sans en dépendre), combat **sur la carte d'exploration** (grille dérivée de la couche
collision), **un héros au départ et quatre à terme** (rien ne suppose l'unicité), échelle
**1 case = 1,5 m**.
