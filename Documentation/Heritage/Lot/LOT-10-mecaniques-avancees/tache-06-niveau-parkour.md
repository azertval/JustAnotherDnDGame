# TACHE-06 — Niveau de démo « parkour » + preuve système {#lot-10-tache-06-niveau-parkour}

**Lot :** [LOT-10](epic.md) · **Emplacement :** `Source/Elements/Levels` · **Statut :** à faire

## Contexte
Pour démontrer et **verrouiller** les mécaniques, il faut un niveau qui les **exige** : au moins un
passage franchissable seulement en enchaînant double saut, wall jump et/ou dash. Comme aux lots
précédents, la franchissabilité est **prouvée par test**, et le test **système** enchaîne la
séquence complète.

## Travail à réaliser
- Concevoir un niveau (JSON, format LOT-07) « parkour » avec, par exemple :
  - un **couloir vertical** (deux murs) à **remonter en wall jump** ;
  - un **gouffre** trop large pour un simple saut, franchi au **dash** (horizontal ou diagonal) ;
  - une plateforme atteignable seulement au **double saut**.
  - Garder chaque passage franchissable avec les réglages par défaut (marges de sûreté).
- **Ajouter** ce niveau à la **séquence** (`demo3.json`) après `demo2.json` (`EX-LVL-010`).
- **Tests** :
  - **Intégration** : le niveau est franchissable avec un scénario d'entrées utilisant les
    mécaniques (dash / wall jump / double saut aux bons instants) → `Won`, jamais `Lost`.
  - **Système** : le test de **parcours complet** enchaîne `demo` → `demo2` → `demo3` jusqu'à la fin
    de séquence.

## Fichiers impactés
- `Source/Elements/Levels/demo3.json` ; `Source/HMI/main.cpp` (ajout à la séquence).
- `Source/Test/Integration/test_physique_personnage.cpp` et `Source/Test/Systeme/test_parcours_complet.cpp`.

## Tests (obligatoires)
- `demo3.json` franchissable avec le scénario mécanique (déterministe, `EX-NFR-002`).
- La séquence complète (`demo` → `demo2` → `demo3`) est franchie par le test système.

## Points d'attention
- **Faisabilité** : chaque obstacle doit être franchissable avec les réglages (hauteurs, portée de
  dash) — garder des marges pour ne pas dépendre d'un timing à la frame près.
- **Scénario de test robuste** : privilégier des entrées « généreuses » (répéter dash/saut) plutôt
  qu'un timing fragile.
- **Tableau à caméra fixe** : le niveau tient dans un écran.
- Ne pas casser les preuves des niveaux 1 et 2.

## Définition de fait (DoD)
- Niveau parkour livré et enchaîné ; franchissabilité prouvée (intégration) et parcours complet
  (système) verts ; build `/W4 /WX`, `CHANGELOG.md` à jour.

## Exigences
`EX-LVL-001`, `EX-LVL-004`, `EX-LVL-010`, `EX-GP-015`, `EX-GP-016`, `EX-GP-017`, `EX-NFR-002`.
