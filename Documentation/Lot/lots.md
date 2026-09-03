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
