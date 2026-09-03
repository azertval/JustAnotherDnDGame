# LOT-21 — Attaques, dégâts et états {#lot-21}

> Statut : **à faire**.
> Prérequis : [LOT-13](@ref lot-13), [LOT-14](@ref lot-14), [LOT-20](@ref lot-20).

## Objectif

Résoudre une attaque au d20 contre la classe d'armure, appliquer les dégâts, gérer critiques,
inconscience et mort.

## Périmètre

- `Source/Core/Combat/Attack.{h,cpp}` : jet d'attaque = d20 + modificateur de caractéristique +
  bonus de maîtrise, contre la CA de la cible.
- `Damage.{h,cpp}` : types de dégâts, résistances, vulnérabilités, immunités.
- `Condition.{h,cpp}` : à terre, entravé, empoisonné, inconscient — et leur effet sur les jets.
- Jets de sauvegarde contre la mort.

## Les règles à ne pas se tromper

- **Un 20 naturel double les dés de dégâts, pas le modificateur.** L'erreur inverse est la plus
  répandue et fausse tout l'équilibrage.
- **Un 1 naturel rate toujours**, quel que soit le total.
- Les PV sont **bornés à 0** par le bas : pas de PV négatifs qui rendraient la réanimation
  arbitraire.

## Le journal de combat

Chaque jet doit être **auditable** : jet brut, modificateurs détaillés, cible, total, résultat.
« Tu as raté » n'est pas une information ; « 7 + 3 = 10 contre CA 15 : raté » en est une. C'est ce
qui permet au joueur de comprendre le système, et au développeur de déboguer l'équilibrage sans
attacher un débogueur.

## Exigences couvertes

`EX-DND-*` (résolution chiffrée) et `EX-CBT-*` (application en combat).

## Critères d'acceptation

- Un 20 naturel double **les dés** et non le modificateur, vérifié par test.
- Un 1 naturel rate, même avec un total supérieur à la CA.
- PV bornés à 0 ; inconscience et jets de sauvegarde contre la mort testés aux bornes.
- Résistances, vulnérabilités et immunités appliquées dans le bon ordre.
- Chaque jet produit une entrée de journal complète et lisible.
