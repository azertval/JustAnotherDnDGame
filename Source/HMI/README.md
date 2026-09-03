# Source/HMI/

Couche de **présentation** : l'unique application du projet, `JustAnotherDnDGame` — une application **Qt**
qui embarque le **rendu du jeu** (via **QRhi**) dans un viewport et l'entoure de l'IHM (menu, options,
éditeur de niveau). Depuis le `LOT-38`, l'IHM « maison » et l'exécutable historique ont été retirés :
ce dossier est la seule cible exécutable (voir `CMakeLists.txt`).

Ce dossier dépend de `../Core/` pour l'état à afficher, mais ne contient pas la logique de jeu
elle-même. Les **assets Qt déclaratifs** (mises en page `.ui`, ressource `.qrc`, thème `.qss`) vivent
hors code dans [`../Elements/`](../Elements/README.md) (`UI/`, `Themes/`).

## Découpage par domaine

| Dossier | Rôle |
|---|---|
| [`Platform/`](Platform/README.md)   | Provisionnement bas niveau (répertoire de l'exécutable). |
| [`Input/`](Input/README.md)         | Entrées : état, mapping, remappage clavier/manette, pont Qt→`Key`. |
| [`Graphics/`](Graphics/README.md)   | Rendu via **QRhi** (pipeline 2D, sprites, caméra, brouillon d'édition). |
| `Game/`      | Simulation d'un niveau (`GameSession`) et viewport Qt jeu/édition (`GameViewport`). |
| [`Localization/`](Localization/README.md) | Catalogue de traduction. |
| `Interface/` | Fenêtre principale, menu, options, remappage (widgets Qt). |
| [`Editor/`](Editor/README.md)       | Périmètre éditeur de niveau (palette, outils, navigateur, logique pure). |
| [`Audio/`](Audio/README.md)         | Réservé (audio hors périmètre au MVP). |

## Build & déploiement

Cible Qt **optionnelle** : sans Qt, la configuration CMake n'échoue pas (les tests unitaires
compilent les sources pures directement). `windeployqt` copie les DLL Qt, le plugin de plateforme et
le runtime du compilateur à côté de l'exécutable — **aucune bibliothèque à installer** côté
utilisateur. Provisionnement de Qt : [`../../External/README.md`](../../External/README.md).

Guide détaillé : [`guide-ihm-qt`](../../Documentation/Guide/guide-ihm-qt.md).
