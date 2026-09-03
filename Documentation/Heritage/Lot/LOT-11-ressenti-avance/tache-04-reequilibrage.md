# TACHE-04 — Rééquilibrage des niveaux + preuves à la vraie taille {#lot-11-tache-04-reequilibrage}

**Lot :** [LOT-11](epic.md) · **Emplacement :** `Source/Elements/Levels` · **Statut :** à faire

## Contexte
Les niveaux `demo`/`demo2`/`demo3` ont été conçus pour un personnage **1×1** ; avec un humanoïde
**0,4 × 0,8** (et la gravité asymétrique), certains passages changent (le couloir de `demo3` fait
pile une tuile de haut, etc.). On **rééquilibre** au besoin et on **prouve** la franchissabilité
avec la **vraie taille** du personnage.

## Travail à réaliser
- **Tests à la vraie taille** : faire apparaître le personnage des tests de franchissabilité
  (`playLevelFile`, parcours **système**) via `core::playerSize()` + `core::playerSpawnPosition`
  (au lieu d'un 1×1 posé au coin), pour éprouver le **vrai** personnage.
- **Rééquilibrer** les niveaux jusqu'à ce que ces preuves repassent au vert :
  - vérifier que chaque niveau reste **franchissable** avec sa mécanique (déplacement, saut, dash) ;
  - vérifier que les propriétés « **requiert** la mécanique » tiennent encore (ex. `demo2` requiert
    le saut, `demo3` requiert le dash) ; ajuster hauteurs/largeurs si la nouvelle taille les casse.
- Les **tests de physique génériques** (hauteur de saut, wall slide, dash…) peuvent conserver un
  personnage 1×1 (ils testent la physique, pas le personnage précis).

## Fichiers impactés
- `Source/Elements/Levels/*.json` (ajustements éventuels).
- `Source/Test/Integration/test_physique_personnage.cpp` (spawn des tests de niveaux à la vraie
  taille), `Source/Test/Systeme/test_parcours_complet.cpp`.

## Tests (obligatoires)
- `demo` / `demo2` / `demo3` **franchissables** à la vraie taille (avec la mécanique attendue).
- Propriétés « requiert la mécanique » préservées (`demo2` sans saut → échec, `demo3` sans dash →
  échec).
- Parcours **système** complet (`demo → demo2 → demo3`) vert.

## Points d'attention
- **Mesurer, pas deviner** : ajuster les niveaux en s'appuyant sur les tests (itérer jusqu'au vert).
- **Marges** : garder de la marge pour ne pas dépendre d'un pixel/frame près (la taille change les
  contacts).
- Ne pas casser les tests de mécanique génériques.

## Définition de fait (DoD)
- Niveaux franchissables à la vraie taille (preuves intégration + système vertes) ; build `/W4 /WX`,
  `CHANGELOG.md` à jour.

## Exigences
`EX-LVL-004`, `EX-LVL-010`, `EX-GP-018`, `EX-NFR-002`, `EX-NFR-020`.
