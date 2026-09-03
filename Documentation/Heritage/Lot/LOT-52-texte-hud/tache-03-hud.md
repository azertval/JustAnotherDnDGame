# TACHE-03 — Affichage tête haute : budgets et tableau courant {#lot-52-tache-03-hud}

**Lot :** [LOT-52](epic.md) · **Emplacement :** `Source/HMI/Game`, `Source/Elements/Localization` · **Statut :** fait

## Contexte
`LOT-12` a introduit les **budgets de sauts et de dashs** par tableau (`EX-GP-024`) : une contrainte
de puzzle qui **refuse** l'action quand le budget est épuisé. Le joueur subit donc un refus sans
jamais avoir su combien il lui restait de sauts.

L'information existe dans `core::Player` (`jumpsRemaining`, `dashesRemaining`) depuis ce lot ; il n'a
jamais été possible de l'afficher, faute de rendu de texte.

## Travail à réaliser
- **Affichage des budgets** : sauts et dashs restants, affichés **uniquement** quand le niveau
  définit un budget (une valeur négative signifie « illimité » — ne rien afficher dans ce cas, plutôt
  qu'un symbole d'infini que le joueur devrait interpréter).
- **Nom du tableau** en cours, discret.
- **Affiché en jeu et en mode essai**, jamais en édition pure.
- **Traduction** (`EX-REN-033`) : libellés dans `fr.lang` et `en.lang`, aucune chaîne en dur — y
  compris les libellés courts.
- **Position et lisibilité** : coin de l'écran, avec un contraste suffisant pour rester lisible sur
  un fond clair comme sur un fond sombre (le fond de niveau est libre). Prévoir un contour ou une
  ombre portée de texte.
- **Aucun effet sur la simulation** (`EX-ARCH-012`) : l'affichage lit `core::Player`, il ne le
  modifie pas.

## Fichiers impactés
- `Source/HMI/Game/GameHud.{h,cpp}` (nouveau) ou extension de `GameSession`.
- `Source/HMI/Game/GameSession.{h,cpp}`.
- `Source/Elements/Localization/fr.lang`, `en.lang`.
- `Source/Test/Unit/HMI/Game/test_game_hud.cpp` (nouveau).

## Tests (obligatoires)
- **Contenu affiché** : niveau avec budgets → les deux compteurs ; niveau sans budget → aucun
  compteur ; budget partiellement défini (sauts limités, dashs illimités) → un seul compteur.
  Fonction **pure** produisant la liste des lignes à afficher, testée sans GPU.
- Décroissance : après un saut, le compteur affiché correspond à `jumpsRemaining`.
- Chaque clé de traduction utilisée existe dans les deux catalogues.

## Points d'attention
- **Ne pas afficher un compteur quand le budget est illimité.** Le cas par défaut de la grande
  majorité des niveaux est « illimité » ; un HUD permanent affichant l'infini serait du bruit visuel
  sur tous les tableaux non-puzzle.
- Vérifier le comportement au **rechargement** après échec : les budgets sont réinitialisés
  (`EX-GP-024`), le HUD doit suivre immédiatement.
- Le HUD ne doit pas masquer une zone de jeu utile : vérifier sur le plus petit cadrage de salle.
- Ne pas ajouter d'information « parce qu'on peut » (position, vitesse, compteur d'images) : le
  périmètre est délibérément minimal, et un affichage de diagnostic est un autre sujet.

## Définition de fait (DoD)
- Les budgets restants et le nom du tableau sont visibles en jeu et en essai, uniquement quand ils
  ont du sens, lisibles sur tout fond, entièrement traduits, sans effet sur la simulation ; le choix
  du contenu est testé sans GPU ; `/W4 /WX` propre.

## Exigences
`EX-IHM-003` (affichage tête haute) ; réutilise `EX-REN-032` (affichage de texte), `EX-GP-024`
(budgets de mouvements), `EX-REN-033` (traduction), `EX-ARCH-012` (rendu sans effet sur la
simulation).
