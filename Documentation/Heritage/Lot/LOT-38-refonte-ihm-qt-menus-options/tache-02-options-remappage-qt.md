# TACHE-02 — Options Qt (V-Sync, langue) + remappage jeu/éditeur/manette {#lot-38-tache-02-options-remappage-qt}

**Lot :** [LOT-38](epic.md) · **Emplacement :** `Source/Editor` · **Statut :** non commencé

## Contexte
Porte en Qt l'écran **Options** (`OptionsScreen` : V-Sync, langue) et les trois écrans de **remappage**
(`GameKeybindingsScreen`, `EditorKeybindingsScreen`, `GamepadBindingsScreen`). Les **modèles** sous-
jacents (`GameKeyBindings`, `EditorKeyBindings`, `GamepadBindings`, `Localization`) sont déjà découplés
du rendu et **réutilisés tels quels** ; seule la présentation change. Le format `keybindings.json` et
les catalogues `.lang` **ne changent pas**.

## Travail à réaliser
- **Options Qt** (`QDialog`/vue) :
  - **V-Sync** : case à cocher pilotant `hmi::GraphicsDevice` (activable, `EX-REN-022`).
  - **Langue** : sélection parmi les catalogues chargés (`Localization`), application immédiate des
    libellés.
  - Persistance des réglages selon le mécanisme existant.
- **Remappage** (trois vues, une par jeu de bindings) :
  - Afficher chaque action et sa touche/bouton courant ; **capturer** une nouvelle touche
    (`keyPressEvent`) ou un bouton manette (sondage XInput) ; refuser/avertir sur conflit selon la
    règle existante.
  - Écrire via le chemin d'enregistrement existant des `…KeyBindings` → `keybindings.json`, sections
    `jeu`/`editeur`/`manette` inchangées.
  - Le panneau d'aide de l'éditeur reflète toujours la touche réellement liée (parité).
- **Localisation** de tous les libellés via `.lang`.

## Fichiers impactés
- `Source/Editor/OptionsDialog.{h,cpp}`, `Source/Editor/KeybindingsView.{h,cpp}` (générique pour les
  trois jeux de bindings).
- Câblage vers `GraphicsDevice`, `Localization`, `…KeyBindings`.

## Tests (obligatoires)
- **Non-régression modèles** : les `…KeyBindings` (chargement/enregistrement) et la détection de
  conflit restent testés (réutiliser `test_editor_keybindings_model.cpp`) — pas de logique dupliquée.
- **Logique de capture testable** : conversion d'un événement de capture en binding, détection de
  conflit, sans Qt fenêtré.
- **Vérification manuelle** : changer V-Sync (effet visible), changer de langue (libellés), remapper
  une action jeu/éditeur/manette et vérifier la persistance après relance.

## Points d'attention
- **Formats inchangés** : `keybindings.json` (sections jeu/editeur/manette) et `.lang` — aucune
  migration.
- **Capture manette** : réutiliser le sondage XInput (throttling `LOT-33`) pour lire un bouton pressé.
- **Conflits de touches** : conserver exactement la règle existante (refus/avertissement).

## Définition de fait (DoD)
- Options (V-Sync, langue) et remappage (jeu/éditeur/manette) en Qt, équivalents aux écrans
  historiques, réglages persistés ; logique de capture **testée**, modèles non dupliqués ; `/W4 /WX`
  propre ; vérification manuelle OK.

## Exigences
`EX-IHM-040` ; reconduit `EX-REN-022` (V-Sync), `EX-CTRL-012` (remappage éditeur) et le remappage
jeu/manette (`LOT-29`/`30`) ; réutilise la localisation (`LOT-06`).
