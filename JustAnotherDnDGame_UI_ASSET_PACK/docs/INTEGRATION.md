# Integration

1. Ajouter `qrc/mercenary_rpg_ui_kit.qrc` à la cible Qt.
2. Importer les composants QML depuis le dossier `qml/`.
3. Les SVG sont organisés physiquement sous `assets/`.
4. Le QRC expose deux chemins :
   - canonique : `qrc:/MercenaryRpgUiKit/assets/...`
   - compatibilité avec les kits précédents : `qrc:/MercenaryRpgUiKit/svg/...`

Exemple canonique :

    Image {
        source: "qrc:/MercenaryRpgUiKit/assets/inventory_inventory/longsword.svg"
    }

Exemple compatible avec les composants historiques :

    Image {
        source: "qrc:/MercenaryRpgUiKit/svg/inventory_inventory/longsword.svg"
    }

Pour un projet Qt Design Studio utilisant `.ui.qml`, ne pas mettre de fonctions
JavaScript arbitraires directement dans le fichier `.ui.qml`. Exposer les valeurs
calculées comme propriétés ou les gérer dans le modèle / un composant autorisé.

Le pack ne redistribue pas les fichiers binaires des polices ; voir `fonts/README.md`.
