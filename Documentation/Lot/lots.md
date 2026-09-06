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

> **Numérotation repartie à `LOT-01`, et les deux plages se recouvrent désormais.** Ce dépôt est
> dérivé de `ProjectGaming` (jeu de plateforme livré en `0.1.3` après 74 lots). Son programme de
> lots est archivé en lecture seule sous `Documentation/Heritage/Lot/`.
>
> On a longtemps pu écrire que les deux ensembles « ne se croisaient jamais ». **Ce n'est plus
> vrai** : la feuille de route atteint `LOT-84` et recouvre entièrement la plage héritée. Les
> spécifications portent **208 renvois `LOT-NN` ambigus dans douze fichiers**, que ni le lint ni
> Doxygen ne signalent. Le `LOT-78` les classe et préfixe les renvois hérités en `LOT-H-XX` ; il
> doit précéder la création du premier dossier de lot au-delà du `LOT-29`. D'ici là, un `LOT-XX`
> cité dans une spécification **peut désigner l'un ou l'autre programme**.

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

## Feuille de route

- @subpage roadmap-010 — le programme complet : ce que le corpus
  `Documentation/SourceBook/` permet d'en tirer, les lots `LOT-08` à `LOT-84`, et l'audit qui
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
| F — Filière contenu | `LOT-30` → `LOT-84` | Préconditions (spécification RPG, numéros, chargement de données), extraction du corpus, catalogues, monde et peuplement, une classe par lot, refonte IHM, mécaniques manquantes |

La phase F compte **48 lots à venir** — quatre numéros (`LOT-31`, `LOT-48`, `LOT-71`, `LOT-73`)
ayant été retirés par fusion et n'étant pas réattribués, et ses trois préconditions (`LOT-77`,
`LOT-78`, `LOT-79`) étant livrées. Elle **ne suit pas** les précédentes : elle
s'entrelace avec B à E, chacun de ses lots servant un lot des phases antérieures qui, sans lui, se
bâtirait sur un catalogue fictif. L'ordre d'exécution recommandé, le chemin critique jusqu'au
`LOT-27` et le graphe complet des dépendances sont donnés par la
[feuille de route](@ref roadmap-010), §6 — et vérifiés en CI par `scripts/lint_lots.py`.

Décisions de cadrage actées avant le `LOT-01` : règles **d20 maison** (compatible SRD dans sa
structure, sans en dépendre), combat **sur la carte d'exploration** (grille dérivée de la couche
collision), **un héros au départ et quatre à terme** (rien ne suppose l'unicité), échelle
**1 case = 1,5 m**.
