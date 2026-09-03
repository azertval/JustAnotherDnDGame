# LOT-38 — Refonte IHM (Qt) : menus, options, remappage & retrait du legacy UI {#lot-38}

> Statut : **✅ terminé**, livré en deux étapes fusionnées sur `main` (`55f161d5` — étape A : menu,
> options, jeu et remappage en Qt ; `8338bc15` — étape B : retrait du legacy, IHM Qt unifiée).
> Prérequis : [LOT-35](@ref lot-35) → [LOT-37](@ref lot-37) (éditeur Qt complet).

## Objectif
Achever la migration de **toute l'UI hors-jeu** vers Qt en portant le **menu principal**, l'écran
**Options** et les écrans de **remappage** (touches jeu, touches éditeur, manette), puis **retirer la
pile d'UI maison devenue morte** (écrans, `SpriteBatch` d'UI, police bitmap, `hmi::Window`, ancienne
boucle). À la fin du lot, il n'existe **qu'une seule technologie d'UI** (Qt) ; **seul le rendu
in-game reste D3D11**, et l'ancien exécutable `JustAnotherDnDGame` est supprimé au profit du nouveau.

## Périmètre

### Inclus
- **Menu principal Qt** : lancer le jeu, ouvrir l'éditeur, options, remappage, quitter, enregistrer
  les logs de session (réutilise `saveSessionLog` / `MemoryLogSink`).
- **Écran Options Qt** : V-Sync (`EX-REN-022`, pilote `GraphicsDevice`), langue (réutilise
  `Localization`), et autres réglages existants.
- **Remappage en Qt** : touches de jeu, touches d'éditeur, boutons de manette — réutilise
  `GameKeyBindings` / `EditorKeyBindings` / `GamepadBindings` (chargement/écriture `keybindings.json`
  inchangés) ; capture des touches/boutons via Qt.
- **Navigation** entre menu / jeu / éditeur gérée côté Qt (fenêtres/vues), remplaçant
  `IScreen` / `ScreenManager` / `ScreenTransition`.
- **Retrait du legacy** : suppression des `…Screen` (`MenuScreen`, `OptionsScreen`, `GameScreen` UI,
  `EditorScreen`, `…KeybindingsScreen`), `ScreenManager`, widgets `HMI/Editor/` maison
  (`TilePalette`, `ToolBar`, `LevelPicker`, `TextInputField`, dessin `BitmapFont` d'UI), `hmi::Window`
  et l'ancienne boucle `main.cpp`. `HMI` ne conserve que le **rendu du jeu** (device, sprite batch,
  sprite renderer, atlas, caméra, room grid, tile visuals).
- **Localisation** : les catalogues `.lang` restent la source des libellés, désormais consommés par
  les widgets Qt.
- Documentation (guides menu/options/contrôles réécrits pour Qt) et nettoyage des tests obsolètes /
  ajout des tests de la logique nouvelle.

### Exclus (hors périmètre de ce lot)
- **Refonte du rendu du jeu lui-même** — reste D3D11, inchangé.
- **Nouveaux réglages** non présents aujourd'hui — le périmètre fonctionnel des options/remappage est
  **reconduit**, pas étendu.
- **Textures depuis fichiers** — [LOT-39](@ref lot-39) (peut être fait avant ou après ce lot).
- **Traduction de nouvelles langues** — inchangé.

## Décisions de cadrage
- **Retirer le legacy seulement ici, après parité** : la coexistence des deux exécutables (depuis le
  [LOT-34](@ref lot-34)) a servi de filet ; une fois menus/options/remappage portés, l'ancien chemin
  n'a plus de raison d'être et son maintien coûterait double. Le retrait est un **objectif explicite**
  du lot, pas un effet de bord.
- **Réutiliser les modèles de bindings et de localisation** : `…KeyBindings` et `Localization` sont
  déjà découplés du rendu — seuls leurs **écrans** sont réécrits en Qt ; le format `keybindings.json`
  et les `.lang` ne changent pas.
- **Navigation par Qt, fin de `IScreen`/`ScreenManager`** : le patron d'écrans maison n'a plus lieu
  d'être une fois Qt maître de l'UI ; sa suppression réduit la surface de code.
- **`HMI` reste une bibliothèque de rendu** (issue du LOT-34) : ce lot en **retire** les parties UI,
  pas les parties rendu de jeu, que la cible Qt continue d'utiliser.

## Exigences couvertes
- Nouvelles : `EX-IHM-040` (menus/options/remappage en Qt), `EX-IHM-041` (UI hors-jeu unifiée sur une
  seule technologie ; pile d'UI maison retirée).
- Reconduites (présentation Qt, comportement conservé) : `EX-REN-022` (V-Sync activable),
  `EX-CTRL-012` (remappage éditeur), remappage jeu/manette (`LOT-29`/`30`), enregistrement des logs de
  session.
- Réutilisées : `EX-NFR-010` (frontière `HMI → Core`), catalogues de localisation (`LOT-06`…).

## Découpage

> État : ✅ fait · 🔄 en cours · ⬜ non commencé.

| Tâche | Intitulé | Emplacement | État |
|-------|----------|-------------|:----:|
| [TACHE-01](tache-01-menu-navigation-qt.md) | Menu principal Qt + navigation (remplace `IScreen`/`ScreenManager`) | `Source/HMI/Interface` | ✅ |
| [TACHE-02](tache-02-options-remappage-qt.md) | Options Qt (V-Sync, langue) + remappage jeu/éditeur/manette | `Source/HMI/Interface` | ✅ |
| [TACHE-03](tache-03-retrait-legacy-ui.md) | Retrait du legacy UI (`…Screen`, widgets maison, `hmi::Window`, *hmi::BitmapFont*, ancienne boucle) | `Source/HMI`, CMake | ✅ |
| [TACHE-04](tache-04-nettoyage-tests-doc.md) | Nettoyage tests, documentation (menu/options/contrôles) & vérification | `Source/Test`, `Documentation` | ✅ |

> La cible `Source/Editor` mentionnée au cadrage n'a jamais existé : la migration a abouti à un
> **exécutable unique** `JustAnotherDnDGame` construit depuis `Source/HMI`, avec l'interface répartie
> entre `Source/HMI/Interface` (menus, options, remappage) et `Source/HMI/Editor` (panneaux et
> gestes d'édition). Les emplacements ci-dessus reflètent le résultat livré.

## Critères d'acceptation du lot
1. Menu principal, Options (V-Sync, langue) et remappage (jeu, éditeur, manette) sont **entièrement
   en Qt** et **fonctionnellement équivalents** aux écrans historiques ; les réglages persistent
   (`keybindings.json`, langue).
2. La navigation menu ↔ jeu ↔ éditeur fonctionne sans l'ancienne pile `IScreen`/`ScreenManager`.
3. **Tout le legacy UI est supprimé** du code (écrans, `SpriteBatch` d'UI, `BitmapFont` d'UI,
   `HMI/Editor/` maison, `hmi::Window`, ancienne boucle `main.cpp`) ; il ne reste **qu'un seul
   exécutable**. `HMI` conserve uniquement le rendu du jeu.
4. Le rendu du jeu et son déterminisme sont **inchangés** ; la suite de tests (hors tests d'UI
   obsolètes retirés) passe à 100 %.
5. Aucune référence morte, aucun avertissement `/W4 /WX` ; la logique nouvelle (navigation, capture de
   binding) découplée de Qt est **testée**.
6. Doxygen et lint des exigences verts ; IHM Qt **vérifiée visuellement** (menus, options, remappage).

## Dépendances
- Bâtit sur [LOT-34](@ref lot-34) → [LOT-37](@ref lot-37). Réutilise `hmi::GameKeyBindings`/
  `EditorKeyBindings`/`GamepadBindings` (`LOT-29`/`30`), `hmi::Localization` (`LOT-06`),
  `GraphicsDevice` (V-Sync), `saveSessionLog`. **Retire** `hmi::Window`, `ScreenManager`, les `…Screen`
  et les widgets `HMI/Editor/`. Ne modifie pas `Core`.

## Navigation des tâches
- @subpage lot-38-tache-01-menu-navigation-qt
- @subpage lot-38-tache-02-options-remappage-qt
- @subpage lot-38-tache-03-retrait-legacy-ui
- @subpage lot-38-tache-04-nettoyage-tests-doc
