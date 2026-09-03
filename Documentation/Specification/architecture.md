# Architecture (décisions dimensionnantes) {#spec-architecture}

> Statut : **livré** (`0.1.0`). Décisions **structurantes**, tenues depuis `LOT-01` (sens des
> dépendances, ECS, frontière simulation ↔ rendu) et confirmées jusqu'à la refonte Qt de l'IHM
> hors-jeu (`LOT-34` → `LOT-39`, voir Traçabilité ci-dessous). Aucune n'a été remise en cause par le
> programme `0.1.0`. Transverse à toutes les specs.

## 1. Modules & dépendances
- `Core` (simulation, indépendant du système), `HMI` (fenêtre, rendu, entrées, éditeur), `Elements` (assets), `Test`.
- \anchor EX-ARCH-001 **EX-ARCH-001** — Le sens des dépendances est `HMI → Core`, jamais l'inverse. `Core` est testable sans fenêtre ni GPU.

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
- \anchor EX-ARCH-022 **EX-ARCH-022** — Rendu **pixel art** : échantillonnage *nearest-neighbor*, zoom caméra de préférence en **facteurs entiers** (netteté).
- La grille de tuiles (gameplay/collisions) et les décors libres **coexistent** dans le même espace monde ; les décors ne sont pas calés sur la grille.

## 4. Frontière simulation ↔ rendu
- \anchor EX-ARCH-030 **EX-ARCH-030** — `Core` met à jour la simulation à **pas de temps fixe** ; `HMI` produit l'image en **lisant** l'état.
- \anchor EX-ARCH-031 **EX-ARCH-031** — Un **facteur d'interpolation** `[0,1]` entre le pas précédent et le pas courant est fourni au rendu pour lisser le mouvement (prévu dès le départ). **Concrétisé en `LOT-33`** : `core::FixedTimestep::interpolationAlpha` est passé au rendu par `hmi::GameSession::render`, et `hmi::SpriteRenderer` dessine chaque entité mobile à `lerp(position précédente, position courante, alpha)` via le composant de présentation `hmi::PreviousPosition` — sans jamais modifier l'état simulé (`EX-ARCH-012`).

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
  `LOT-40`), car `Core` ne doit connaître aucune ressource graphique (`EX-NFR-010`, `EX-ARCH-010`) ;
  les **niveaux** et les **décors** restent des données de `Core`, chargées et validées par leur
  propre chargeur (`EX-LVL-004`). Il n'existe donc **pas** de gestionnaire de ressources unique dans
  `Core` — la formulation initiale, antérieure à la séparation `Core`/`HMI` telle qu'elle est
  aujourd'hui appliquée, l'aurait obligé à dépendre de la présentation.
  Le **rechargement à chaud** des assets graphiques, écarté au MVP, est concrétisé en `LOT-43`.

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
aucun lot n'est donc pas une exigence orpheline (`LOT-66`).

> **Refonte IHM (`LOT-34` → `LOT-39`)** : l'interface **hors-jeu** (éditeur, menus, options) migre vers **Qt**, tandis que le **rendu in-game reste Direct3D 11** (`EX-ARCH-050`), embarqué dans un viewport Qt. Depuis le `LOT-38`, l'IHM « maison » et l'exécutable historique ont été retirés : `Source/HMI` porte désormais **l'unique application** (`JustAnotherDnDGame`, cible Qt) — code réparti par domaine (`Platform/`, `Input/`, `Graphics/`, `Game/`, `Localization/`, `Interface/`, `Editor/`) — et les **assets Qt déclaratifs** (`.ui`, `.qrc`, thème `.qss`) vivent dans `Source/Elements` (`UI/`, `Themes/`). La frontière `HMI → Core` (`EX-ARCH-010`) et la frontière simulation ↔ rendu (`EX-ARCH-030`/`031`) sont **inchangées**. Voir [`interface-ihm.md`](@ref spec-interface-ihm) (`EX-IHM-*`) et [`guide-ihm-qt`](@ref guide-ihm-qt).
