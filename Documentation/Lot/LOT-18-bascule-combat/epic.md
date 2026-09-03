# LOT-18 — Bascule exploration ↔ combat {#lot-18}

> Statut : **à faire**.
> Prérequis : [LOT-05](@ref lot-05) (modes de jeu), [LOT-10](@ref lot-10) (déclencheurs),
> [LOT-13](@ref lot-13) (fiches des combattants).

## Objectif

Déclencher une rencontre depuis l'exploration, geler le monde, monter la grille de combat, et en
revenir — sans que le joueur perde quoi que ce soit au passage.

## La décision qui structure ce lot

Le combat se déroule **sur la carte d'exploration**, jamais dans une arène chargée à part
(décision de cadrage, cf. `Documentation/Specification/vision.md`). La grille tactique est
**dérivée de la couche collision** du format v3 (`LOT-04`).

C'est ce qui donne la continuité visuelle d'un Chrono Trigger plutôt que la rupture d'un JRPG
classique — et c'est ce qui justifie rétroactivement le soin mis au format de carte en phase B.
En contrepartie, *toute carte doit être un terrain tactique valide* : contrainte de level design
signalée dans l'éditeur dès le `LOT-11`.

## Périmètre

- `CombatMode` : le second mode de `LOT-05`.
- `Source/Core/Combat/Encounter.{h,cpp}` : définition **JSON** — ennemis, positions de départ
  (relatives au déclencheur), conditions.
- Déclencheurs : contact avec un ennemi de carte, zone de rencontre, action de dialogue.
- **Sauvegarde et restauration de l'état d'exploration** : position, orientation, caméra, entités.

## Le critère qui compte

Entrer puis sortir d'un combat doit restituer **exactement** l'état d'exploration — aux PV près,
qui eux ont changé. Un ennemi vaincu est retiré de la carte **durablement** : c'est un drapeau de
monde (`LOT-16`), persisté (`LOT-17`), pas un booléen local perdu au rechargement de la carte.

## Exigences couvertes

Catégorie `EX-CBT-*`, déclarée par ce lot dans `Documentation/Specification/combat.md`.

## Critères d'acceptation

- Aller-retour exploration → combat → exploration restituant l'état, PV mis à jour.
- Un ennemi vaincu ne réapparaît pas, y compris après avoir quitté et rechargé la carte.
- Une fuite ramène à l'exploration sans que l'ennemi soit marqué vaincu.
- Testable headless : le montage et le démontage d'une rencontre ne demandent ni fenêtre ni GPU.
