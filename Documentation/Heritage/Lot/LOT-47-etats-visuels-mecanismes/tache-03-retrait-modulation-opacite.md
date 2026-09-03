# TACHE-03 — Retrait des modulations d'opacité {#lot-47-tache-03-retrait-modulation-opacite}

**Lot :** [LOT-47](epic.md) · **Emplacement :** `Source/HMI/Game` · **Statut :** fait

## Contexte
`hmi::GameSession` code aujourd'hui l'état des mécanismes dans le **canal alpha** de la teinte :

- `refreshDoorVisuals()` : porte ouverte → `tint.a = 0.25f`, fermée → `1.0f` ;
- `refreshDangerStateVisuals()` : `INACTIVE_ALPHA = 0.35f`, `ACTIVE_ALPHA = 1.0f` ;
- `refreshBlockVisuals()` : repositionne les blocs et applique leur échelle.

C'était un placeholder honnête tant que tout était en couleurs plates. Appliqué à de vraies
textures, il produirait des portes semi-transparentes — un résultat pire que l'état actuel, parce
que l'incohérence deviendrait visible.

Ce retrait est la contrepartie du lot : sans lui, deux systèmes coderaient le même état.

## Travail à réaliser
- **Remplacer** les modulations d'alpha par le choix de clip (TACHE-01 et TACHE-02) dans
  `refreshDoorVisuals` et `refreshDangerStateVisuals`.
- **Rendre `tint` à son rôle** : un ajustement colorimétrique, jamais un canal d'information d'état.
  Supprimer les constantes `INACTIVE_ALPHA`/`ACTIVE_ALPHA`.
- **`refreshBlockVisuals`** : conserver le repositionnement et l'échelle (ce sont des données de
  simulation, pas un artifice visuel), retirer ce qui relève de l'état visuel.
- **Mode Physique** : décider explicitement du comportement. Le mode Physique n'a pas d'assets
  animés ; il doit conserver un moyen de distinguer une porte ouverte d'une porte fermée. Conserver
  la modulation d'alpha **uniquement** dans cette branche est acceptable et cohérent — c'est un
  rendu de diagnostic, pas le rendu du jeu. À documenter comme tel.

## Fichiers impactés
- `Source/HMI/Game/GameSession.{h,cpp}` : `refreshMechanismDiagnosticTint`, appelée au **rendu**
  (dépend du mode courant, décision purement visuelle), remplace `refreshDoorVisuals`/
  `refreshDangerStateVisuals`.
- `Source/HMI/Graphics/MechanismVisuals.{h,cpp}` : `mechanismDiagnosticAlpha`, la décision
  elle-même extraite en fonction **pure** (mode + état + paire d'alpha → alpha), pour rester
  testable sans GPU — `GameSession` ne fait plus que l'appeler et écrire le résultat sur `tint`.
- `Source/Test/Unit/HMI/Graphics/test_mechanism_visuals.cpp` (tests de `mechanismDiagnosticAlpha`).

## Tests (obligatoires)
- En mode **Texture**, `mechanismDiagnosticAlpha` renvoie toujours `1.0`, quel que soit l'état :
  c'est la garantie qu'aucun quad de mécanisme n'a une teinte alpha différente de 1 dans ce mode
  (`GameSession` ne fait qu'écrire cette valeur sur `core::Sprite::tint`, sans autre logique).
- En mode **Physique**, `mechanismDiagnosticAlpha` renvoie la paire d'alpha fournie par
  l'appelant — porte ouverte distinguable, danger actif distinguable.
- Les tests de gameplay existants (mécanismes, dangers, blocs) passent **sans modification** : ce lot
  ne touche pas la simulation.

## Points d'attention
- **Ne pas retirer la distinction en mode Physique par excès de zèle** : ce mode sert précisément à
  lire l'état réel du niveau, une porte ouverte doit y rester identifiable.
- Vérifier qu'aucun autre endroit du code n'utilise l'alpha comme information (recherche sur
  `tint.a`) : le but est qu'il n'en reste **aucun** en mode Texture.
- Le repositionnement des blocs poussables n'est pas un effet visuel : il reflète leur position
  simulée et doit rester.

## Définition de fait (DoD)
- Plus aucune modulation d'opacité ne code un état en mode Texture ; le mode Physique conserve sa
  lisibilité selon la règle documentée ; les tests de gameplay passent inchangés ; `/W4 /WX` propre.

## Exigences
`EX-REN-006` (apparence pilotée par l'état logique) ; réutilise `EX-REN-046` (bascule),
`EX-ARCH-012` (rendu sans effet sur la simulation), `EX-NFR-004` (vérification sans GPU).
