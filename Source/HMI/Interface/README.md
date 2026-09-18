# HMI/Interface/

Widgets **Qt** du châssis de l'éditeur de cartes (`LevelEditor`) : la fenêtre principale, ses
actions et le système de design. Les mises en page sont décrites hors code dans `Source/Ui/Editor/*.ui`
(Qt Designer) et le thème dans `Elements/Themes/theme-editor.qss`. Les écrans du jeu, eux, sont en
Qt Quick (`Source/Ui/`, modèles dans `HMI/Runtime/`).

- `MainWindow` — fenêtre principale de l'éditeur : le canevas (`hmi::EditorViewport`) au centre ;
  la palette, le navigateur de cartes, les couches et les entités en docks détachables dont la
  disposition est persistée (`EX-IHM-010`/`011`). La fenêtre ne possède aucune donnée d'édition :
  le canevas est le seul propriétaire du brouillon, les panneaux demandent et il applique.
- `ActionCatalog` / `EditorActions` — les commandes de l'éditeur comme actions réutilisables
  (`EX-IHM-055`). Depuis le `LOT-68`, chaque action déclare sa **surface** (`ActionSurface`) : la
  barre d'outils ne porte que les outils et les commandes à usage continu, le reste vit au menu
  (`EX-IHM-074`).
- `DesignTokens` — jetons de design : source unique des couleurs, espacements, typographie et
  tailles (`EX-IHM-050`, `EX-IHM-051`).
- `ApplicationTheme` — application du système de design : style, palettes, feuille de style et
  polices (`applyEditorTheme`, `applyStyleSheet`, `applyFont`).
- `StyleSheetTemplate` — substitution des marqueurs d'un modèle de feuille de style par les jetons.
- `FontResolution` — résolution de la famille de police de l'IHM, logique pure (`EX-IHM-052`).
- `ThemeResolution` — résolution du thème clair/sombre effectif du châssis, logique pure
  (`EX-IHM-054`).
- `IconGeometry` / `ThemeIcons` — icônes dessinées par code : une géométrie **pure** décide *quoi*
  dessiner, un peintre Qt décide *comment* (`EX-IHM-055`).

Réf. specs : [`interface-ihm.md`](../../../Documentation/Specification/interface-ihm.md) (`EX-IHM-*`),
guide [`guide-ihm-qt`](../../../Documentation/Guide/guide-ihm-qt.md).
