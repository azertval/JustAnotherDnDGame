# Elements/UI/

Assets **Qt déclaratifs** de l'IHM, éditables hors code (Qt Designer) et compilés par la cible
`JustAnotherDnDGame` (`AUTOUIC`/`AUTORCC`, cf. `Source/HMI/CMakeLists.txt`, propriété
`AUTOUIC_SEARCH_PATHS`).

- Un `.ui` par écran, panneau et boîte de dialogue ; `uic` génère les en-têtes `ui_*.h` inclus par
  les widgets correspondants (`Source/HMI/Interface/`, `Source/HMI/Editor/`).
- Depuis le `LOT-68`, **toute** mise en page vit ici : les deux boîtes de dialogue
  (`ResizeDialog.ui`, `ShortcutsDialog.ui`) et les quatre panneaux (`PalettePanel.ui`,
  `LinkPanel.ui`, `PixelHistoryPanel.ui`, `PixelPalettePanel.ui`) qui étaient encore bâtis bouton
  par bouton en C++ ont rejoint le dossier. Le C++ ne garde que le fonctionnel — modèles,
  connexions, retraduction.
- `MainWindow.ui` déclare aussi la **barre de menus** complète, ses sous-menus et les deux barres
  d'outils. Restent insérées en code les seules choses réellement dynamiques : les actions du
  catalogue et les bascules de visibilité des panneaux.

> Piège XML : `--` est **interdit à l'intérieur d'un commentaire**, et `uic` le refuse sans
> indulgence. Employer le tiret cadratin.
- `resources.qrc` — ressource Qt embarquée ; référence les **deux** feuilles de style de
  `../Themes/` (alias `:/resources/theme-identity.qss` et `:/resources/theme-editor.qss`), une par
  portée depuis le `LOT-73` (`EX-IHM-082`). Voir `../Themes/README.md` pour ce qui distingue les
  deux et pourquoi elles ne sont plus un seul fichier.
