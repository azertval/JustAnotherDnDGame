# Gameplay {#spec-gameplay}

> Statut : **socle**. Cette page fixe ce que toute carte et tout écran du jeu supposent : un monde
> en tuiles que le héros ne traverse pas, et des états de jeu explicites. Le gameplay du RPG
> lui-même est décrit ailleurs — l'exploration dans [`exploration.md`](exploration.md), les règles
> dans [`regles-d20.md`](regles-d20.md), le personnage dans [`rpg.md`](rpg.md), le combat dans
> [`combat.md`](combat.md). Dépend de [`vision.md`](vision.md).

## 1. Monde en tuiles

Une carte est une **grille de tuiles** ; chaque case porte un type (`core::TileType`) : le vide, la
matière pleine, l'entrée, et le vocabulaire de terrain du RPG (`EX-EXP-005`). Le format qui la
décrit est dans [`niveaux.md`](niveaux.md).

- \anchor EX-GP-001 **EX-GP-001** — Une carte doit être représentée par une grille de tuiles
  typées.
- \anchor EX-GP-002 **EX-GP-002** — Une tuile solide (`core::isSolid`) doit empêcher le héros de la
  traverser.
- \anchor EX-GP-014 **EX-GP-014** — Les collisions du héros avec les tuiles solides doivent être
  résolues sur les deux axes, sans traversée à vitesse élevée ni blocage contre un coin
  (`EX-EXP-002`, `EX-EXP-003`).

Chaque règle est déterministe : à état d'entrée identique, comportement identique (`EX-NFR-002`).

## 2. États de jeu

- \anchor EX-GP-040 **EX-GP-040** — Le jeu doit gérer des états distincts — menu, exploration,
  pause, options, crédits et écrans du RPG — portés par `hmi::ScreenFlow`, avec des transitions
  explicites et unidirectionnelles (`EX-GP-041`). Détaillé côté interface par `EX-IHM-090`.
- \anchor EX-GP-041 **EX-GP-041** — Les transitions entre états doivent être explicites et
  unidirectionnelles à chaque événement (machine à états).

## Exigences retirées {#gp-retirees}

> Ancres conservées, jamais renumérotées : les lots livrés s'y réfèrent. Chacune décrivait une
> mécanique que le jeu n'a pas et que le moteur ne porte plus.

- \anchor EX-GP-003 **EX-GP-003** *(retirée au `LOT-88`)* — pentes et suivi de surface.
- \anchor EX-GP-004 **EX-GP-004** *(retirée au `LOT-88`)* — arrondis et suivi de surface.
- \anchor EX-GP-005 **EX-GP-005** *(retirée au `LOT-88`)* — blocs poussables de taille réduite.
- \anchor EX-GP-006 **EX-GP-006** *(retirée au `LOT-88`)* — pentes et arrondis de plafond.
- \anchor EX-GP-007 **EX-GP-007** *(retirée au `LOT-88`)* — arrondis concaves.
- \anchor EX-GP-010 **EX-GP-010** *(retirée au `LOT-88`)* — marche horizontale ; le déplacement est
  `EX-EXP-001`.
- \anchor EX-GP-011 **EX-GP-011** *(retirée au `LOT-88`)* — saut.
- \anchor EX-GP-012 **EX-GP-012** *(retirée au `LOT-88`)* — gravité.
- \anchor EX-GP-013 **EX-GP-013** *(retirée au `LOT-88`)* — saut au sol seulement.
- \anchor EX-GP-015 **EX-GP-015** *(retirée au `LOT-88`)* — sauts aériens.
- \anchor EX-GP-016 **EX-GP-016** *(retirée au `LOT-88`)* — glissade et saut mural.
- \anchor EX-GP-017 **EX-GP-017** *(retirée au `LOT-88`)* — ruée (dash).
- \anchor EX-GP-018 **EX-GP-018** *(retirée au `LOT-88`)* — ressenti vertical du saut.
- \anchor EX-GP-019 **EX-GP-019** *(retirée au `LOT-88`)* — masse et vitesse terminale de chute.
- \anchor EX-GP-020 **EX-GP-020** *(retirée au `LOT-88`)* — interrupteur ; ce qui agit sur une carte
  est une entité (`EX-LVL-017`).
- \anchor EX-GP-021 **EX-GP-021** *(retirée au `LOT-88`)* — porte liée à un interrupteur.
- \anchor EX-GP-022 **EX-GP-022** *(retirée au `LOT-88`)* — bloc poussable.
- \anchor EX-GP-023 **EX-GP-023** *(retirée au `LOT-88`)* — clé et porte verrouillée.
- \anchor EX-GP-024 **EX-GP-024** *(retirée au `LOT-88`)* — budget de sauts et de ruées.
- \anchor EX-GP-025 **EX-GP-025** *(retirée au `LOT-88`)* — plaque de pression.
- \anchor EX-GP-026 **EX-GP-026** *(retirée au `LOT-88`)* — plateforme mobile.
- \anchor EX-GP-027 **EX-GP-027** *(retirée au `LOT-88`)* — bloc descendant.
- \anchor EX-GP-028 **EX-GP-028** *(retirée au `LOT-88`)* — bloc fragile.
- \anchor EX-GP-029 **EX-GP-029** *(retirée au `LOT-88`)* — bloc éphémère.
- \anchor EX-GP-030 **EX-GP-030** *(retirée en `LOT-67`)* — sortie qui termine un niveau : il n'y a
  plus de niveau à terminer.
- \anchor EX-GP-031 **EX-GP-031** *(retirée en `LOT-67`)* — échec au contact d'un danger ; ce que
  devient la mort d'un personnage est le sujet du `LOT-72`.
- \anchor EX-GP-032 **EX-GP-032** *(retirée en `LOT-67`)* — redémarrage d'un niveau après échec ;
  remplacé par la sauvegarde du `LOT-17`.
- \anchor EX-GP-050 **EX-GP-050** *(retirée au `LOT-88`)* — danger directionnel.
- \anchor EX-GP-051 **EX-GP-051** *(retirée au `LOT-88`)* — danger mobile.
- \anchor EX-GP-052 **EX-GP-052** *(retirée au `LOT-88`)* — danger commuté.
- \anchor EX-GP-053 **EX-GP-053** *(retirée au `LOT-88`)* — danger temporisé.
- \anchor EX-GP-054 **EX-GP-054** *(retirée au `LOT-88`)* — route d'une plateforme mobile.
- \anchor EX-GP-055 **EX-GP-055** *(retirée au `LOT-88`)* — capacités de saut et de ruée par niveau.
- \anchor EX-GP-056 **EX-GP-056** *(retirée au `LOT-88`)* — ruée chargée.
- \anchor EX-GP-057 **EX-GP-057** *(retirée au `LOT-88`)* — poussée renforcée pendant une ruée.
- \anchor EX-GP-058 **EX-GP-058** *(retirée au `LOT-88`)* — chute plongeante.
- \anchor EX-GP-060 **EX-GP-060** *(retirée au `LOT-88`)* — ruée suivant les pentes.
- \anchor EX-GP-061 **EX-GP-061** *(retirée au `LOT-88`)* — enchaînement ruée et saut.

## Traçabilité

Exploration : [`exploration.md`](exploration.md). Format des cartes : [`niveaux.md`](niveaux.md).
Contrôles : [`controles.md`](controles.md).
