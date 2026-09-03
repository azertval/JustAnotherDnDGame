# TACHE-03 — Retrait du legacy UI (écrans maison, fenêtre Win32, ancienne boucle) {#lot-38-tache-03-retrait-legacy-ui}

**Lot :** [LOT-38](epic.md) · **Emplacement :** `Source/HMI`, `Source/Editor`, CMake · **Statut :** non commencé

## Contexte
Une fois menus/options/remappage portés (TACHE-01/02), l'ancienne pile d'UI maison n'a **plus de rôle**
et la coexistence des deux exécutables (depuis le [LOT-34](@ref lot-34)) a joué son rôle de filet.
Cette tâche **retire le legacy** pour ne garder **qu'une seule technologie d'UI** (Qt), `HMI` se
réduisant à la **bibliothèque de rendu du jeu**. C'est un objectif explicite du lot (pas un effet de
bord) : il faut atteindre la **parité** avant de supprimer.

## Travail à réaliser
- **Supprimer** (après vérification de parité fonctionnelle) :
  - Écrans : `MenuScreen`, `OptionsScreen`, `EditorScreen`, `GameKeybindingsScreen`,
    `EditorKeybindingsScreen`, `GamepadBindingsScreen`, et le `GameScreen` **en tant qu'`IScreen`**
    (conserver la logique de rendu de jeu réutilisée par le viewport Qt, extraite au `LOT-34`).
  - Infrastructure d'écrans : `IScreen`, `ScreenManager`, `ScreenTransition`, `RenderContext` si
    devenu inutile (ou adapté).
  - Widgets éditeur maison : `HMI/Editor/` (`TilePalette`, `ToolBar`, `LevelPicker`, `TextInputField`,
    validations si migrées, `EditorLayout`, `EditorLog`) — **sauf** la logique pure déjà extraite et
    réutilisée (taxonomie, validations) qui vit désormais côté partagé/`Editor`.
  - Dessin d'UI maison : usages `BitmapFont` pour l'UI (la police reste si le rendu de jeu s'en sert ;
    sinon retirer).
  - Plateforme : `hmi::Window` (Win32) et l'**ancienne boucle** `Source/HMI/main.cpp` ; l'exécutable
    `JustAnotherDnDGame` legacy est supprimé au profit de `JustAnotherDnDGame`.
- **CMake** : retirer la cible exe legacy, nettoyer les sources supprimées ; `JustAnotherDnDGame`
  devient l'unique exécutable et hérite de la copie des assets (`Levels`, `Localization`).
- **Release** : basculer `release.yml` sur `JustAnotherDnDGame` + `windeployqt` (préparé au
  `LOT-34`), retirer toute référence à l'ancien exe.

## Fichiers impactés
- Suppression sous `Source/HMI/Interface/` (écrans + manager), `Source/HMI/Editor/`,
  `Source/HMI/Platform/Window.{h,cpp}`, `Source/HMI/main.cpp`.
- `Source/HMI/CMakeLists.txt`, `Source/Editor/CMakeLists.txt`, `Source/CMakeLists.txt`,
  `.github/workflows/release.yml`.

## Tests (obligatoires)
- **Aucune référence morte** : le projet compile sans les fichiers retirés (`/W4 /WX`, pas de symbole
  manquant).
- **Suite de tests** : retirer/adapter les tests d'UI legacy devenus sans objet (`Source/Test/Unit/
  HMI/Editor/`, `test_editor_keybindings_model` conservé si le modèle l'est) ; **conserver** les tests
  de logique migrée. La suite reste **verte**.
- **Non-régression jeu** : le rendu de jeu (réutilisé par le viewport) et son déterminisme sont
  inchangés ; parcours système (`test_parcours_complet`) vert.
- **Vérification manuelle** : l'unique exécutable Qt couvre menu/jeu/éditeur/options/remappage.

## Points d'attention
- **Parité d'abord** : ne supprimer un écran/chemin que lorsque son équivalent Qt est vérifié — sinon
  perte de fonctionnalité.
- **Distinguer rendu de jeu (à garder) et UI (à retirer)** dans `HMI` : `GameScreen` mélange les deux ;
  s'appuyer sur l'extraction faite au `LOT-34` TACHE-04.
- **Suppressions Git massives** : procéder par étapes vérifiables (compilation + tests entre chaque
  bloc de suppression) pour isoler toute régression.

## Définition de fait (DoD)
- Tout le legacy UI supprimé, un **seul exécutable** (`JustAnotherDnDGame`), `HMI` réduit au rendu de
  jeu ; aucune référence morte ; suite de tests verte ; release basculée ; `/W4 /WX`, Doxygen, lint
  verts ; vérification manuelle OK.

## Exigences
`EX-IHM-041` (UI hors-jeu unifiée, legacy retiré) ; préserve `EX-NFR-002` (déterminisme jeu),
`EX-NFR-010` (frontière `HMI → Core`).
