# LOT-03 — Agrégat `LevelData` {#lot-03}

> Statut : **à faire**.
> Prérequis : [LOT-02](@ref lot-02) (confort d'outillage ; pas une dépendance stricte).

## Objectif

Remplacer le constructeur positionnel de `core::Level` par un agrégat nommé, **avant** que le RPG
n'y ajoute ses propres champs.

## Le problème

Le constructeur de `core::Level` comptait **19 paramètres** ; le `LOT-01` l'a ramené à 11 en
retirant le gameplay de plateforme, mais la dette de fond demeure — elle est actée dans l'en-tête
lui-même (`Source/Core/Levels/Level.h`). Le RPG va rajouter au moins six champs : couches de
tuiles (`LOT-04`), entités placées, connexions de carte, zones de rencontre, points d'apparition,
drapeaux. On repasserait au-dessus de 17 paramètres positionnels, où deux `std::optional<std::string>`
voisins s'intervertissent sans que le compilateur bronche.

Solder maintenant, alors que la liste est au plus court, coûte le minimum.

## Périmètre

- `struct LevelData` : agrégat public portant les mêmes champs, dans `Source/Core/Levels/Level.h`.
- `Level` construit depuis un `LevelData&&`. Les *designated initializers* C++20 rendent chaque
  site de construction lisible (`.name = …, .tileMap = …`).
- Constructeur historique conservé `[[deprecated]]` **le temps de ce lot seulement**, pour ne pas
  toucher des centaines de sites d'un coup, puis retiré avant la fin du lot.
- Sites à migrer : `LevelLoader.cpp`, `LevelWriter.cpp`, `LevelDraft.cpp`, et les tests de
  `Source/Test/Unit/Core/Levels/`.

## Exigences couvertes

`EX-LVL-*` nouvelle (« un niveau se construit depuis un agrégat nommé »), `EX-ARCH-*`.

## Critères d'acceptation

- Plus aucun appel au constructeur positionnel ; le constructeur `[[deprecated]]` a disparu.
- Refactoring à **comportement constant** : les tests passent, adaptés mécaniquement (la forme de
  l'appel change, jamais l'intention du test).
- Doxygen vert (chaque champ de `LevelData` documenté, plus de `@param` orphelin).
