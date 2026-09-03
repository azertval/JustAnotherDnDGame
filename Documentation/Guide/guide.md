# Guide du développeur {#guide}

Ce guide explique **toutes les notions couvertes par le moteur** et **comment le code les
implémente**. Objectif : un développeur ayant des notions de C++ mais **aucune expérience du
développement de jeu vidéo** comprend l'ensemble du code **en autonomie** — chaque page part des
définitions de base (qu'est-ce qu'une boucle de jeu ? un ECS ? une collision continue ? un *coyote
time* ?) avant d'entrer dans l'implémentation, plutôt que de présupposer ce vocabulaire acquis.
Chaque page décrit les fonctions clés, leurs invariants, le **pourquoi** des choix de conception
(pas seulement le *quoi*), et renvoie aux **explications mathématiques/algorithmiques** derrière
les concepts.

## Comment lire ce guide

- Les noms de types et de fonctions (`core::World`, `core::sweepAabb`, …) sont **cliquables** :
  ils mènent à la référence de code Doxygen (signature, doc détaillée).
- Les liens externes (⧉) pointent vers les **fondements mathématiques/algorithmiques**.
- Le *quoi* et le *pourquoi* vivent dans les [spécifications](@ref specifications) ; ce guide
  couvre le *comment* — y compris les notions de game dev prérequises pour le comprendre.
- Les pages sont **indépendantes mais s'appuient les unes sur les autres** (voir leurs sections
  « Voir aussi ») : @ref guide-maths pose le vocabulaire (vecteurs, AABB, unités) réutilisé par
  @ref guide-physique et @ref guide-niveaux ; @ref guide-boucle et @ref guide-ecs posent les deux
  piliers d'architecture (déterminisme, données/logique) sur lesquels tout le reste s'appuie. En cas
  de doute sur un terme, remonter à la page qui le définit plutôt que de le supposer connu.

## Architecture en deux couches

Le moteur sépare strictement :

- **`Core`** — logique pure : ECS, mathématiques, physique, modèle de niveau, gameplay, temps.
  **Aucune dépendance** à DirectX ni à la fenêtre → testable sans GPU (`EX-NFR-010`).
- **`HMI`** — présentation : l'application **Qt** (fenêtre, menu, options, éditeur), le rendu
  Direct3D 11 du jeu embarqué dans un viewport, et les entrées. Dépend de `Core`, **jamais l'inverse**
  (`EX-ARCH-010`).

La règle d'or : **la simulation est dans `Core`, déterministe et testée** ; `HMI` orchestre et
affiche. Cette frontière est ce qui rend le moteur analysable domaine par domaine.

## Plan du guide

- @subpage guide-boucle — la boucle de jeu et le **pas de temps fixe** (déterminisme).
- @subpage guide-ecs — l'**ECS** maison (entités, composants, systèmes, vues).
- @subpage guide-maths — les **mathématiques** du moteur (`Vector2`, AABB, unités).
- @subpage guide-physique — la **physique** du personnage : balayage AABB, gravité, saut, dash.
- @subpage guide-niveaux — les **niveaux** : modèle, chargement JSON, mécanismes, budgets.
- @subpage guide-entrees — les **entrées** et leur traduction en **actions logiques**.
- @subpage guide-rendu — le **rendu 2D** : Direct3D 11, sprite batching, atlas, caméra, interpolation.
- @subpage guide-journalisation — la **journalisation** et les **assertions** : niveaux, sinks, macros.
- @subpage guide-editeur — l'**éditeur de niveaux intégré** : brouillon mutable, peinture, undo/redo, essai immédiat.
- @subpage guide-atelier-pixel-art — l'**atelier pixel art** : opérations pures, historique local, canevas, palettes.
- @subpage guide-ecrans — la **navigation** Qt : menu, jeu, éditeur et options (pages empilées, signaux).
- @subpage guide-ihm-qt — la **refonte IHM vers Qt** : socle applicatif, viewport Direct3D 11, boucle et entrées Qt.
- @subpage guide-design-ihm — le **système de design** et l'**architecture de l'information** : jetons, thème, actions, barre d'état.
- @subpage guide-audio — l'**audio** : moteur Qt Multimedia, catalogue de sons, détection des transitions et table événement → son.

Les notions d'apprentissage automatique mises en œuvre par le solveur IA font l'objet d'un guide
séparé, archivé sous `Documentation/Heritage/Guide-Annexe/` avec le programme d'IA qu'il
documentait (retiré au `LOT-01`).
