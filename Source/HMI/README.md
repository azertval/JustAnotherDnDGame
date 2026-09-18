# Source/HMI/

Couche de **présentation** : ce que le jeu (`JustAnotherRpgGame`, Qt Quick, point d'entrée dans
`../App/Game/`) et l'éditeur de cartes (`LevelEditor`, Qt Widgets, défini ici) partagent — rendu du
jeu via **QRhi**, entrées, traduction —, puis l'éditeur lui-même. La partie sans Qt Widgets est
la bibliothèque statique `HmiLib`, que consomment aussi les tests (voir `CMakeLists.txt`).

Ce dossier dépend de `../Core/` pour l'état à afficher, mais ne contient pas la logique de jeu
elle-même. Les **assets Qt déclaratifs** (mises en page `.ui`, ressource `.qrc`, thème `.qss`) vivent
hors code dans [`../Elements/`](../Elements/README.md) (`UI/`, `Themes/`) ; les écrans du jeu (QML)
dans `../Ui/`.

## Découpage par domaine

| Dossier | Rôle |
|---|---|
| [`Platform/`](Platform/README.md)   | Provisionnement bas niveau (répertoire de l'exécutable, minidump). |
| [`Input/`](Input/README.md)         | Entrées : état, manette, raccourcis de l'éditeur, pont Qt→`Key`. |
| [`Graphics/`](Graphics/README.md)   | Rendu via **QRhi** (pipeline 2D, caméra, lieu, arène, brouillon d'édition). |
| [`Game/`](Game/README.md)           | La carte qu'on parcourt (`WorldPlay`), partagée par le jeu et l'essai de l'éditeur. |
| `Presentation/` | Logique de présentation pure (enchaînement des écrans, échelle, valeurs de fiche, crédits). |
| `Runtime/`   | Les types C++ que les écrans du jeu voient : module QML `Jadg.Runtime` (vues-modèles, surfaces de rendu). |
| [`Localization/`](Localization/README.md) | Catalogue de traduction. |
| [`Interface/`](Interface/README.md) | Fenêtre de l'éditeur, actions, thème (widgets Qt). |
| [`Editor/`](Editor/README.md)       | Périmètre éditeur de cartes (canevas, panneaux, logique pure). |
| [`Audio/`](Audio/README.md)         | Moteur de lecture et volume ; aucun son livré. |

## Build & déploiement

Cible Qt **optionnelle** : sans Qt, la configuration CMake n'échoue pas (les tests unitaires
compilent les sources pures directement). `windeployqt` copie les DLL Qt, le plugin de plateforme et
le runtime du compilateur à côté de l'exécutable — **aucune bibliothèque à installer** côté
utilisateur. Provisionnement de Qt : [`../../External/README.md`](../../External/README.md).

Guide détaillé : [`guide-ihm-qt`](../../Documentation/Guide/guide-ihm-qt.md).
