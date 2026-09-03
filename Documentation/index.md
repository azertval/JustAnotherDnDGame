# JustAnotherDnDGame

RPG 2D **en vue de dessus** développé **from scratch** en **C++20 / Qt QRhi** (Direct3D 11 sous
Windows), sans moteur tiers : exploration en temps réel façon Zelda, et rencontres en **combat
tactique au tour par tour** régi par un système **d20** maison.

Cette documentation rassemble, en un seul endroit, les **spécifications** du
projet, les **lots de travail**, le **manuel utilisateur** et la **référence de
code** générée à partir des sources.

## En bref
- **Langage & rendu** : C++20, Qt QRhi (Direct3D 11 sous Windows), boucle de jeu à pas de temps
  fixe déterministe.
- **Architecture** : un cœur de simulation (`Core`) **indépendant** de la présentation
  (`HMI`), reposant sur un **ECS maison** ; les assets statiques vivent dans `Elements`.
- **Qualité** : build sans avertissement (`/W4 /WX`), tests unitaires et d'intégration
  (GoogleTest), documentation Doxygen et CI GitHub Actions.

## Origine

Ce dépôt est **dérivé de `ProjectGaming`**, un jeu de plateforme/puzzle en vue de côté livré en
`0.1.3` après 74 lots. Le `LOT-01` en a repris le moteur (ECS, pas fixe, mathématiques, balayage
AABB, chargeur de niveaux, rendu, éditeur, IHM Qt) et retiré tout le gameplay propre à la vue de
côté. L'historique git d'origine est **intégralement conservé**, et le programme de lots du
platformer reste consultable en lecture seule sous `Documentation/Heritage/`.

## Avancement

Le détail de chaque lot (objectifs, tâches, avancement) est dans la rubrique
[Lots](@ref lots) ; l'historique des livraisons est dans `CHANGELOG.md`.

## Navigation
- @subpage guide — **Guide du développeur** : comprendre tout le moteur (concepts, code, maths).
- @subpage cahiertest — **Cahier de test** : tous les cas de test (catégorie, criticité, étapes).
- @subpage specifications — besoins, contraintes et exigences (`EX-…`), conventions de code.
- @subpage lots — plan de travail : un lot par incrément, découpé en tâches.
- @subpage manuel — manuel utilisateur (télécharger et lancer le jeu).
- **Référence de code** — classes, espaces de noms et fichiers de `Source/` : voir
  l'arbre de navigation (menu latéral) et les onglets *Namespaces* / *Classes*.
