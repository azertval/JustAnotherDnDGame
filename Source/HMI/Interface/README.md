# HMI/Interface/

Widgets **Qt** de l'IHM hors-jeu : la fenêtre principale et les écrans qui ne relèvent pas de
l'éditeur de niveau. Les mises en page sont décrites hors code dans `Elements/UI/*.ui` (Qt Designer)
et le thème dans `Elements/Themes/` (une feuille par portée : `theme-identity.qss`,
`theme-editor.qss`).

- `MainWindow` — fenêtre principale : `QStackedWidget` (menu / options / éditeur) et panneaux
  dockables (`QDockWidget`) autour du viewport central. Depuis le `LOT-68`, ces panneaux sont
  répartis en **deux espaces de travail exclusifs** (`EditorWorkspace`, `EX-IHM-073`) : édition de
  niveau ou atelier de dessin d'assets, chacun persistant **sa** disposition (`EX-IHM-011`). Lance aussi la
  séquence de jeu (`startGame`).
- `EditorWorkspace` — table **pure** de la répartition panneaux/barres/menus entre les deux espaces.
- `ActionCatalog` / `EditorActions` — les commandes de l'éditeur comme actions réutilisables
  (`EX-IHM-055`). Depuis le `LOT-68`, chaque action déclare sa **surface** (`ActionSurface`) : la
  barre d'outils ne porte que les outils et les commandes à usage continu, le reste vit au menu
  (`EX-IHM-074`).
- `MainMenu` — menu principal (Jouer / Éditeur / Options / Quitter), depuis `MainMenu.ui`.
- `OptionsPage` — options en onglets, depuis `OptionsPage.ui` : **V-Sync** (`EX-REN-022`),
  l'affichage du **compteur de diagnostic** (`LOT-68`), le **volume**, un onglet **Général**
  (sélecteur de **langue** `EX-REN-033` + bouton **« Enregistrer les journaux »**) et le remappage
  clavier/manette (onglets ajoutés en code). Les sélecteurs de résolution et de limite d'images/s
  ont été **retirés** au `LOT-68` : grisés et non branchés, ils promettaient un réglage inexistant
  (`EX-IHM-072`). Émet `languageChanged`/`saveLogsRequested` vers `MainWindow`.
- `IdentityScale`, `ParchmentFrame`, `MenuBackdropGeometry`, `KeyHintText` — géométries et
  textes **purs** de l'habillage parchemin des écrans du jeu (`EX-IHM-070`), peints par
  `ParchmentPanel`, `MenuEntryButton` et `MainMenu`. Même découpage que les icônes du `LOT-56` :
  une fonction pure décide *quoi* dessiner, un peintre Qt décide *comment*.
- `KeybindingsWidget` / `GamepadBindingsWidget` — capture et affichage du remappage des touches et
  des boutons de manette (délèguent à `hmi::GameKeyBindings` / `EditorKeyBindings` /
  `GamepadBindings`, logique pure testée). Le remappage manette affiche aussi l'**état de connexion**
  (sondage XInput périodique tant que l'onglet est visible).

Réf. specs : [`interface-ihm.md`](../../../Documentation/Specification/interface-ihm.md) (`EX-IHM-*`),
guide [`guide-ihm-qt`](../../../Documentation/Guide/guide-ihm-qt.md).
