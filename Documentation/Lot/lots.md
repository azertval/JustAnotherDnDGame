# Lots {#lots}

Le travail est découpé en **lots** (un incrément livrable par lot), chacun dans
un sous-dossier `LOT-XX-nom/` contenant un `epic.md` (objectif, périmètre,
critères d'acceptation) et des fichiers `tache-NN.md` (une unité de travail
chacun). Les lots référencent les [spécifications](@ref specifications) via les
identifiants d'exigences `EX-…`.

Contrairement aux spécifications, les lots **conservent** leur numéro (`LOT-XX`) :
c'est un identifiant stable, jamais réordonné.

> **Numérotation repartie à `LOT-01`.** Ce dépôt est dérivé de `ProjectGaming` (jeu de plateforme
> livré en `0.1.3` après 74 lots). Son programme de lots est archivé en lecture seule sous
> `Documentation/Heritage/Lot/` et n'est plus référencé ici : les deux ensembles ne se croisant
> jamais, un `LOT-XX` de cette page désigne sans ambiguïté un lot du RPG.

## Lots

- @subpage lot-01
- @subpage lot-02
- @subpage lot-03
- @subpage lot-04
- @subpage lot-05
- @subpage lot-06
- @subpage lot-07
- @subpage lot-08
- @subpage lot-09
- @subpage lot-10
- @subpage lot-11
- @subpage lot-12
- @subpage lot-13
- @subpage lot-14
- @subpage lot-15
- @subpage lot-16
- @subpage lot-17
- @subpage lot-18
- @subpage lot-19
- @subpage lot-20
- @subpage lot-21
- @subpage lot-22
- @subpage lot-23
- @subpage lot-24
- @subpage lot-25
- @subpage lot-26
- @subpage lot-27
- @subpage lot-28
- @subpage lot-29

## Matière première

- @subpage corpus-sources — analyse des huit PDF de `Documentation/SourceBook/` (contenu,
  faisabilité de l'extraction, licences en sommeil) et **programme de lots proposé** pour en tirer
  les catalogues JSON qu'exigent les `LOT-13`, `LOT-14`, `LOT-25` et `LOT-26`, ainsi que les
  textures et l'habillage d'interface.

## Programme

La feuille de route complète va du socle technique au *vertical slice* jouable — « un personnage
explore une carte top-down, parle à un PNJ, déclenche une rencontre, gagne un combat tactique au
d20 » — en cinq phases :

| Phase | Lots | Objet |
|---|---|---|
| A — Fondation | `LOT-01` → `LOT-05` | Fork et purge, bibliothèque `HmiLib`, agrégat `LevelData`, format de niveau multi-couches, modes de jeu |
| B — Exploration | `LOT-06` → `LOT-11` | Déplacement top-down 8 directions, tri par Y, vocabulaire de tuiles RPG, graphe de cartes, entités et interaction, éditeur multi-couches |
| C — Noyau RPG | `LOT-12` → `LOT-17` | Dés et jets d20, fiche de personnage, inventaire, PNJ et dialogues, quêtes, sauvegarde |
| D — Combat tactique | `LOT-18` → `LOT-24` | Bascule exploration ↔ combat, grille tactique, initiative, attaques, ligne de vue, IA, IHM de combat |
| E — Contenu et finition | `LOT-25` → `LOT-29` | Sorts, économie, contenu du slice, audio et version `0.2.0`, groupe de quatre personnages |

Décisions de cadrage actées avant le `LOT-01` : règles **d20 maison** (compatible SRD dans sa
structure, sans en dépendre), combat **sur la carte d'exploration** (grille dérivée de la couche
collision), **un héros au départ et quatre à terme** (rien ne suppose l'unicité), échelle
**1 case = 1,5 m**.
