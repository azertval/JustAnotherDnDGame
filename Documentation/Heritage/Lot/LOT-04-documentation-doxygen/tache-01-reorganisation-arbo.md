# TACHE-01 — Réorganisation de l'arborescence documentaire {#lot-04-tache-01-reorganisation-arbo}

**Lot :** [LOT-04](epic.md) · **Emplacement :** racine → `Documentation/` · **Statut :** fait

## Contexte
Les documents projet sont dispersés : `Specification/` et `Lot/` sont à la racine, et `conventions.md` vit à part dans `Documentation/`. Pour un site de documentation cohérent (et une génération Doxygen simple à cadrer), tout le documentaire est regroupé sous `Documentation/`, et les conventions rejoignent les spécifications.

## Travail à réaliser
- Déplacer `Specification/` → `Documentation/Specification/` (`git mv`, historique préservé).
- Déplacer `Lot/` → `Documentation/Lot/` (y compris le dossier `LOT-04` lui-même).
- **Retirer les préfixes numériques** des specs (`00-vision.md` → `vision.md`, …, `08-decors.md` → `decors.md`) : l'ordre sera porté par la liste `@subpage` de l'index (TACHE-04), plus par les noms de fichiers. Les lots, eux, **conservent** leur identifiant `LOT-XX` (numéros stables, non réordonnés).
- Intégrer `Documentation/conventions.md` aux spécifications, **sans numéro** : `Documentation/Specification/conventions.md`.
- Mettre à jour **toutes** les références au fil des déplacements et renommages :
  - `README.md` (racine), `CONTRIBUTING.md`, `CHANGELOG.md`.
  - `Documentation/README.md`, `Source/Core/Diagnostics/README.md`, `Source/Test/README.md`.
  - Liens relatifs internes des lots (`Lot/README.md`, epics : `../../Specification/…`).
  - **Liens croisés entre specs** (une spec référençant une autre par son ancien nom `NN-...md`).
  - Exigence `EX-NFR-012` (spec des exigences non fonctionnelles) dont le lien `../Documentation/conventions.md` doit pointer vers `conventions.md` (même rubrique).

## Fichiers impactés
- Déplacements/renommages : `Specification/**` (dont retrait des préfixes), `Lot/**`, `Documentation/conventions.md` → `Documentation/Specification/conventions.md`.
- Édits de références : `README.md`, `CONTRIBUTING.md`, `CHANGELOG.md`, `Documentation/README.md`, `Source/Core/Diagnostics/README.md`, `Source/Test/README.md`, `Documentation/Lot/README.md`, epics de lots, spec des exigences non fonctionnelles, liens croisés entre specs.

## Vérifications (obligatoires)
- Recherche globale : **aucune** occurrence résiduelle de `](Specification/`, `](Lot/`, `Documentation/conventions.md`, ni d'un nom de spec préfixé `NN-...md`, pointant vers un chemin disparu.
- Chaque lien relatif Markdown (spec ↔ spec, spec ↔ lot, epic ↔ tâche) résout vers un fichier existant.
- `git mv` utilisé (déplacements **et** renommages apparaissent comme tels dans `git status`).

## Points d'attention
- Le `README.md` racine **reste** (vitrine GitHub) mais pointe désormais vers `Documentation/`.
- `Documentation/generated/` reste gitignoré ; ne pas le déplacer ni le committer.
- Ne rien changer au code de `Source/` (seuls des READMEs y sont édités pour leurs liens).

## Définition de fait (DoD)
- Arborescence conforme à l'epic ; conventions intégrées aux spécifications.
- Zéro lien cassé (vérifié) ; renommages traçables dans l'historique git.

## Exigences
`EX-NFR-012`.
