# Contrôles & entrées {#spec-controles}

> Statut : **livré** (`0.1.0`). Clavier, souris et manette (XInput) jouables/navigables, remappage
> inclus. Dépend de [`gameplay.md`](gameplay.md).

## 1. Périphériques
- \anchor EX-CTRL-001 **EX-CTRL-001** — Le jeu doit être jouable **entièrement au clavier**.
- \anchor EX-CTRL-002 **EX-CTRL-002** — Le jeu doit supporter une **manette** (XInput).

## 2. Actions du jeu (mapping logique)
Les entrées sont traduites en **actions logiques** (pas de code métier lié à une touche physique), pour rendre le remappage et la manette possibles.

| Action logique | Clavier (défaut) | Manette (défaut) |
|----------------|------------------|------------------|
| Aller à gauche | ← (remappable, `LOT-H-29`) | Stick gauche / D-pad gauche (remappable, `LOT-H-30`) |
| Aller à droite | → (remappable, `LOT-H-29`) | Stick gauche / D-pad droite (remappable, `LOT-H-30`) |
| Sauter | Espace (remappable, `LOT-H-29`) | A (remappable, `LOT-H-30`) |
| Dash (8 directions) | Maj (remappable, `LOT-H-29`) | Épaule droite (remappable, `LOT-H-30`) |
| Interagir (`EX-CTRL-022`) | E (remappable) | X (remappable) |
| Quitter vers le menu (jeu) | Échap | B / Start |
| Valider (menu) | Entrée | A |
| Retour (menu) | Échap | B |

- \anchor EX-CTRL-010 **EX-CTRL-010** — Chaque action de gameplay doit être définie comme une action logique, dissociée de la touche physique.
- \anchor EX-CTRL-011 **EX-CTRL-011** — L'état d'une action doit distinguer **pressée**, **maintenue** et **relâchée** dans une frame (nécessaire au *jump buffering*).
- \anchor EX-CTRL-012 **EX-CTRL-012** — Le mapping doit être **reconfigurable** (au minimum via un fichier de configuration). Implémenté pour un sous-ensemble d'actions de jeu et d'éditeur, clavier (`LOT-H-29`) et manette (`LOT-H-30`) : Options → Touches de jeu / Touches de l'éditeur / Touches de la manette, persistance dans `Settings/keybindings.json`. Les deux sources (clavier, manette) sont remappables indépendamment l'une de l'autre — remapper l'une n'affecte jamais l'autre.
- \anchor EX-CTRL-013 **EX-CTRL-013** — Le **dash** doit être une action logique dédiée (touche par défaut : **Maj**), sa **direction** étant donnée par les touches directionnelles (8 directions), à défaut par l'**orientation** courante du personnage.
- \anchor EX-CTRL-022 **EX-CTRL-022** — **Interagir** doit être une action logique dédiée (défauts :
  **E** au clavier, **X** à la manette), remappable indépendamment sur chaque source
  (`EX-CTRL-012`) et distinguant ses fronts (`EX-CTRL-011`). Elle **complète** l'activation par
  contact des mécanismes, qu'elle ne remplace pas (`EX-GP-020` autorise les deux) : les niveaux
  livrés restent franchissables à l'identique. Un fichier de remappage antérieur reste lisible,
  l'action y prenant sa valeur par défaut. Premier usage : le ramassage de la **clé**
  (`EX-GP-023`, `LOT-H-63`).

## 3. Réactivité
- \anchor EX-CTRL-020 **EX-CTRL-020** — La latence entrée → action ne doit pas dépasser **une frame** de simulation. Depuis `LOT-H-33`, garanti **à tout framerate de rendu** : les fronts (pressée/relâchée) sont consommés par **pas de simulation**, non par frame de rendu — un appui capturé sur une frame réelle sans pas (rendu > 60 Hz) n'est plus perdu.
- \anchor EX-CTRL-021 **EX-CTRL-021** — La lecture des entrées doit être échantillonnée une fois par frame, en amont de la mise à jour de la logique. L'état brut reste échantillonné **une fois par frame** (`Window::pumpMessages`) ; l'avancée de la ligne de base des fronts (`Window::beginInputFrame`) est en revanche cadencée sur le **pas de simulation** (`LOT-H-33`), pour ne pas effacer un front avant qu'un pas ne l'ait lu.

## Traçabilité
Le module d'entrées relève de `Source/HMI` (acquisition) mais expose un état d'actions consommé par `Source/Core` (logique), sans dépendance inverse. Voir [`exigences-non-fonctionnelles.md`](exigences-non-fonctionnelles.md) pour l'architecture.
