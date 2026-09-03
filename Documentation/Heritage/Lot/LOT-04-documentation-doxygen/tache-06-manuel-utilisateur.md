# TACHE-06 — Manuel utilisateur (squelette + première page) {#lot-04-tache-06-manuel-utilisateur}

**Lot :** [LOT-04](epic.md) · **Emplacement :** `Documentation/Manuel/` · **Statut :** fait

## Contexte
Le site doit accueillir, à terme, un **manuel utilisateur** destiné aux non-développeurs. Ce lot en pose la structure et livre **une première page réelle** ; le contenu approfondi viendra plus tard.

## Travail à réaliser
- Créer `Documentation/Manuel/` avec une page d'accueil « Manuel utilisateur » (ancre `{#manuel}`), intégrée à la navigation depuis l'accueil (`@subpage`).
- Rédiger **une première page réelle** : « Télécharger et lancer le jeu » — récupérer la release Debug (`debug-latest`, publiée par `release.yml`), décompresser, lancer l'exécutable ; prérequis Windows.
- Prévoir la place des futures pages (prise en main, commandes) sous forme d'une brève liste « À venir », sans en rédiger le contenu.

## Fichiers impactés
- `Documentation/Manuel/manuel.md` (accueil de la rubrique, nouveau ; nommé `manuel.md` et non `index.md` pour éviter un conflit de page principale Doxygen).
- `Documentation/Manuel/telecharger-et-lancer.md` (première page réelle, nouveau).

## Vérifications (obligatoires)
- La rubrique « Manuel » s'ouvre depuis l'accueil et contient la page « Télécharger et lancer ».
- Les instructions de la première page sont exactes vis-à-vis de la release Debug produite par la CI (`release.yml`, artefact `debug-latest`).
- Aucun avertissement Doxygen.

## Points d'attention
- Ton **non technique** : la cible est un utilisateur, pas un développeur.
- Ne pas anticiper des fonctionnalités de jeu inexistantes ; se limiter à ce qui est réellement livrable aujourd'hui (lancement de l'exécutable).

## Définition de fait (DoD)
- Rubrique Manuel présente et navigable, avec une première page utile et exacte, sans avertissement.

## Exigences
`EX-NFR-012`.
