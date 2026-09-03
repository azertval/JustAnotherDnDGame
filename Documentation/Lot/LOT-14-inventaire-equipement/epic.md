# LOT-14 — Inventaire et équipement {#lot-14}

> Statut : **à faire**.
> Prérequis : [LOT-13](@ref lot-13).

## Objectif

Porter, équiper et consommer des objets, avec un effet **mesurable** sur la fiche.

## Périmètre

- `Source/Core/Rpg/Item.{h,cpp}` : type, poids, valeur, effets. Catalogue en **JSON**.
- `Inventory.{h,cpp}` : contenu, capacité de charge.
- `Equipment.{h,cpp}` : emplacements (arme principale, arme secondaire, armure, accessoires).
- Statistiques **dérivées** : armure → CA, arme → dés de dégâts et caractéristique d'attaque.

## Le piège

**Retirer un équipement doit annuler exactement son effet.** L'erreur classique est d'appliquer un
bonus en additionnant à la volée (`ca += 2`) : après trois équipements et deux retraits dans le
désordre, la CA a dérivé. La CA doit être **recalculée** depuis l'équipement porté, jamais
accumulée.

C'est un critère d'acceptation, pas une préférence de style.

## Exigences couvertes

`EX-INV-*`, `EX-DND-*` (statistiques dérivées).

## Critères d'acceptation

- Équiper une armure change la CA du montant attendu ; la retirer **restitue exactement** la valeur
  d'origine, quel que soit l'ordre des opérations.
- Un test équipe et retire dans plusieurs ordres et vérifie l'absence de dérive.
- Capacité de charge respectée, avec un comportement défini au dépassement.
- Catalogue d'objets entièrement en données.
