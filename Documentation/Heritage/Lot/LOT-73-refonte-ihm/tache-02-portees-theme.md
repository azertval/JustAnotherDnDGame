# TACHE-02 — Le rejeu de thème cesse d'être global {#lot-73-tache-02-portees-theme}

**Lot :** [LOT-73](epic.md) · **Emplacement :** `Source/HMI/Interface`, `Source/Elements/Themes` ·
**Statut :** fait

## Contexte
Le `LOT-56` a défini deux **portées** d'habillage — l'identité du jeu, invariante, et le châssis
d'édition, variable — mais les a écrites dans une **seule feuille**, appliquée à l'**application**.

Cela restait sans conséquence tant que rien n'y changeait en cours d'exécution. Le `LOT-68` a changé
cela : les grandeurs de la portée identité sont multipliées par un facteur qui suit la hauteur de la
fenêtre. Reposer la feuille applicative repolit **tous** ses widgets — 862 sur cette application —
et la re-polish leur recalcule métriques, tailles et dispositions. Mesure en configuration Debug :
**cinq secondes par appel**.

Un glisser de bordure qui longe un seuil de facteur le franchit des dizaines de fois. Le
regroupement introduit précédemment (un minuteur de 200 ms) rendait le coût supportable sans le
supprimer, et déplaçait le problème : la nouvelle taille atterrissait **après** que la fenêtre eut
été placée et peinte, d'où un recalage visible. Deux appels échappaient de surcroît au regroupement
(changement de thème clair/sombre, changement de facteur).

Le coût était disproportionné à sa cause : le facteur ne concerne que les quelques dizaines de
widgets des écrans du jeu.

## Travail réalisé
- **`theme.qss` scindé en deux fichiers** :
  - `Source/Elements/Themes/theme-identity.qss` — la portée identité, appliquée sur la **pile
    d'écrans** ;
  - `Source/Elements/Themes/theme-editor.qss` — le châssis d'édition, appliqué sur
    l'**application**.
  Chacun porte en tête l'énoncé de sa portée et la raison de la séparation. `resources.qrc` expose
  les deux sous des alias stables.
- **`hmi::identityStyleSheet()`** (nouveau) : rend la feuille d'identité substituée plutôt que de
  l'appliquer — c'est l'appelant qui sait **où** la poser. `hmi::applyStyleSheet` ne charge plus que
  la feuille du châssis. Le chargement et la substitution, communs aux deux, sont factorisés dans un
  `loadStyleSheet` d'unité de compilation.
- **`MainWindow::applyIdentityStyleSheet`** : pose la feuille sur `_stack`. Les panneaux dockables,
  les barres d'outils, la barre de menus et la barre d'état n'appartiennent pas à cette portée et ne
  sont donc plus repolis.
- **Regroupement supprimé** (`_themeReapplyTimer`, `THEME_REAPPLY_DEBOUNCE_MS`) : il n'y a plus rien
  à différer. Le facteur s'applique **dans** le redimensionnement, là où l'utilisateur l'attend —
  le gel disparaît par construction, non par temporisation.

## Vérification
- `ApplicationThemeTest.LesDeuxPorteesSontDansDeuxFichiersDisjoints` (nouveau) : la feuille
  d'identité ne référence aucun jeton `editor.color.`, celle du châssis aucun jeton `identity.`.
  C'est ce test qui empêche la fusion de revenir par inadvertance.
- `ApplicationThemeTest.EtancheiteDesPortees` et `…AvecLesVraisThemes` : réécrits pour comparer la
  feuille d'identité **entière** sous les deux thèmes, au lieu de repérer une frontière de section
  dans un texte concaténé — un repérage qu'un simple déplacement de commentaire cassait.
- `ThemeTeardownGuardsTest` : ses garde-fous lisent désormais les **deux** feuilles concaténées,
  pour que la séparation ne rétrécisse pas ce qu'ils couvrent.
