# Lots {#lots}

Le travail est découpé en **lots** (un incrément livrable par lot). Un lot **démarré** reçoit son
sous-dossier `LOT-XX-nom/` avec un `epic.md` (objectif, périmètre, critères d'acceptation) ; les
lots **à venir** vivent dans la [feuille de route](@ref roadmap-010), qui est leur unique source de
vérité. Les lots référencent les [spécifications](@ref specifications) via les identifiants
d'exigences `EX-…`.

> **Pourquoi les lots à venir n'ont pas de dossier.** Ils en avaient un, vide de tout travail et
> écrit contre des familles d'exigences qui n'existent pas encore. Deux documents décrivant le même
> programme divergent — ils avaient déjà commencé. Un dossier se crée désormais **au démarrage** du
> lot, pas des années avant.

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

## Feuille de route

- @subpage roadmap-010 — le programme complet : ce que le corpus
  `Documentation/SourceBook/` permet d'en tirer, les lots `LOT-08` à `LOT-76`, et l'audit qui
  a confronté le tout à l'état réel du dépôt.

## Programme

La cible est un **bac à sable** dans l'univers complet de Tanares : dix régions jouables, treize
espèces, seize classes, cent-soixante-seize créatures. Le *vertical slice* du `LOT-27` — « un
personnage explore une carte top-down, parle à un PNJ, déclenche une rencontre, gagne un combat
tactique au d20 » — reste le jalon qui prouve la boucle avant qu'on la généralise.

| Phase | Lots | Objet |
|---|---|---|
| A — Fondation | `LOT-01` → `LOT-05` | Fork et purge, bibliothèque `HmiLib`, agrégat `LevelData`, format de niveau multi-couches, modes de jeu |
| B — Exploration | `LOT-06` → `LOT-11` | Déplacement top-down 8 directions, tri par Y, vocabulaire de tuiles RPG, graphe de cartes, entités et interaction, éditeur |
| C — Noyau RPG | `LOT-12` → `LOT-17` | Dés et jets d20, fiche de personnage, inventaire, PNJ et dialogues, quêtes, sauvegarde |
| D — Combat tactique | `LOT-18` → `LOT-24` | Bascule exploration ↔ combat, grille tactique, initiative, attaques, ligne de vue, IA, IHM de combat |
| E — Contenu et finition | `LOT-25` → `LOT-29` | Sorts, économie, contenu du slice, audio et version `0.2.0`, groupe de quatre personnages |
| F — Filière contenu | `LOT-30` → `LOT-76` | Extraction du corpus, catalogues de données, monde et peuplement, une classe par lot, refonte IHM et éditeur, mécaniques manquantes |

La phase F **ne suit pas** les précédentes : elle s'entrelace avec B à E, chacun de ses lots servant
un lot des phases antérieures qui, sans lui, se bâtirait sur un catalogue fictif. L'ordre
d'exécution recommandé est donné par la [feuille de route](@ref roadmap-010), §6.

Décisions de cadrage actées avant le `LOT-01` : règles **d20 maison** (compatible SRD dans sa
structure, sans en dépendre), combat **sur la carte d'exploration** (grille dérivée de la couche
collision), **un héros au départ et quatre à terme** (rien ne suppose l'unicité), échelle
**1 case = 1,5 m**.
