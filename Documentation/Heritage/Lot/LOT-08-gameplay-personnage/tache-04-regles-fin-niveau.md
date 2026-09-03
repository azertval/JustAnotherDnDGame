# TACHE-04 — Règles de fin de niveau (succès / échec) {#lot-08-tache-04-regles-fin-niveau}

**Lot :** [LOT-08](epic.md) · **Emplacement :** `Source/Core/Levels` · **Statut :** à faire

## Contexte
Une fois le personnage mobile, il faut **statuer sur l'issue** du niveau. Cette logique est
**pure** (géométrie de recouvrement contre le modèle de niveau), sans rendu ni entrée, donc
testable en isolation. Elle ne **déclenche** rien (menu, réinitialisation) : elle **classe** l'état
courant ; l'intégration (TACHE-06) agira sur le verdict.

## Travail à réaliser
- **`LevelOutcome`** (`enum class`) : `Playing`, `Won`, `Lost`.
- **Fonction d'évaluation pure** : à partir de la boîte du personnage (`Aabb`), du `Level`
  (positions de sortie, tuiles `Danger`) et des **limites** de la grille, renvoyer l'issue :
  - **`Won`** si le personnage recouvre la tuile de **sortie** (`EX-GP-030`).
  - **`Lost`** s'il recouvre une tuile **`Danger`** ou s'il **sort par le bas** du niveau
    (`y` au-delà de la limite basse) — `EX-GP-031`.
  - **`Playing`** sinon.
- Priorité déterministe si recouvrement simultané (définir et documenter l'ordre, p. ex. échec
  prioritaire sur succès), pour un comportement reproductible (`EX-NFR-002`).

## Fichiers impactés
- `Source/Core/Levels/LevelOutcome.h` (+ `.cpp` si nécessaire) — nouveau.
- `Source/Core/CMakeLists.txt`, `Source/Test/CMakeLists.txt`.

## Tests (obligatoires)
- Personnage sur la **sortie** → `Won`.
- Personnage sur une tuile **`Danger`** → `Lost`.
- Personnage **sous la limite basse** → `Lost`.
- Personnage en zone libre → `Playing`.
- **Simultané** sortie + danger → issue déterministe documentée (échec prioritaire).

## Points d'attention
- **Donnée pure** : pas de rendu, pas d'entrée, pas de `World` requis — travaille sur `Aabb` +
  `Level`. (Le lien avec l'entité personnage se fait dans l'intégration.)
- Cohérence du recouvrement tuile↔boîte avec le repère (origine haut-gauche, tuile = 1 unité).
- Ne pas **muter** le niveau ni l'état : fonction d'observation.

## Définition de fait (DoD)
- `LevelOutcome` + évaluateur documentés et **testés** (`ctest` vert) ; build `/W4 /WX`.

## Exigences
`EX-GP-030`, `EX-GP-031`, `EX-NFR-002`, `EX-NFR-010`, `EX-ARCH-011`.
