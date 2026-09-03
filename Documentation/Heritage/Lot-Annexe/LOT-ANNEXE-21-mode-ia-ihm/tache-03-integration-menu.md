# TACHE-03 — Intégration au menu principal {#lot-annexe-21-tache-03-integration-menu}

**Lot :** [LOT-ANNEXE-21](epic.md) · **Emplacement :** `Source/HMI/Interface` · **Statut :** à faire

## Contexte
`TACHE-02` fournit `AiModeScreen` ; cette tâche le rend accessible depuis le menu principal, à la
place de l'ancienne entrée « Regarder l'IA jouer » (`LOT-ANNEXE-18`), dont l'onglet Rejeu de
`AiModeScreen` reprend le rôle.

## Travail à réaliser
- **`Elements/UI/MainMenu.ui`** : retire `watchAiButton`, ajoute `aiModeButton` (« Mode IA »).
- **`HMI/Interface/MainMenu.h/.cpp`** : retire le signal `watchAiRequested`, ajoute
  `aiModeRequested`.
- **`HMI/Interface/MainWindow.h/.cpp`** : retire `watchAiPlay()` (logique de sélection de fichier
  de rejeu, désormais dans l'onglet Rejeu de `AiModeScreen`), ajoute `openAiMode()`/`closeAiMode()`
  (transition d'écran, même patron que `openCredits`/`closeCredits`).
- **`Elements/Localization/en.lang`/`fr.lang`** : `menu.watch_ai` → `menu.ai_mode`.

## Fichiers impactés
- `Source/Elements/UI/MainMenu.ui` — modifié.
- `Source/HMI/Interface/MainMenu.h/.cpp` — modifié.
- `Source/HMI/Interface/MainWindow.h/.cpp` — modifié.
- `Source/Elements/Localization/en.lang`/`fr.lang` — modifiés.

## Tests (obligatoires)
- **Navigation complète** : Menu → Mode IA → Retour revient au menu ; aucune entrée « Regarder
  l'IA jouer » résiduelle.
- **Fermeture pendant un run actif** : quitter l'écran Mode IA (bouton Retour) pendant un
  entraînement actif ne plante pas — le run continue ou s'arrête proprement selon le choix retenu
  en TACHE-02 (`requestStop` avant destruction).

## Points d'attention
- **Aucune autre entrée de menu n'est réordonnée** : ce lot remplace une entrée par une autre au
  même endroit, il ne redispose pas le reste du menu.

## Définition de fait (DoD)
- Navigation fonctionnelle de bout en bout ; build `/W4 /WX` sans avertissement ; Doxygen à jour ;
  `EX-IA-022` déclarée dans l'`epic.md` du lot.

## Notions abordées
Aucune notion d'apprentissage automatique nouvelle : cette tâche est d'ordre IHM (navigation d'écran).
Le vocabulaire employé est défini dans @ref guide-annexe-apprentissage-renforcement.

## Exigences
`EX-IA-022` (nouvelle, du même lot).
