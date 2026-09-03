# TACHE-03 — Boucle à pas de temps fixe (testable) {#lot-01-tache-03-boucle-pas-fixe}

**Lot :** [LOT-01](epic.md) · **Module :** `Source/Core` · **Statut :** terminé

## Contexte
La simulation doit être **déterministe** et indépendante du framerate d'affichage. La logique de cadencement est placée dans `Core` pour être **testable sans fenêtre ni GPU** (`EX-NFR-010`).

## Travail à réaliser
- Créer une classe `FixedTimestep` dans `Core` :
  - Pas de temps fixe configurable (défaut : **1/60 s**).
  - Méthode qui, à partir du **temps réel écoulé** fourni, calcule le **nombre de pas de simulation** à exécuter et conserve le **reste** (accumulateur).
  - Bornage anti-« spirale de la mort » (plafond du nombre de pas rattrapés par frame).
- Ne dépend d'**aucune API système** (pas de `QueryPerformanceCounter` dans la classe : le temps réel est passé en paramètre, mesuré par l'appelant côté `HMI`).

## Fichiers impactés
- `Source/Core/FixedTimestep.h`, `Source/Core/FixedTimestep.cpp` (nouveau).
- `Source/Core/CMakeLists.txt` (ajout des sources).
- `Source/Test/Unit/test_fixed_timestep.cpp` (nouveau).

## Tests (obligatoires)
- Un temps écoulé égal au pas fixe → exactement **1** pas.
- Un temps écoulé de 2,5 pas → **2** pas exécutés et un reste de 0,5 pas conservé, puis complété au tick suivant.
- Un temps écoulé énorme → nombre de pas **plafonné** (anti-spirale).
- Un temps écoulé nul ou inférieur au pas → **0** pas.

## Définition de fait (DoD)
- Comportement conforme, couvert par les tests ci-dessus (`ctest` vert).
- Aucune dépendance système dans la classe (compilable/testable en isolation).
- Compile `/W4 /WX`, formaté, API documentée `.h` + `.cpp`.

## Exigences
`EX-REN-021`, `EX-REN-020`, `EX-NFR-010`, `EX-NFR-002`, `EX-NFR-020`.
