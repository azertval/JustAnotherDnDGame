# Elements/Themes/

Feuille de style **Qt** (`.qss`) de l'éditeur, éditable hors code et embarquée via
`../../Ui/Editor/resources.qrc`. C'est un **modèle** : ses marqueurs sont substitués au chargement par les
jetons de `Editor/Logic/DesignTokens.h` — aucune couleur littérale n'y figure.

- `theme-editor.qss` — **châssis d'édition** : fenêtre, panneaux dockables, barres, arbres, tables,
  contrôles, boîtes de dialogue. **Variable** : suit le thème clair/sombre. Ses grandeurs `tokens.*`
  ne sont jamais multipliées — le châssis est un outil de travail dont les tailles suivent les
  réglages du système. Posée sur l'**application** par `hmi::applyStyleSheet`, elle ne change qu'au
  changement de thème.

Les écrans du jeu sont en Qt Quick (`Source/Ui/`) et ne lisent aucune feuille `.qss`.
