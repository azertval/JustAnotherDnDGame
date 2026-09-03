# TACHE-07 — Encodage d'observation de l'IA : 33 → 36 canaux {#lot-74-tache-07-encodage-ia}

**Lot :** [LOT-74](epic.md) · **Emplacement :** `Source/AiSolver/Env` · **Statut :** fait

## Contexte
L'IA de résolution observe le monde par une fenêtre de tuiles encodée en canaux **catégoriels**, un
par `TileType` : `aisolver::TileWindowEncoder::encode` produit un tenseur
`(channelCount(), windowSize(), windowSize())`, avec `CHANNEL_COUNT = 33` aujourd'hui.

Ajouter trois types de tuile fait donc passer l'observation à **36 canaux**. Ce n'est pas un détail
d'implémentation : la **forme** du tenseur d'entrée change, donc la première couche de tout réseau
entraîné avant ce lot n'a plus les bonnes dimensions. Les modèles déjà entraînés ou publiés ne se
rechargent plus.

Deux conséquences se distinguent, et il faut les traiter séparément :

- **La signification des canaux existants**, elle, est préservée — c'est exactement pourquoi le
  cadrage impose d'ajouter les trois valeurs **après** `MovingPlatform` plutôt que de les insérer.
  Insérer au milieu aurait décalé les indices, et un réseau réentraîné n'aurait rien pu réutiliser
  d'une intuition antérieure sur, par exemple, le canal des dangers.
- **La forme du tenseur** change quoi qu'il arrive. C'est irréductible dès lors qu'un type de tuile
  est ajouté ; il ne s'agit donc pas de l'éviter, mais de la **détecter proprement**.

## Travail à réaliser
- `AiSolver/Env/TileWindowEncoder.h` : `CHANNEL_COUNT` dérive de la sentinelle unique de TACHE-02, et
  le paragraphe « **Non dérivé automatiquement** … à mettre à jour manuellement » est remplacé par la
  règle nouvelle. La valeur passe mécaniquement à 36.
- **Chargement d'un modèle à l'ancienne forme : déjà couvert, relecture de code.**
  `aisolver::nn::loadWeights` (`AiSolver/Nn/Serialization.h`) valide déjà « magique, version, nombre
  de couches et **forme de chaque couche** avant toute modification », et renvoie `false` — une
  erreur récupérable, jamais une exception — en laissant le réseau intact. Un modèle entraîné à
  33 canaux est donc refusé sans qu'une ligne soit écrite. Le contrôle de *nombre de couches* était
  déjà testé ; le cas réel de ce lot ne l'est pas, la structure gardant le même nombre de couches et
  ne changeant que la **largeur** de la première : un test dédié a été ajouté pour lui.
- Consigner dans le `CHANGELOG.md` (TACHE-10) que les modèles antérieurs doivent être réentraînés.

## Fichiers impactés
- `Source/AiSolver/Env/TileWindowEncoder.h`.
- `Source/Test/Unit/AiSolver/Nn/test_serialization.cpp` : le cas « même structure, entrée élargie ».
- Aucun code de chargement modifié — voir « déjà couvert » ci-dessus.

## Tests (obligatoires)
- `Test/Unit/AiSolver/Env/test_tile_window_encoder.cpp` : `channelCount()` vaut le nombre réel de
  types ; les trois nouvelles tuiles produisent chacune leur canal, distinct de tous les autres.
- **Indices préservés** : un test qui verrouille l'indice de canal de quelques types **existants**
  (`Solid`, `Danger`, `MovingPlatform`) — c'est lui qui garantit que la décision « ajouter en fin »
  tient dans le temps, et qui se déclenchera si quelqu'un insère un type au milieu plus tard.
- **Refus d'un modèle à l'ancienne forme** : `loadWeights` renvoie `false` et le réseau cible garde
  ses poids (`RejetDUnModeleALAncienneLargeurDObservation`).
- Le rejeu IA existant (`RejeuIaSysteme`) reste vert.
- **Effet de bord mesuré, à consigner** : `ActorCriticTrainerTest.ConvergenceDuCritiqueAPolitiqueFigee`
  a échoué après le passage à 36 canaux — l'entrée du critique s'élargit à nombre de neurones cachés
  constant, et 80 épisodes ne suffisaient plus à rendre la décroissance mesurable (moyenne des dix
  derniers : `3585,8` contre `3572,6` pour les dix premiers). Le run est porté à 200 épisodes : la
  propriété testée est inchangée, seule sa fenêtre de mesure s'adapte à une entrée plus large.

## Points d'attention
- **Ne pas transformer ce refus en conversion automatique.** Compléter un ancien modèle par trois
  canaux nuls serait tentant et faux : le réseau n'a jamais vu ces tuiles, et une conversion
  silencieuse produirait un agent qui *paraît* fonctionner. Le refus explicite est le comportement
  correct.
- Le réentraînement des modèles livrés est **hors périmètre** du lot (voir `### Exclus` de l'épic) ;
  cette tâche ne fait que rendre la rupture visible et propre.
- Vérifier que le mode IA de l'IHM affiche l'erreur au lieu de l'avaler : un modèle refusé
  silencieusement au chargement se lirait comme un bug d'entraînement.

## Définition de fait (DoD)
- `channelCount()` vaut 36 et dérive de la sentinelle ; un modèle à l'ancienne forme est refusé avec
  un message explicite ; indices des canaux existants verrouillés par un test ; `ctest` vert.

## Exigences
`EX-NFR-040`, `EX-ARCH-011`, `EX-GP-027`, `EX-GP-028`, `EX-GP-029`.
