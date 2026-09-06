# Elements/Themes/

Feuilles de style **Qt** (`.qss`) de l'IHM, éditables hors code et embarquées via
`../UI/resources.qrc`. Ce sont des **modèles** : leurs marqueurs sont substitués au chargement par
les jetons de `HMI/Interface/DesignTokens.h` — aucune couleur littérale n'y figure.

Une feuille **par portée** depuis le `LOT-73` (`EX-IHM-082`), et non plus une seule appliquée à
toute l'application. Elles vivaient ensemble tant que rien n'y changeait en cours d'exécution ; le
facteur d'agrandissement du `LOT-68` a changé cela, et reposer la feuille applicative repolissait
alors ses 862 widgets — cinq secondes par redimensionnement en configuration Debug — pour une
préoccupation ne touchant que les écrans du jeu.

- `theme-identity.qss` — **identité du jeu**, portée par `objectName` (`#MainMenu`, `#OptionsPage`,
  `#PauseScreen`, `#LevelSelectScreen`, `#LevelCompleteScreen`, `#CreditsScreen`).
  **Invariante** : jamais affectée par le thème clair/sombre de l'éditeur (`EX-IHM-054`). Ses
  grandeurs `identity.size.*` / `identity.space.*` sont multipliées par le facteur entier de
  `hmi::identityScaleFor`. Posée par `MainWindow` sur la **pile d'écrans**, jamais sur l'application.
- `theme-editor.qss` — **châssis d'édition** : fenêtre, panneaux dockables, barres, arbres, tables,
  contrôles, boîtes de dialogue. **Variable** : suit le thème clair/sombre. Ses grandeurs `tokens.*`
  ne sont jamais multipliées — le châssis est un outil de travail dont les tailles suivent les
  réglages du système, là où les écrans du jeu sont une image agrandie. Posée sur l'**application**
  par `hmi::applyStyleSheet`, elle ne change qu'au changement de thème.

> Les deux feuilles sont **disjointes**, et un test le vérifie
> (`ApplicationThemeTest.LesDeuxPorteesSontDansDeuxFichiersDisjoints`) : aucun jeton `editor.color.*`
> dans l'identité, aucun jeton `identity.*` dans le châssis. Une seule règle d'identité replacée dans
> la feuille du châssis ramènerait le rejeu applicatif complet.
