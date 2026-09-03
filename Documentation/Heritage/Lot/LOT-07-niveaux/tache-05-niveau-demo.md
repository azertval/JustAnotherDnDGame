# TACHE-05 — Niveau de démonstration {#lot-07-tache-05-niveau-demo}

**Lot :** [LOT-07](epic.md) · **Emplacement :** `Source/Elements/Levels` · **Statut :** à faire

## Contexte
Le chargement (TACHE-03/04) a besoin d'un **vrai fichier** à ouvrir, et l'intégration (TACHE-06)
d'un niveau à afficher. Cette tâche fournit **un** niveau de démonstration valide, versionné dans
`Source/Elements/Levels` (`EX-LVL-001`), et le rend disponible à l'exécution.

## Travail à réaliser
- Créer un fichier de niveau au **format JSON** (liste de tuiles-objets, `EX-LVL-003`), **valide**
  au sens de la TACHE-04 : tuiles dans les bornes, une entrée et une sortie, et au moins **un
  mécanisme** interrupteur↔porte (`switch`/`door` liés par identifiant) pour exercer le
  chargement des liaisons.
- Ajouter un `README` à `Source/Elements/Levels` (format, types de tuiles, où sont les niveaux).
- **CMake** : copier `Source/Elements/Levels/*` à côté de l'exécutable (comme les catalogues de
  traduction `.lang`), pour un chargement par chemin relatif au binaire.

## Fichiers impactés
- `Source/Elements/Levels/<nom>.json` (nouveau), `Source/Elements/Levels/README.md` (nouveau).
- `Source/HMI/CMakeLists.txt` (copie post-build des niveaux).

## Vérifications (obligatoires)
- Le fichier **se charge et se valide** sans erreur (via le chargeur/les tests, ou au lancement).
- Le niveau est **franchissable sur le papier** (une entrée, une sortie atteignable) — la
  franchissabilité *automatisée* (`EX-NFR-021`) relève d'un lot de gameplay ultérieur.
- Les fichiers de niveaux sont bien **copiés** à côté de l'exécutable.

## Points d'attention
- Rester **simple** : un petit niveau (≈ 12×8) illustrant tuiles solides, un danger, une entrée,
  une sortie et une paire interrupteur/porte.
- Coordonnées `x` = colonne, `y` = ligne, origine haut-gauche ; liaison `switch.id` ↔
  `door.opensWith` (cf. `niveaux.md`).
- Un seul niveau ici : l'enchaînement et les 3 niveaux du MVP (`EX-LVL-010`…`EX-LVL-012`) sont
  hors périmètre.

## Définition de fait (DoD)
- Un niveau de démo valide est fourni et copié à côté de l'exécutable ; documenté.

## Exigences
`EX-LVL-001`, `EX-LVL-003`, `EX-LVL-012` (partiel : un niveau).
