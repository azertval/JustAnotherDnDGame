# LOT-15 — PNJ et dialogues {#lot-15}

> Statut : **à faire**.
> Prérequis : [LOT-05](@ref lot-05) (mode « monde gelé »), [LOT-10](@ref lot-10) (entités et
> interaction), [LOT-12](@ref lot-12) (jets de compétence en dialogue).

## Objectif

Parler à un PNJ via un arbre de dialogue scripté, avec choix, conditions et jets de compétence.

## Périmètre

- `Source/Core/Rpg/Dialogue.{h,cpp}` : graphe de nœuds en **JSON** — réplique, choix, condition sur
  drapeau, action (donner un objet, poser un drapeau, démarrer une quête), **jet de compétence**
  (ex. Persuasion contre difficulté 15, avec deux suites possibles).
- `DialogueRunner` : machine à états **pure**, testable headless. Elle ne connaît ni Qt, ni le
  rendu, ni le monde — elle consomme un graphe et une source de drapeaux, elle produit un nœud
  courant et des choix.
- `DialogueMode` (`LOT-05`) : le monde est **gelé** pendant la conversation.
- Widget Qt sur `DesignTokens` / `PixelFrameWidget` / `BitmapFont`.
- Tout le texte via `hmi::Localization` — **aucun libellé en dur** (`EX-REN-033`).

## Ce qui fait la qualité de ce lot

La séparation runner pur / widget. Un dialogue doit être **jouable en test headless**, nœud par
nœud, sans fenêtre : c'est ce qui permettra de vérifier des arbres de vingt nœuds sans les cliquer
à la main. Le widget ne fait qu'afficher ce que le runner décide.

## Exigences couvertes

`EX-RPG-*` (dialogue, choix, conditions, actions), `EX-IHM-*`.

## Critères d'acceptation

- Un dialogue de dix nœuds, deux conditions et un jet de Persuasion se parcourt **en headless**.
- Un graphe mal formé (nœud cible inconnu, cycle non intentionnel, choix vide) est **rejeté au
  chargement** avec un message exploitable, pas découvert en jeu.
- L'exploration est **gelée** pendant le dialogue : aucun pas de simulation d'exploration consommé.
- Traduction fr/en complète, aucun texte en dur.
