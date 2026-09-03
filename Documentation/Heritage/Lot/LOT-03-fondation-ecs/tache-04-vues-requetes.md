# TACHE-04 — Requêtes / vues multi-composants {#lot-03-tache-04-vues-requetes}

**Lot :** [LOT-03](epic.md) · **Emplacement :** `Source/Core/Ecs` · **Statut :** fait

## Contexte
Les systèmes ont besoin d'itérer les entités possédant un **ensemble** de composants (ex. toutes celles ayant `Transform` **et** `Velocity`). C'est le rôle des vues.

## Travail à réaliser
- Une **vue** `view<A, B, ...>()` exposée par le `World` :
  - Itère les entités possédant **tous** les composants demandés.
  - Donne accès, pour chaque entité, à ses composants (références) et à son handle `Entity`.
  - Itération pilotée par la **plus petite pool** parmi les types demandés (optimisation), avec filtrage sur les autres.
- Forme d'API au choix : `for (auto [entity, a, b] : world.view<A, B>())` ou `world.view<A, B>().each([](Entity, A&, B&){ ... })`.

## Fichiers impactés
- `Source/Core/Ecs/View.h` (nouveau, template) et intégration `World`.
- `Source/Test/Unit/test_view.cpp` (nouveau).

## Tests (obligatoires)
- Une vue `<A, B>` itère **exactement** les entités ayant A et B (ni celles avec A seul, ni B seul).
- Les composants fournis par la vue correspondent bien à l'entité itérée.
- La modification d'un composant via la vue est visible ensuite (référence, pas copie).
- Vue vide (aucune entité correspondante) : itération sans erreur.

## Points d'attention
- La vue ne doit **pas** invalider ses itérateurs si l'on ne modifie que les valeurs des composants (documenter l'interdiction d'ajouter/détruire pendant l'itération, ou définir un contrat clair).
- Conventions : documentation `.h`, `[[nodiscard]]` sur `view(...)`.

## Définition de fait (DoD)
- Sélection multi-composants exacte et efficace, testée (`ctest` vert).
- Compile `/W4 /WX`, formaté, documenté Doxygen.

## Exigences
`EX-ARCH-010`, `EX-ARCH-012`, `EX-NFR-010`, `EX-NFR-020`.
