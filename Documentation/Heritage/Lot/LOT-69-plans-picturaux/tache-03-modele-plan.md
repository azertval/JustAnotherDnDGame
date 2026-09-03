# TACHE-03 — Modèle de plan, format de niveau et brouillon annulable {#lot-69-tache-03-modele-plan}

**Lot :** [LOT-69](epic.md) · **Emplacement :** `Source/Core/Levels` · **Statut :** fait

## Contexte
`core::Decor` décrit un objet posé : `assetName`, `position`, `scale`, `rotation`, `layer`,
`manipulable`. Un plan n'est rien de tout cela — il couvre le niveau entier, n'a ni position ni
rotation, et porte à la place une densité, des facteurs de défilement et une profondeur. C'est un
type nouveau, pas une évolution de l'ancien.

Le nom `Plane` est libre : le code est en anglais (`Decor`, `Mechanism`, `DangerLink`,
`TileTextureOverride`), la documentation en français, et aucun symbole `Plane` n'existe dans
`Source`. « Layer » est déjà pris deux fois (`DecorLayer`, `RenderLayer`) et serait ambigu.

## Travail à réaliser
- Créer `Source/Core/Levels/Plane.h` — donnée **pure** (`EX-ARCH-011`), sur le patron de `Decor` :
  `fileName`, `pixelsPerUnit` (défaut 16), `parallaxX`/`parallaxY` (défaut 1), `opacity` (défaut 1),
  `depth` (`PlaneDepth::Behind`/`Front`, défaut `Behind`). L'**ordre du vecteur est significatif**,
  comme il l'était pour les décors.
- `Level` : `std::vector<Plane> _planes` et `bool _parallaxEnabled = true`, avec accesseurs.
- **Lecture** (`LevelLoader`) : tableau `"planes"` et drapeau `"parallax"`. Valider `pixelsPerUnit ∈
  {4, 8, 16}`, `fileName` non vide, facteurs finis, `opacity ∈ [0,1]`. Appliquer les garde-fous de
  `EX-DEC-044` : plafond de **16** plans, refus si `width × ppu` ou `height × ppu` dépasse **16384**
  (limite de dimension de texture, D3D11 feature level 11), avertissement journalisé au-delà d'un
  total raisonnable. Un `depth` inconnu retombe silencieusement sur `Behind`, comme le fait déjà
  l'axe d'un danger mobile.
- **Compatibilité** : rien à faire ici. Voir « Écart assumé » ci-dessous — le sort du champ
  `"decors"` est traité par `TACHE-04`, avec le retrait du système qu'il décrit.
- **Écriture** (`LevelWriter`) : émettre `"planes"`, omettre tout champ à sa valeur par défaut
  (convention du `LOT-67`), omettre `"parallax"` quand il vaut `true`, et ne **jamais** réémettre
  `"decors"` — charger puis enregistrer suffit donc à migrer un fichier.
- **Brouillon** (`LevelDraft`) : `addPlane`, `removePlane`, `setPlaneDensity`, `setPlaneParallax`,
  `setPlaneOpacity`, `setPlaneDepth`, `movePlaneForward`/`Backward`/`ToFront`/`ToBack`,
  `setLevelParallaxEnabled`. Reprendre les signatures des mutateurs de décor retirés, qui renvoyaient
  le nouveau rang en `std::optional<std::size_t>`.

## Fichiers impactés
`Source/Core/Levels/Plane.h` (nouveau), `Level.h`, `LevelLoader.{h,cpp}`, `LevelWriter.{h,cpp}`,
`LevelDraft.{h,cpp}`, `Source/Core/CMakeLists.txt`.

## Tests (obligatoires)
- `test_plane.cpp` (nouveau) : valeurs par défaut, invariants du type.
Regroupés dans un **`test_plane.cpp`** unique plutôt qu'éclatés entre `test_level_loader`,
`test_level_writer` et `test_level_draft` : le lecteur qui cherche « comment se comporte un plan »
trouve tout au même endroit, comme `test_decor.cpp` le faisait pour les décors.

- Valeurs par défaut du type ; densités acceptées et refusées.
- `addPlane` préserve l'ordre ; `removePlane` au rang donné, sans effet hors bornes.
- **Chaque** mutateur annulable *et* refaisable — le test qui empêche de reproduire l'oubli de
  `pushUndo()` corrigé après coup au `LOT-67`.
- **Un mutateur refusé n'empile aucun pas d'annulation** : densité invalide, opacité hors `[0,1]`,
  `NaN`, parallaxe infinie, rang hors bornes.
- Réordonnancement (avancer, reculer, premier, dernier), y compris aux extrémités.
- Chargement dans l'ordre déclaré, avec repli de `parallaxY` sur `parallaxX`.
- Round-trip complet ; champs par défaut absents du JSON ; niveau sans plan n'écrivant ni
  `"planes"` ni `"parallax"` ; drapeau de parallaxe survivant au round-trip.
- Garde-fous : densité invalide, trop de plans, plan dépassant la limite de texture — **et** le cas
  symétrique où la même largeur passe à une densité plus faible, pour montrer que c'est la
  *combinaison* qui est refusée.

## Écart assumé par rapport au cadrage initial

Le cadrage plaçait ici le traitement du champ obsolète `"decors"` (ignoré avec avertissement). Il a
été **déplacé vers `TACHE-04`** pendant l'implémentation, pour une raison qui n'apparaissait qu'une
fois le code sous les yeux : le faire ici rendrait cette tâche **destructive** — les décors
cesseraient d'être chargés alors que le type et son rendu existent encore, cassant les tests de
décor pour les rétablir deux tâches plus loin.

Séparées, les deux tâches deviennent nettes : `TACHE-03` est **purement additive** — tous les tests
préexistants passent sans une seule retouche — et `TACHE-04` porte l'intégralité du retrait, y
compris le sort du champ dans le format. Le comportement final est identique ; seul le découpage
change.

## Points d'attention
Chaque mutateur doit appeler `pushUndo()`. L'oubli n'est pas théorique : le `LOT-67` a dû corriger
après coup `setJumpBudget`/`setDashBudget`, seules propriétés de niveau non annulables du brouillon.
Le test « chaque mutateur est annulable » est ce qui empêche la récidive.

`pixelsPerUnit` est un `int` et non un `enum` : il sert **directement** au calcul des dimensions
attendues (`width × ppu`), et un `enum` imposerait une table de conversion à chaque site d'appel. La
contrainte `{4, 8, 16}` est validée au chargement, pas portée par le type.

Le constructeur de `Level` atteindra **19 paramètres**. Aucun lot ne l'a refactoré ; celui-ci n'est
pas le bon endroit, sa surface étant déjà maximale. Dette signalée, non traitée.

## Definition de fait (DoD)
Le format lit et écrit les plans, ignore proprement l'ancien champ, tous les niveaux livrés se
chargent, chaque mutateur est annulable. `ctest` à 100 %.

## Exigences
`EX-DEC-040`, `EX-DEC-041`, `EX-DEC-042`, `EX-DEC-044`, `EX-LVL-009`, `EX-LVL-004`, `EX-LVL-005`,
`EX-ARCH-011`, `EX-NFR-040`.
