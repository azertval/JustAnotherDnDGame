# TACHE-02 — Parcours polyligne deterministe {#lot-67-tache-02-parcours-polyligne}

**Lot :** [LOT-67](epic.md) · **Emplacement :** `Source/Core/Gameplay` · **Statut :** fait

## Contexte
`PlatformController::boxAtStep` calculait une onde triangulaire sur un **segment unique**. Le
passage a une route a N points impose de generaliser ce calcul sans rien perdre de sa propriete
essentielle : la position est fonction du seul numero de pas, jamais d'une accumulation.

Le code convertissait par ailleurs `stepCount + phase` en `float` — ce qui perd le bit de poids
faible au-dela d'environ 16,7 millions de pas (~77 h de jeu) et decale visiblement la plateforme en
fin de longue session. Le refactor est l'occasion de corriger ce defaut preexistant.

## Travail a realiser
- Creer `core::PlatformPath` (`Core/Gameplay/PlatformPath.h`) : sommets du parcours, longueurs
  cumulees, longueur totale et longueur de cycle. En circuit ferme, le point de depart est repete en
  fin de liste — c'est ce qui materialise le segment de fermeture.
- Cette primitive est **partagee** avec l'overlay d'edition : le trajet dessine est celui parcouru,
  jamais une reimplementation parallele.
- Precalculer les routes au constructeur du controleur : `boxAtStep` est appelee plusieurs fois par
  pas et par consommateur (portage du personnage, des blocs, interpolation d'affichage).
- Mener le calcul de distance en **double precision** ; seule la position finale repasse en `float`.
- Localiser le segment courant par recherche dans les longueurs cumulees, de sorte qu'un segment de
  longueur nulle (point duplique) soit saute sans division par zero.
- Cycle : le double de la route en aller-retour, son perimetre en circuit ferme.

## Fichiers impactes
`Source/Core/Gameplay/PlatformPath.{h,cpp}` (nouveaux), `PlatformController.{h,cpp}`,
`Source/Core/CMakeLists.txt`.

## Tests (obligatoires)
- `test_platform_path.cpp` (nouveau) : ordre des sommets ; repetition du depart en circuit ferme ;
  route vide jamais fermee ; distinction des deux cycles ; distance negative repliee.
- `test_platform_controller.cpp` : route a trois points parcourue puis refaite a l'envers ; circuit
  ferme ne rebroussant jamais chemin ; route degeneree immobile ; point duplique traverse sans
  incident ; **aucune derive apres vingt millions de pas**.
- Les tests existants de l'aller-retour a deux points restent inchanges : c'est la preuve de
  non-regression la plus directe.

## Points d'attention
`_configs` est une **copie** du vecteur du niveau, et `_paths` en derive : les deux doivent rester
alignes. Ne pas ajouter d'API de reconfiguration a chaud sans reconstruire les routes.

Le contrat `PlatformSample` ne change pas : `CharacterPhysicsSystem`, `BlockController` et
`GameSession` ne voient rien de ce refactor.

## Definition de fait (DoD)
Une plateforme a un point se comporte au chiffre pres comme avant. Les modes aller-retour et
circuit ferme sont couverts par des tests aux instants remarquables. `ctest` a 100 %.

## Exigences
`EX-GP-054`, `EX-GP-026`, `EX-NFR-002`, `EX-NFR-040`.
