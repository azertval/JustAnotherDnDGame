# TACHE-02 — Sentinelle unique de fin d'énumération {#lot-74-tache-02-borne-enumeration}

**Lot :** [LOT-74](epic.md) · **Emplacement :** `Source/Core/Levels/TileType.h` et ses trois
consommateurs · **Statut :** fait

## Contexte
La décision de cadrage du lot ajoute les trois valeurs **après** `MovingPlatform` plutôt que de les
insérer au milieu de l'énumération, pour ne pas décaler les canaux d'observation de l'IA (voir
TACHE-07). Le prix de ce choix est connu et **écrit dans le code** : trois fichiers recopient à la
main la borne « dernier énumérateur de `TileType` ».

`Source/Test/Systeme/test_couverture_mecaniques.cpp` le dit sans détour :

> Meme borne (`TileType::MovingPlatform`, le dernier enumerateur) et meme technique que
> `core::parseTileType` […] : un type ajoute APRES MovingPlatform exigerait de bouger cette borne
> aux DEUX endroits.

Et `Source/AiSolver/Env/TileWindowEncoder.h` porte la même dette sous une autre forme, une constante
littérale :

> Nombre de canaux catégoriels = nombre de valeurs de `core::TileType`. **Non dérivé
> automatiquement** (`Core` n'expose pas de compte de catégories) : à mettre à jour manuellement si
> `TileType` gagne une valeur — risque documenté, pas mitigé par ce lot.

Trois copies d'un même fait, dont deux se signalent elles-mêmes comme fragiles. Ce lot est le
premier à devoir réellement les déplacer. Les déplacer **une troisième fois à la main** serait payer
la dette sans la rembourser : cette tâche la supprime.

## Travail à réaliser
- `Core/Levels/TileType.h` : exposer **une** source de vérité pour la fin de l'énumération —
  `inline constexpr int TILE_TYPE_COUNT`, adossée à un dernier énumérateur, avec un commentaire
  Doxygen qui dit explicitement qu'ajouter une valeur ne demande **rien** d'autre. Retenir la forme
  qui n'introduit pas de valeur parasite dans les `switch` exhaustifs du projet : une constante
  libre est préférable à un énumérateur `Count`, qui obligerait chaque `switch` sans `default` à
  traiter un cas qui n'existe pas.
- `Core/Levels/TileTypeName.cpp` : `parseTileType` itère jusqu'à la sentinelle, plus jusqu'à
  `MovingPlatform`.
- `Test/Systeme/test_couverture_mecaniques.cpp` : `allContentTileTypes` idem, et le commentaire qui
  décrivait la dette est remplacé par la règle nouvelle (« un type ajouté n'importe où est pris en
  compte sans modification ici »).
- `AiSolver/Env/TileWindowEncoder.h` : `CHANNEL_COUNT` dérive de la sentinelle au lieu du littéral
  `33`, et le paragraphe « Non dérivé automatiquement » disparaît. La conséquence fonctionnelle du
  changement de valeur est traitée en TACHE-07 ; ici, seule la **dérivation** change.

## Fichiers impactés
- `Source/Core/Levels/TileType.h` (la sentinelle), `Source/Core/Levels/TileTypeName.cpp`.
- `Source/Test/Systeme/test_couverture_mecaniques.cpp`.
- `Source/AiSolver/Env/TileWindowEncoder.h`.
- `Source/Test/Unit/HMI/Editor/test_tile_taxonomy.cpp` (quatrième copie, voir Points d'attention).
- `Source/Test/Unit/AiSolver/Env/test_observation_bords_carte.cpp` : le nombre de canaux y était
  écrit `33` en dur, à trois endroits ; il dérive maintenant de `TileWindowEncoder::CHANNEL_COUNT`.

## Tests (obligatoires)
- `Test/Unit/Core/Levels/test_tile_type_name.cpp` : la table de `parseTileType` contient **tous** les
  types, les trois nouveaux compris — ce test échouerait aujourd'hui si la borne n'était pas
  déplacée, c'est lui qui prouve la correction.
- Un test qui verrouille la sentinelle contre l'énumération réelle, de sorte qu'un futur ajout sans
  mise à jour soit impossible plutôt que silencieux.
- `Test/Unit/AiSolver/Env/test_tile_window_encoder.cpp` : `channelCount()` vaut le nombre réel de
  types.

## Points d'attention
- Cette tâche est un **prérequis strict** de TACHE-01 côté résultat : tant qu'elle n'est pas faite,
  les trois nouveaux types existent dans l'énumération mais restent invisibles de `parseTileType` —
  un niveau qui les contient ne se chargerait pas. Les deux tâches peuvent s'écrire dans l'ordre
  qu'on veut, mais elles se livrent ensemble.
- Ne pas en profiter pour réordonner l'énumération : les niveaux sont sérialisés **par nom**, donc
  un réordonnancement ne les casserait pas, mais il décalerait les canaux d'observation de l'IA —
  exactement ce que la décision de cadrage cherche à éviter.
- Il y avait bien une **quatrième** copie, que la recherche textuelle initiale n'avait pas vue :
  `Test/Unit/HMI/Editor/test_tile_taxonomy.cpp` recalculait sa propre constante
  `TILE_TYPE_COUNT = static_cast<std::size_t>(TileType::MovingPlatform) + 1`. Seule la compilation
  puis l'exécution l'ont revélée (la taxonomie annonçait 36 types, la borne locale en attendait 33).
  Elle dérive désormais elle aussi de `core::TILE_TYPE_COUNT` — la dette est soldée aux **quatre**
  endroits.

## Définition de fait (DoD)
- La borne de fin d'énumération n'apparaît plus qu'à **un seul endroit** du dépôt ; les trois
  anciens sites la consomment ; build `/W4 /WX` ; `ctest` vert.

## Exigences
`EX-ARCH-011`, `EX-GP-027`, `EX-GP-028`, `EX-GP-029`.
