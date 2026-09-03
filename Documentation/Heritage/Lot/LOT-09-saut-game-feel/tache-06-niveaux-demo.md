# TACHE-06 — Niveaux de démo (séquence, dont saut requis) + preuve {#lot-09-tache-06-niveaux-demo}

**Lot :** [LOT-09](epic.md) · **Emplacement :** `Source/Elements/Levels` · **Statut :** à faire

## Contexte
La progression (TACHE-05) a besoin de **contenu** : une séquence d'au moins **deux** niveaux, de
difficulté croissante, dont un **exige le saut** (sinon les techniques de TACHE-03/04 ne sont pas
démontrées). Comme au LOT-08, la franchissabilité est **prouvée par test** — et ici, on prouve
aussi que le saut est **nécessaire**.

## Travail à réaliser
- **Niveau 1** : prise en main sans saut — réutiliser/adapter le niveau **descendant** du LOT-08
  (déplacement + chute + danger). Le renommer si la séquence adopte une convention (`demo1.json`).
- **Niveau 2** : **saut requis** — une **section ascendante** (marche de ~1 à 2 tuiles, dans la
  limite des ~2,5 tuiles de hauteur de saut) et/ou un **saut au-dessus d'un danger** (fosse).
  Entrée, sortie et validation conformes (`EX-LVL-004`).
- **Séquence** : cohérente avec la source choisie en TACHE-05 (liste explicite ou noms triés). Les
  niveaux sont **copiés** à côté de l'exécutable (comme au LOT-07/08).
- **Tests d'intégration** (dans `Core`, via les systèmes de simulation) :
  - **Niveau 2 franchissable avec saut** : un scénario d'entrées (déplacement + `jumpPressed` aux
    bons instants) atteint `Won` sans `Lost`.
  - **Saut requis** : un scénario « déplacement seul, sans saut » **n'atteint pas** `Won` sur le
    niveau 2 (bloqué par la marche) — prouve la nécessité du saut.
  - **Enchaînement** : franchir le niveau 1 puis le niveau 2 mène à la **fin de séquence** (test de
    la logique de séquence de TACHE-05, ou test d'intégration si praticable hors GPU).

## Fichiers impactés
- `Source/Elements/Levels/*.json` (niveaux 1 et 2 de la séquence).
- `Source/Test/Integration/test_physique_personnage.cpp` (ou fichier dédié) : tests de
  franchissabilité et de nécessité du saut.

## Tests (obligatoires)
- Niveau 2 : **avec saut** → `Won` ; **sans saut** → jamais `Won` (bloqué).
- Le niveau 1 reste franchissable (préserver/adapter la preuve du LOT-08).
- Scénarios **déterministes** (mêmes entrées → même issue, `EX-NFR-002`).

## Points d'attention
- **Hauteur atteignable** : la marche du niveau 2 doit être franchissable avec le réglage de saut
  (garder une marge sous ~2,5 tuiles) — sinon niveau infranchissable.
- **Tableau à caméra fixe** : chaque niveau tient dans un écran.
- **Scénario de test** : injecter `jumpPressed` au bon moment (au sol, avant la marche) ; garder le
  scénario lisible et robuste aux petits écarts de réglage (marge).
- Cohérence avec la **séquence** de TACHE-05 (ordre, noms, copie CMake).

## Définition de fait (DoD)
- Deux niveaux livrés et enchaînés ; franchissabilité **avec saut** et **nécessité du saut**
  prouvées par tests ; `ctest` vert ; build `/W4 /WX`, `CHANGELOG.md` à jour.

## Exigences
`EX-LVL-001`, `EX-LVL-004`, `EX-LVL-012`, `EX-GP-011`, `EX-NFR-002`, `EX-NFR-020`.
