# HMI/Game/

Le **jeu** partagé par l'application et l'éditeur : une carte qu'on parcourt, et ce qu'il faut pour
la dessiner — sans Qt.

- `WorldPlay` — joue l'exploration d'une carte : une `core::ExplorationSession`, la table
  d'apparence du lieu (`Scene/<lieu>/appearance.json`, `hmi::PlaceAppearance`) et les figurines
  posées dessus (les PNJ, puis le héros). `step()` avance d'un pas avec l'intention du joueur ;
  `snapshot()` rend l'instantané que dessine `hmi::WorldSceneRenderer`. Le jeu (`hmi::WorldModel`)
  et l'essai immédiat de l'éditeur (`hmi::EditorViewport`) s'en servent tous deux, si bien que
  l'essai montre exactement ce que le jeu montrera (`EX-EDIT-055`) ; chaque appelant garde sa
  cadence et ses signaux.

Réf. : guide [`guide-ihm-qt`](../../../Documentation/Guide/guide-ihm-qt.md),
[`guide-boucle`](../../../Documentation/Guide/guide-boucle.md).
