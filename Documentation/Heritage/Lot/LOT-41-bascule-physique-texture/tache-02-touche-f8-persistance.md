# TACHE-02 — Touche `F8` et persistance du mode {#lot-41-tache-02-touche-f8-persistance}

**Lot :** [LOT-41](epic.md) · **Emplacement :** `Source/HMI/Game`, `Source/HMI/Interface` · **Statut :** fait

## Contexte
Le mode introduit en TACHE-01 doit être basculable par le joueur comme par le level designer, et
retrouvé au lancement suivant. Deux mécanismes existent déjà et doivent être réutilisés tels quels :

- une **touche non remappable** gérée en dur dans `GameViewport::keyPressEvent` — précédent exact :
  `Qt::Key_F10`, qui bascule la grille de repère de l'éditeur ;
- une **préférence persistée** via `QSettings` — précédent exact : la disposition des docks
  (`MainWindow::saveLayout`/`restoreLayout`, `EX-IHM-011`) et la langue active.

## Travail à réaliser
- **`F8`** dans `GameViewport::keyPressEvent`, en dur, bascule `RenderMode`. Actif en édition, en
  essai et en jeu réel.
- **Aucune entrée dans les tables de remappage** : ni `hmi::GameAction`, ni `hmi::EditorAction`, ni
  la section correspondante de `keybindings.json`, ni l'UI de remappage. C'est une vérification à
  faire explicitement, pas une omission implicite.
- **Défaut `Texture`** dans toutes les configurations de build. **Supprimer** toute branche
  `core::kDeveloperBuild` liée au mode de rendu (le cadrage initial en prévoyait une).
- **Persistance** : lire le mode au démarrage et l'écrire à chaque bascule, via `QSettings`, sous une
  clé nommée. Une valeur absente ou invalide retombe silencieusement sur `Texture`.

## Fichiers impactés
- `Source/HMI/Game/GameViewport.{h,cpp}`.
- `Source/HMI/Interface/MainWindow.{h,cpp}` (lecture/écriture de la préférence, selon l'endroit où
  `QSettings` est déjà utilisé).

## Tests (obligatoires)
- Conversion mode ↔ valeur persistée : aller-retour, valeur absente, valeur invalide → défaut.
  Fonction **pure**, testée sans Qt ni GPU.
- Vérification que `F8` n'apparaît dans aucune table de remappage : contrôle sur les enums d'actions
  et sur les valeurs par défaut.

## Points d'attention
- `F8` ne doit pas entrer en conflit avec un raccourci Qt de la fenêtre principale ni être capturé
  par un widget ayant le focus. Vérifier le comportement quand un dock a le focus clavier.
- La bascule ne doit **rien** changer à la simulation : aucun pas fixe supplémentaire, aucune
  réinitialisation de niveau.
- Écrire la préférence à chaque bascule est acceptable (`QSettings` bufferise) ; ne pas l'écrire
  dans la boucle de rendu.

## Définition de fait (DoD)
- `F8` bascule dans les trois contextes, hors tables de remappage ; le défaut est `Texture` partout ;
  le choix survit à un redémarrage ; conversion testée ; `/W4 /WX` propre.

## Exigences
`EX-REN-046` (bascule, défaut unique, persistance) ; réutilise `EX-IHM-011` (persistance des
préférences d'interface), `EX-CTRL-012` (remappage — dont cette touche est explicitement exclue).
