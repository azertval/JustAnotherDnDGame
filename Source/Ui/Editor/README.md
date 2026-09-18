# Ui/Editor/

Assets **Qt déclaratifs** de l'éditeur, éditables hors code (Qt Designer) et compilés par la cible
`LevelEditor` (`AUTOUIC`/`AUTORCC`, cf. `Source/HMI/CMakeLists.txt`, propriété
`AUTOUIC_SEARCH_PATHS`).

- Un `.ui` par fenêtre, panneau et boîte de dialogue ; `uic` génère les en-têtes `ui_*.h` inclus par
  les widgets correspondants (`Source/HMI/Interface/`, `Source/HMI/Editor/`) : `MainWindow.ui`,
  les deux boîtes de dialogue (`ResizeDialog.ui`, `ShortcutsDialog.ui`) et les quatre panneaux
  (`PalettePanel.ui`, `LevelBrowserPanel.ui`, `LayersPanel.ui`, `EntityPanel.ui`). Depuis le
  `LOT-68`, **toute** mise en page vit ici ; le C++ ne garde que le fonctionnel — modèles,
  connexions, retraduction.
- `MainWindow.ui` déclare aussi la **barre de menus** complète, ses sous-menus et les barres
  d'outils. Restent insérées en code les seules choses réellement dynamiques : les actions du
  catalogue et les bascules de visibilité des panneaux.

> Piège XML : `--` est **interdit à l'intérieur d'un commentaire**, et `uic` le refuse sans
> indulgence. Employer le tiret cadratin.
- `resources.qrc` — ressource Qt embarquée ; référence la feuille de style du châssis d'édition,
  `../../Elements/Themes/theme-editor.qss` (alias `:/resources/theme-editor.qss`, `EX-IHM-082`). Voir
  `../../Elements/Themes/README.md`.
