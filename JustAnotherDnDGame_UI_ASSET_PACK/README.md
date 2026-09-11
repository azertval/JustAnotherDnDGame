# JADG — UI Asset Preview

Petit projet Qt 6 / QML pour prévisualiser le contenu de `JADG_UI_ASSET_PACK_FINAL` dans une seule fenêtre.

## Écrans

- Compétences : attaques/sortilèges, 9 catégories de sorts, détail sélectionné
- Équipe mercenaire : écran complet issu du pack
- Carte : world map + contrôles du pack
- Menu principal
- Options
- Crédits
- Galerie des assets SVG réutilisables

## Pré-requis

- Qt 6.5+ avec `Core`, `Gui`, `Qml`, `Quick`, `QuickControls2`
- CMake 3.21+
- Compilateur C++17

## Compiler

```bash
cmake -S . -B build
cmake --build build --config Release
```

Puis lancer `JADG_UI_ASSET_PREVIEW` depuis `build` (le chemin exact dépend de la plateforme/configuration).

Le projet embarque les assets dans un `.qrc`, donc aucune copie d'asset n'est nécessaire après compilation.
