# TACHE-02 — Implémentation des niveaux {#lot-25-tache-02-implementation-niveaux}

**Lot :** [LOT-25](epic.md) · **Emplacement :** `Source/Elements/Levels` · **Statut :** fait

> **Écart au plan initial** : les niveaux ont été conçus directement en JSON (calculs de géométrie
> vérifiés par des tests d'intégration réactifs, itérés via `--gtest_filter`) plutôt que dans
> l'éditeur intégré comme prévu ci-dessous — voir la note de `TACHE-04` sur la vérification visuelle
> pour la justification (aucune session interactive dans cette conversation).

## Contexte
Traduit le tableau mécanique → niveau(x) de `TACHE-01` en fichiers de niveaux réels, via
l'éditeur intégré (`@ref guide-editeur`) plutôt qu'en JSON écrit à la main — cohérent avec
`EX-EDIT-*` (l'édition passe par l'outil, pas par le texte brut).

## Travail à réaliser
- **Supprimer** `demo.json`…`demo5.json` (et toute autre trace de l'ancienne séquence dans
  `Source/Elements/Levels/`) — base vide, conformément à la décision de cadrage de l'épic.
- Pour chaque niveau **isolé** du tableau de `TACHE-01` : le concevoir **entièrement** dans
  l'éditeur intégré (peinture, mécanismes, essai immédiat `P`), l'enregistrer dans
  `Source/Elements/Levels/`, avec le nom décidé en `TACHE-01` (ex. `demo-double-saut.json`,
  `demo-pente.json`) — plus lisible qu'une numérotation `demoN.json` qui ne dit rien du contenu.
- **En dernier**, une fois tous les niveaux isolés vérifiés individuellement : concevoir le
  **niveau final combiné** (`demo-final.json` ou nom équivalent décidé en `TACHE-01`), d'après le
  contenu défini en `TACHE-01` — le construire seulement après coup permet de réutiliser les
  agencements de tuiles déjà éprouvés dans les niveaux isolés plutôt que d'improviser à partir de
  rien.

## Fichiers impactés
- `Source/Elements/Levels/*.json` : suppression des fichiers existants, création des nouveaux
  selon le tableau de `TACHE-01`.

## Tests (obligatoires)
- Chaque niveau **isolé**, testé immédiatement dans l'éditeur (`P`) : franchissable en utilisant la
  mécanique visée, **infranchissable** sans elle (vérification manuelle, cohérente avec le critère
  d'acceptation 1 de l'épic).
- Le niveau **final combiné**, testé de la même façon : franchissable en utilisant l'ensemble des
  mécaniques qu'il mélange (critère d'acceptation 2 de l'épic) — pas seulement « ne plante pas ».

## Points d'attention
- **Un niveau qui ne teste rien n'a pas sa place** : si un niveau s'avère franchissable sans jamais
  utiliser la mécanique qu'il était censé isoler, le corriger avant de passer à `TACHE-03` plutôt
  que de livrer un test système qui ne teste rien en pratique.
- **Supprimer avant de recréer** : vérifier qu'aucune référence résiduelle aux anciens noms
  (`demo.json`…`demo5.json`) ne subsiste ailleurs dans le dépôt (recherche texte) avant de passer à
  `TACHE-03`, qui les remplace dans `main.cpp`/le test système.
- Conserver la convention de nommage des fichiers de niveaux **stable** une fois choisie en début
  de tâche : `TACHE-03` (séquence + test système) référence ces noms directement.

## Définition de fait (DoD)
- Tous les niveaux isolés du tableau de `TACHE-01` **et** le niveau final combiné existent, sont
  enregistrés, et vérifiés manuellement franchissables.

## Exigences
Aucune exigence propre — production de contenu, pas de code moteur.
