# LOT-16 — Niveaux de grande taille {#lot-16}

> Statut : **terminé**. Ce lot rend accessible ce que `Core` permet déjà sans restriction
> (aucune taille maximale dans `TileMap`/`LevelDraft`) mais que l'ergonomie de l'éditeur limitait de
> fait : choisir directement une grande taille de grille pendant l'édition, et voir un niveau
> plus grand que la fenêtre correctement dans l'éditeur **et** en jeu.

## Objectif
- Permettre de **saisir directement** une largeur et une hauteur cibles (`largeur x hauteur`)
  plutôt que d'incrémenter case par case aux flèches (conservées pour l'ajustement fin), sous un
  **plafond généreux** (`EX-EDIT-017`).
- Corriger la caméra pour qu'elle **englobe tout le niveau**, dans l'éditeur comme en jeu, y
  compris pour un niveau plus grand que la fenêtre — aujourd'hui, dans les deux écrans, le zoom ne
  descend jamais sous 1×, donc une partie de la grille reste hors champ dès qu'elle dépasse la
  fenêtre (`EX-REN-013` corrigée).

## Périmètre

### Inclus
- **Plafond de taille et analyse du format `largeur x hauteur`** : nouvelle validation dédiée
  (`hmi::LevelSizeValidation`, sur le modèle de `LevelNameValidation` de LOT-15), plafond partagé
  par la boîte de dialogue et par le redimensionnement aux flèches (aucun chemin ne le contourne).
- **Boîte de dialogue de redimensionnement** (`Ctrl+R`) : réutilise le mécanisme de saisie de texte
  de LOT-15 (`TextInputField`), généralisé pour porter aussi bien le nom que la taille — un seul
  champ actif à la fois, pas de duplication de la plomberie de saisie.
- **Correction du calcul de zoom automatique** : `EditorScreen::renderGrid` (cadrage auto **et**
  borne minimale du zoom manuel) et `GameScreen::render` partagent la même faute d'origine — un
  plancher `max(1.0, …)` qui empêche de dézoomer sous facteur 1, donc de faire tenir un niveau plus
  grand que la fenêtre. Corrigé aux trois emplacements par la même règle : zoom entier tant qu'il
  est `≥ 1` (netteté pixel art inchangée pour les niveaux actuels), zoom fractionnaire seulement
  quand nécessaire pour englober un niveau plus grand.
- **Correction de spécification** : `EX-REN-013` (« la caméra doit suivre le personnage ») ne
  correspondait déjà plus à l'implémentation (caméra fixe cadrant le tableau depuis LOT-08) —
  reformulée pour décrire la stratégie retenue (cadrer tout le niveau, pas suivre).

### Exclus (hors périmètre de ce lot)
- **Caméra suiveuse du joueur** (avec zone morte/marge, façon *platformer* classique) — écartée
  explicitement au profit du cadrage « niveau entier », plus simple et suffisant pour l'objectif
  (aucune zone invisible), sans logique de suivi/bornage à concevoir.
- **Contenu livré** : aucun nouveau niveau de démonstration n'est ajouté à la séquence de jeu ; ce
  lot livre la **capacité**, pas du contenu l'exploitant.
- **Décors, pipeline photo → pixel art** — inchangé, toujours hors périmètre (`EX-DEC-*`).

## Décisions de cadrage
- **Le plafond de taille vit dans `HMI`, pas dans `Core`.** `TileMap`/`LevelDraft` restent
  **sans limite** (cohérent avec l'existant, `EX-NFR-010` : logique pure, aucune règle produit
  arbitraire mêlée à la donnée). Le plafond est un garde-fou d'**usage** (éviter une grille
  démesurée par erreur de saisie ou flèche maintenue trop longtemps), porté par la seule couche qui
  a une notion d'« utilisateur » — l'éditeur.
- **Un seul plafond, appliqué à toutes les voies de redimensionnement.** Flèches et boîte de
  dialogue appellent toutes deux `EditorScreen::requestResize`, qui borne la cible avant d'appliquer
  — pas de second point de contrôle à maintenir en cohérence.
- **La boîte de dialogue réutilise `TextInputField` tel quel** (LOT-15) : seul un nouveau
  validateur est nécessaire (`hmi::isValidLevelSize`), sur le même principe que
  `isValidLevelName`. Le slot de saisie de texte d'`EditorScreen` (`_nameInput` → généralisé) sert
  aux trois usages (création, renommage, redimensionnement) : un seul champ peut être actif à la
  fois, cohérent avec l'existant.
- **Format de saisie : `largeur x hauteur`** (ex. `40x30`), séparateur `x`/`X` tolérant les espaces
  autour — un seul champ de texte, pas un formulaire à deux cases, pour rester sur le mécanisme de
  saisie déjà livré sans en construire un second.
- **Caméra « niveau entier » plutôt que suiveuse** : plus simple (aucune zone morte, aucun bornage
  aux limites à calculer, aucun risque de saccade si le joueur va vite), et suffit à l'objectif
  formulé (« aucune zone invisible »). Le compromis assumé : les tuiles/le personnage rapetissent
  visuellement à mesure que le niveau grandit — acceptable, le plafond de taille (`EX-EDIT-017`)
  garde ce rapetissement dans des proportions raisonnables.
- **Le zoom fractionnaire est une exception locale, pas un changement de politique.** `EX-ARCH-022`
  dit déjà « zoom **de préférence** en facteurs entiers » — la préférence reste la règle par défaut
  (petits niveaux, zoom ≥ 1, inchangé) ; le zoom fractionnaire n'intervient que lorsque c'est
  strictement nécessaire pour respecter l'objectif « aucune zone invisible ».

## Exigences couvertes
- Nouvelle : `EX-EDIT-017`.
- Corrigée (formulation, pas le sens de l'exigence globale) : `EX-REN-013`.
- Réutilisées (approfondies sans changer leur sens) : `EX-EDIT-005` (redimensionnement),
  `EX-EDIT-013` (caméra manuelle de l'éditeur, dont le cadrage automatique est corrigé),
  `EX-ARCH-022` (zoom pixel art, préférence — pas absolu), `EX-NFR-010` (logique pure côté `Core`).

## Découpage

> État : ✅ fait · 🔄 en cours · ⬜ non commencé.

| Tâche | Intitulé | Emplacement | État |
|-------|----------|-------------|:----:|
| [TACHE-01](tache-01-plafond-validation-taille.md) | Plafond de taille et validation « largeur x hauteur » | `HMI/Editor`, `HMI/Input` | ✅ |
| [TACHE-02](tache-02-boite-dialogue-redimensionnement.md) | Boîte de dialogue de redimensionnement (`Ctrl+R`) | `HMI/Interface` | ✅ |
| [TACHE-03](tache-03-camera-niveau-entier.md) | Caméra : englober tout le niveau (éditeur et jeu) | `HMI/Interface` | ✅ |
| [TACHE-04](tache-04-documentation-verification.md) | Documentation et vérification | `Documentation` | ✅ |

## Critères d'acceptation du lot
1. Depuis l'éditeur, `Ctrl+R` ouvre une saisie pré-remplie de la taille courante ; taper une
   nouvelle taille valide (ex. `60x40`) et valider redimensionne la grille en un geste — avec la
   même confirmation destructrice qu'aux flèches si la nouvelle taille perdrait l'entrée, la sortie
   ou une liaison (`EX-EDIT-012`, inchangé).
2. Une saisie invalide (format incorrect, valeur nulle/négative, ou au-delà du plafond) est
   **refusée** avec un message compréhensible, sans modifier la grille.
3. Ni les flèches ni la boîte de dialogue ne permettent de dépasser le plafond de taille.
4. Un niveau plus grand que la fenêtre reste **entièrement visible** (zoom automatique adapté),
   aussi bien dans l'éditeur qu'en essai immédiat/en jeu — sans zone hors champ.
5. Un niveau qui tient déjà dans la fenêtre (toutes les tailles livrées à ce jour) s'affiche
   **à l'identique** d'avant ce lot (zoom entier inchangé) — non-régression.
6. Logique nouvelle **couverte par des tests** (`ctest` vert), déterministe, sans GPU. Build
   `/W4 /WX` sans avertissement, Doxygen et `CHANGELOG.md` à jour, lint des exigences vert.

## Dépendances
- Réutilise le brouillon mutable et `requestResize` (LOT-14/15, `LevelDraft`, `EditorScreen`), le
  mécanisme de saisie de texte et les garde-fous (LOT-15, `TextInputField`, `PendingConfirmation`),
  la caméra (`Camera2D`, LOT-05) et le rendu de niveau en jeu (`GameScreen`, LOT-07/09).

## Navigation des tâches
- @subpage lot-16-tache-01-plafond-validation-taille
- @subpage lot-16-tache-02-boite-dialogue-redimensionnement
- @subpage lot-16-tache-03-camera-niveau-entier
- @subpage lot-16-tache-04-documentation-verification
