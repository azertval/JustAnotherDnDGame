# Architecture (décisions dimensionnantes) {#spec-architecture}

> Statut : **livré** (`0.1.0`). Décisions **structurantes**, tenues depuis `LOT-01` (sens des
> dépendances, ECS, frontière simulation ↔ rendu) et confirmées jusqu'à la refonte Qt de l'IHM
> hors-jeu (`LOT-H-34` → `LOT-H-39`, voir Traçabilité ci-dessous). Aucune n'a été remise en cause par le
> programme `0.1.0`. Transverse à toutes les specs.

## 1. Modules & dépendances
- `Core` (simulation, indépendant du système), `HMI` (fenêtre, rendu, entrées, éditeur), `Elements` (assets), `Test`.
- \anchor EX-ARCH-001 **EX-ARCH-001** — Le sens des dépendances est `HMI → Core`, jamais l'inverse. `Core` est testable sans fenêtre ni GPU.
- \anchor EX-ARCH-002 **EX-ARCH-002** — L'**ordre des passes** d'un pas fixe est une donnée
  du **mode de jeu**, jamais de l'orchestrateur. Un RPG en a au moins trois — exploration
  (déplacement, animation, caméra, mécanismes, issue), dialogue (monde gelé, seul le runner de
  dialogue avance), combat (initiative, tour actif, résolution d'action) — et les entasser en `if`
  dans la boucle de session ferait grossir sans fin une fonction déjà longue. L'orchestrateur garde
  ce qui ne dépend pas du mode (monde ECS, caméra, événements, HUD, pas fixe, interpolation) et
  **offre** ses passes ; le mode décide seul de leur enchaînement, et la sélection se fait par
  **polymorphisme**, jamais par un test de type. L'interface des passes ne parle que de `Core` :
  un mode se vérifie donc sans fenêtre, quand son orchestrateur, lui, exige un GPU. Concrétisé en
  `LOT-05`.

## 2. Modèle d'entités : ECS
Choix retenu : **ECS complet**, hébergé dans `Core`. Assumé plus lourd, justifié par le grand nombre de types d'objets manipulables visés (dont décors dynamiques, cf. §11).

- \anchor EX-ARCH-010 **EX-ARCH-010** — La simulation repose sur un **ECS** (entités = identifiants, composants = données pures, systèmes = logique).
- \anchor EX-ARCH-011 **EX-ARCH-011** — Les **composants ne contiennent pas de logique** ; la logique vit dans les **systèmes**.
- \anchor EX-ARCH-012 **EX-ARCH-012** — Le rendu (`HMI`) **lit** les composants (ex. `Transform`, `Sprite`) sans les muter.
- Composants pressentis (indicatif) : `Transform` (position/échelle/rotation en unités monde), `Velocity`, `Collider`, `Sprite`, `Animation`, `Layer`.
- Systèmes pressentis : mouvement/physique, collisions, animation (`Core`) ; rendu (lecture, `HMI`).

## 3. Coordonnées & unités — trois espaces distincts
| Espace | Unité | Rôle |
|--------|-------|------|
| **Monde** | tuile (float) | Position logique des objets et décors, indépendante de l'écran |
| **Art / texel** | pixel natif de l'asset | Résolution du pixel art |
| **Écran** | pixel d'affichage | Dépend de la résolution / du zoom |

- \anchor EX-ARCH-020 **EX-ARCH-020** — Unité monde = **1 tuile**, positions en **float**, origine **haut-gauche**, axe **Y vers le bas**.
- \anchor EX-ARCH-021 **EX-ARCH-021** — Un facteur **pixels-par-unité** (16) régit la conversion monde → écran ; les conversions sont centralisées (pas de constantes éparpillées).
- \anchor EX-ARCH-022 **EX-ARCH-022** — Rendu **fidèle à la nature de l'asset** : l'échantillonnage se choisit par asset — *nearest* pour une image dont les pixels sont signifiants (les tuiles héritées), **interpolé** pour une illustration peinte, qui n'a pas de grille à préserver et que le plus proche voisin rendrait crénelée. Le zoom caméra n'est plus contraint aux facteurs entiers : cette contrainte n'a jamais servi qu'à ne pas casser la grille du pixel art.
  > **Refondue au `LOT-66`.** Elle imposait « rendu **pixel art** : *nearest-neighbor*, zoom de préférence en facteurs entiers », et c'est d'elle que **dix autres exigences** tenaient leur justification — la racine devait tomber la première, sans quoi chaque feuille aurait pu citer une règle abandonnée sans que rien ne le signale. La scène, elle, filtre encore en *nearest* : c'est correct pour les tuiles héritées, qui sont ses seuls assets aujourd'hui, et le basculement suit l'arrivée des plans peints ([LOT-76](@ref lot-76)).
- La grille de tuiles (gameplay/collisions) et les décors libres **coexistent** dans le même espace monde ; les décors ne sont pas calés sur la grille.

## 4. Frontière simulation ↔ rendu
- \anchor EX-ARCH-030 **EX-ARCH-030** — `Core` met à jour la simulation à **pas de temps fixe** ; `HMI` produit l'image en **lisant** l'état.
- \anchor EX-ARCH-031 **EX-ARCH-031** — Un **facteur d'interpolation** `[0,1]` entre le pas précédent et le pas courant est fourni au rendu pour lisser le mouvement (prévu dès le départ). **Concrétisé en `LOT-H-33`** : `core::FixedTimestep::interpolationAlpha` est passé au rendu par `hmi::GameSession::render`, et `hmi::SpriteRenderer` dessine chaque entité mobile à `lerp(position précédente, position courante, alpha)` via le composant de présentation `hmi::PreviousPosition` — sans jamais modifier l'état simulé (`EX-ARCH-012`).

## 5. Mathématiques dans Core
- \anchor EX-ARCH-040 **EX-ARCH-040** — `Core` définit **ses propres types** mathématiques (`Vector2`, `Rect`, …), **sans dépendance DirectX**. La conversion vers `DirectXMath` a lieu uniquement à la frontière de rendu (`HMI`).

## 6. Abstraction de rendu
- \anchor EX-ARCH-050 **EX-ARCH-050** — Le rendu est un **wrapper mince** au-dessus de Direct3D 11 (sprite batch). Pas de couche d'abstraction multi-backend (DirectX-only est acté).

## 7. Modèle de threading
- \anchor EX-ARCH-060 **EX-ARCH-060** — Boucle **mono-thread** au MVP. Un éventuel chargement asynchrone sera isolé plus tard, sans remettre en cause la simulation déterministe.

## 8. Communication inter-systèmes
- \anchor EX-ARCH-070 **EX-ARCH-070** — Communication par **appels directs / observateur simple**. Pas de bus d'événements tant que le couplage reste faible (réévalué si nécessaire).

## 9. Gestion des ressources
- \anchor EX-ARCH-080 **EX-ARCH-080** — Les ressources sont gérées par **nom logique**, avec
  chargement **à la demande** et mise en cache. La gestion vit **du côté qui possède la ressource** :
  les **textures** relèvent de `HMI` (registre construit sur le décodage d'image et Direct3D 11,
  `LOT-H-40`), car `Core` ne doit connaître aucune ressource graphique (`EX-NFR-010`, `EX-ARCH-010`) ;
  les **niveaux** et les **décors** restent des données de `Core`, chargées et validées par leur
  propre chargeur (`EX-LVL-004`). Il n'existe donc **pas** de gestionnaire de ressources unique dans
  `Core` — la formulation initiale, antérieure à la séparation `Core`/`HMI` telle qu'elle est
  aujourd'hui appliquée, l'aurait obligé à dépendre de la présentation.
  Le **rechargement à chaud** des assets graphiques, écarté au MVP, est concrétisé en `LOT-H-43`.

## 10. Contrainte « éditeur intégré »
- \anchor EX-ARCH-090 **EX-ARCH-090** — Le modèle de niveau **et les décors** constituent un **état ECS mutable et sérialisable** ; le rendu de `HMI` est utilisable **hors mode jeu** ; les états de jeu incluent un état **Éditeur**. (Respecté tôt = cheap ; rajouté tard = cher.)

## 11. Décors dynamiques (accommodation dimensionnante)
Les décors (cf. [`decors.md`](decors.md)) sont manipulables **à la conception (éditeur)** et **à terme en jeu par le joueur** (mécanique). Conséquence structurante :

- \anchor EX-ARCH-100 **EX-ARCH-100** — Les décors sont des **entités de la simulation** (`Core`), et non de simples éléments de rendu, afin d'être manipulables de façon **déterministe** et **sérialisable**.

## Traçabilité
Ces décisions conditionnent tous les lots. Détail des décors et du pipeline pixel art : [`decors.md`](decors.md). Exigences non fonctionnelles associées : [`exigences-non-fonctionnelles.md`](exigences-non-fonctionnelles.md).

`EX-ARCH-001`, `EX-ARCH-060` et `EX-ARCH-070` sont des **invariants transverses** : chaque lot les
respecte par construction (sens des dépendances, boucle mono-thread, communication directe) sans
avoir besoin de les citer nommément dans son « Exigences couvertes ». Qu'ils n'apparaissent dans
aucun lot n'est donc pas une exigence orpheline (`LOT-H-66`).

> **Refonte IHM (`LOT-H-34` → `LOT-H-39`)** : l'interface **hors-jeu** (éditeur, menus, options) migre vers **Qt**, tandis que le **rendu in-game reste Direct3D 11** (`EX-ARCH-050`), embarqué dans un viewport Qt. Depuis le `LOT-H-38`, l'IHM « maison » et l'exécutable historique ont été retirés : `Source/HMI` porte désormais **l'unique application** (`JustAnotherDnDGame`, cible Qt) — code réparti par domaine (`Platform/`, `Input/`, `Graphics/`, `Game/`, `Localization/`, `Interface/`, `Editor/`) — et les **assets Qt déclaratifs** (`.ui`, `.qrc`, thème `.qss`) vivent dans `Source/Elements` (`UI/`, `Themes/`). La frontière `HMI → Core` (`EX-ARCH-010`) et la frontière simulation ↔ rendu (`EX-ARCH-030`/`031`) sont **inchangées**. Voir [`interface-ihm.md`](@ref spec-interface-ihm) (`EX-IHM-*`) et [`guide-ihm-qt`](@ref guide-ihm-qt).

> **Séparation de la conception et du code (`LOT-86`)** : l'IHM se scinde en **deux applications**.
> Le **jeu** passe à **Qt Quick** (`JustAnotherDnDGame`, `QGuiApplication`, ne lie pas
> `Qt6::Widgets`) ; l'**éditeur de niveaux** reste en Qt Widgets dans son propre binaire
> (`LevelEditor`). Une couche de **présentation** (`Source/HMI/Presentation`) transforme l'état du
> jeu en données affichables sans rien dessiner, et les écrans vivent en QML dans `Source/Ui` — que
> Qt Design Studio ouvre et réenregistre. `EX-ARCH-001` (sens des dépendances), `EX-ARCH-030`/`031`
> (frontière simulation ↔ rendu) et `EX-ARCH-050` (rendu au travers de QRhi) sont **inchangées** :
> le portage déplace l'hôte du rendu (`QRhiWidget` → `QQuickRhiItem`), jamais sa cible. Voir
> [`interface-ihm.md`](@ref spec-interface-ihm) §11 (`EX-IHM-100` → `EX-IHM-105`) et
> @ref guide-conception-qds.
