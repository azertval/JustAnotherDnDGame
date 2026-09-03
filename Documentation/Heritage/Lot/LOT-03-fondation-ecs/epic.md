# LOT-03 — Fondation ECS & mathématiques Core {#lot-03}

> Statut : **terminé**. Substrat de la simulation : gameplay, tuiles et décors en dépendent. Décision ⚠️ tranchée : **ECS maison** (sparse sets).

## Objectif
Mettre en place, **dans `Core`**, un **ECS** fonctionnel et testé (entités, composants, systèmes, `World`) ainsi que les **types mathématiques** de base (`Vector2`, `Rect`, `Transform`). Le tout **indépendant de DirectX**, entièrement couvert par des tests unitaires.

À l'issue du lot : on peut créer des entités, leur attacher des composants, itérer dessus via des systèmes exécutés à pas de temps fixe — sans aucune fenêtre ni GPU.

## ⚠️ Décision préalable : ECS maison vs bibliothèque
| Option | Description | Pour | Contre |
|--------|-------------|------|--------|:----:|
| **Maison** *(par défaut, cohérent « from scratch »)* | Implémenter l'ECS nous-mêmes (sparse sets). | Maîtrise totale, valeur pédagogique, aucune dépendance. | Plus de code et de tests à écrire. |
| **EnTT** | Intégrer la bibliothèque header-only EnTT via FetchContent. | Mûre, performante, rien à écrire. | Dépendance externe, API imposée, s'éloigne du « from scratch ». |

Recommandation : **maison** (aligné sur l'esprit du projet). Si EnTT est retenu, ce lot se réduit à une tâche d'intégration + une couche d'adaptation aux conventions.

## Périmètre

### Inclus
- Types mathématiques de `Core` : `Vector2`, `Rect`, données de `Transform`.
- Entités : **handles générationnels** (index + génération), création/destruction, recyclage.
- Stockage de composants : **sparse set** typé (add/get/has/remove).
- **Requêtes/vues** : itérer les entités possédant un ensemble de composants.
- **Systèmes** et `World` : enregistrement et exécution des systèmes à **pas de temps fixe**.
- Un composant `Transform` et un système de mouvement de démonstration.

### Exclus (lots ultérieurs)
- Sérialisation concrète des composants (préparée par le design *données pures*, implémentée au lot « chargement de niveaux »).
- Rendu (les systèmes de rendu vivront dans `HMI` et **liront** les composants).
- Composants de gameplay spécifiques (collisions, animation…) au-delà de la démo.

## Exigences couvertes
- `EX-ARCH-010` — simulation fondée sur un ECS hébergé dans `Core`.
- `EX-ARCH-011` — composants = données pures ; logique dans les systèmes.
- `EX-ARCH-012` — les composants sont lisibles par le rendu sans mutation (API d'accès en lecture).
- `EX-ARCH-030` — systèmes exécutés à pas de temps fixe (orchestration `World`).
- `EX-ARCH-040` — types mathématiques propres à `Core`, sans dépendance DirectX.
- `EX-NFR-010`, `EX-NFR-002`, `EX-NFR-020` — `Core` testable, déterministe, couvert par tests.
- Prépare `EX-ARCH-090` / `EX-ARCH-100` (composants *données pures* → sérialisables ; décors comme entités).

## Découpage

> État : ✅ fait · 🔄 en cours · ⬜ non commencé.

| Tâche | Intitulé | Emplacement | État |
|-------|----------|-------------|:----:|
| [TACHE-01](tache-01-math-core.md) | Types mathématiques (`Vector2`, `Rect`) | `Core/Math` | ✅ Fait |
| [TACHE-02](tache-02-entites.md) | Entités : handles générationnels & cycle de vie | `Core/Ecs` | ✅ Fait |
| [TACHE-03](tache-03-stockage-composants.md) | Stockage de composants (sparse set typé) | `Core/Ecs` | ✅ Fait |
| [TACHE-04](tache-04-vues-requetes.md) | Requêtes / vues multi-composants | `Core/Ecs` | ✅ Fait |
| [TACHE-05](tache-05-systemes-world.md) | Systèmes & `World` (orchestration pas fixe) | `Core/Ecs` | ✅ Fait |
| [TACHE-06](tache-06-transform-demo.md) | Composant `Transform` + système de mouvement (démo) | `Core/Ecs` | ✅ Fait |

## Critères d'acceptation du lot
1. On peut créer/détruire des entités ; un handle vers une entité détruite est détecté comme **invalide** (génération).
2. On peut attacher/lire/retirer des composants typés sur une entité.
3. Une vue multi-composants itère **exactement** les entités possédant tous les composants demandés.
4. Un système de mouvement met à jour les `Transform` à partir d'une vitesse, de façon **déterministe**, au pas fixe.
5. Couverture par tests unitaires de chaque brique (entités, composants, vues, systèmes, math) ; `ctest` vert.
6. Build **sans avertissement** (`/W4 /WX`), **CI verte**, API documentée en Doxygen, `CHANGELOG.md` mis à jour.

## Dépendances
- Réutilise `FixedTimestep` de [LOT-01](../LOT-01-fenetre-boucle-jeu/epic.md) pour cadencer `World::update` (dépendance douce : l'ECS est testable indépendamment).
- Utilise la **journalisation et les assertions** de [LOT-02](../LOT-02-journalisation/epic.md) (`JADG_ASSERT` pour les préconditions des pools et handles).

## Navigation des tâches
- @subpage lot-03-tache-01-math-core
- @subpage lot-03-tache-02-entites
- @subpage lot-03-tache-03-stockage-composants
- @subpage lot-03-tache-04-vues-requetes
- @subpage lot-03-tache-05-systemes-world
- @subpage lot-03-tache-06-transform-demo
