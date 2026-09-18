# Guide du développeur {#guide}

Ce guide explique **toutes les notions couvertes par le moteur** et **comment le code les
implémente**. Objectif : un développeur ayant des notions de C++ mais **aucune expérience du
développement de jeu vidéo** comprend l'ensemble du code **en autonomie** — chaque page part des
définitions de base (qu'est-ce qu'une boucle de jeu ? un ECS ? une couche de tuiles ? un lot
de sprites ?) avant d'entrer dans l'implémentation, plutôt que de présupposer ce vocabulaire acquis.
Chaque page décrit les fonctions clés, leurs invariants, le **pourquoi** des choix de conception
(pas seulement le *quoi*), et renvoie aux **explications mathématiques/algorithmiques** derrière
les concepts.

## Comment lire ce guide

- Les noms de types et de fonctions (`core::World`, `core::ExplorationSession`, …) sont
  **cliquables** : ils mènent à la référence de code Doxygen (signature, doc détaillée).
- Les liens externes (⧉) pointent vers les **fondements mathématiques/algorithmiques**.
- Le *quoi* et le *pourquoi* vivent dans les [spécifications](@ref specifications) ; ce guide
  couvre le *comment* — y compris les notions de game dev prérequises pour le comprendre.
- Les pages sont **indépendantes mais s'appuient les unes sur les autres** (voir leurs sections
  « Voir aussi ») : @ref guide-maths pose le vocabulaire (vecteurs, rectangles, unités) réutilisé
  par @ref guide-niveaux et @ref guide-rendu ; @ref guide-boucle et @ref guide-ecs posent les deux
  piliers d'architecture (déterminisme, données/logique) sur lesquels tout le reste s'appuie. En cas
  de doute sur un terme, remonter à la page qui le définit plutôt que de le supposer connu.

## Architecture en deux couches

Le moteur sépare strictement :

- **`Core`** — logique pure : ECS, mathématiques, modèle de carte, exploration, combat, règles, temps.
  **Aucune dépendance** à DirectX ni à la fenêtre → testable sans GPU (`EX-NFR-010`).
- **`HMI`** — présentation : deux applications Qt depuis le `LOT-86` — le **jeu** en Qt Quick,
  l'**éditeur de niveaux** en Qt Widgets —, le rendu de scène sur QRhi (Direct3D 11 par défaut sous
  Windows) et les entrées. Dépend de `Core`, **jamais l'inverse** (`EX-ARCH-010`).
- **`Source/Ui`** — ce que la **conception** modifie : les écrans du jeu en QML déclaratif, leurs
  jetons et leurs ornements. Ne connaît de `HMI` que ses vues-modèles (`EX-IHM-100`).

La règle d'or : **la simulation est dans `Core`, déterministe et testée** ; `HMI` orchestre et
affiche. Cette frontière est ce qui rend le moteur analysable domaine par domaine.

## Plan du guide

- @subpage guide-boucle — la boucle de jeu et le **pas de temps fixe** (déterminisme).
- @subpage guide-ecs — l'**ECS** maison (entités, composants, systèmes, vues).
- @subpage guide-maths — les **mathématiques** du moteur (`Vector2`, `Rect`, unités).
- @subpage guide-niveaux — les **cartes** : modèle, couches, entités, chargement JSON.
- @subpage guide-entrees — les **entrées** et leur traduction en **actions logiques**.
- @subpage guide-rendu — le **rendu 2D** : QRhi, lot de sprites, atlas, caméra, composition de scène.
- @subpage guide-journalisation — la **journalisation** et les **assertions** : niveaux, sinks, macros.
- @subpage guide-editeur — l'**éditeur de niveaux** : brouillon mutable, peinture, undo/redo, essai immédiat.
- @subpage guide-ecrans — la **navigation** : table de transitions, routeur et pile d'écrans QML, pause.
- @subpage guide-ihm-qt — l'**IHM Qt** : deux applications (jeu en Qt Quick, éditeur en Widgets), module QML, surface de rendu QRhi.
- @subpage guide-conception-qds — **concevoir les écrans** dans Qt Design Studio : ce qui se modifie sans jamais ouvrir un fichier source.
- @subpage guide-design-ihm — l'**éditeur outil interne** et l'**architecture de l'information** : barre d'état, panneaux, échelle des écrans du jeu.
- @subpage guide-audio — l'**audio** : moteur Qt Multimedia et réglage du volume.
