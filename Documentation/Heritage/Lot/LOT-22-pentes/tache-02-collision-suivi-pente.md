# TACHE-02 — Collision et suivi de pente {#lot-22-tache-02-collision-suivi-pente}

**Lot :** [LOT-22](epic.md) · **Emplacement :** `Core/Physics`, `Core/Ecs/Systems` · **Statut :** fait

## Contexte
La tâche la plus risquée du lot : ajouter une passe de résolution après le balayage `sweepAabb`
existant, sans régresser la physique plate déjà testée (sauts, dash, wall jump…). Consomme le
modèle posé en TACHE-01.

## Travail à réaliser
- **`Core/Physics/SweptCollision.cpp`/`.h`** : `SweepResult` gagne un indicateur optionnel (ou une
  fonction séparée est ajoutée) pour exposer, après le balayage classique, si la boîte chevauche
  une colonne de pente sous elle — nécessaire pour que `CharacterPhysicsSystem` sache s'il doit
  appliquer le suivi. Décision d'implémentation à trancher en écrivant le code : soit `sweepAabb`
  reste inchangée et une nouvelle fonction libre `resolveSlope(box, tiles)` est appelée séparément
  par `CharacterPhysicsSystem`, soit la logique est intégrée à `sweepAabb` elle-même. **Préférer la
  fonction séparée** : elle isole le risque (une régression dans le suivi de pente ne peut pas
  casser `sweepAabb`, testée et utilisée par tout le reste du moteur).
- **`Core/Ecs/Systems/CharacterPhysicsSystem.cpp`** : après l'étape 5 (résolution `sweepAabb`,
  inchangée), nouvelle étape : si la case sous le centre horizontal du personnage (ou ses deux
  pieds, à trancher en implémentant) est une pente, et que le personnage est à une hauteur proche
  de la surface (tolérance dédiée) **et ne vient pas de sauter** (vitesse verticale non
  franchement négative), caler `transform.position.y` sur la hauteur de la pente et forcer
  `player.grounded = true`.

## Fichiers impactés
- `Source/Core/Physics/SweptCollision.h`/`.cpp` (ou nouveau fichier dédié au suivi de pente,
  ex. `Core/Physics/SlopeFollow.h`/`.cpp`, si séparer le code réduit le risque de régression).
- `Source/Core/Ecs/Systems/CharacterPhysicsSystem.cpp`.
- Tests : nouveau `Source/Test/Unit/Core/Physics/test_slope_follow.cpp` (ou équivalent) + nouveaux
  cas dans `Source/Test/Integration/test_physique_personnage.cpp`.

## Tests (obligatoires)
- Marcher horizontalement sur une pente ascendante fait monter le personnage progressivement
  (position Y qui suit la formule de hauteur), sans jamais passer au travers ni léviter au-dessus.
- Symétrique en descente.
- Tomber depuis au-dessus d'une pente pose le personnage sur sa surface au premier contact, sans
  la traverser (pas de tunneling à vitesse de chute élevée).
- Sauter depuis une pente produit une impulsion identique à un saut depuis un sol plat (le suivi de
  pente ne l'annule pas immédiatement).
- Transition pente → sol plat (et l'inverse) sans à-coup ni blocage.
- **Suite de régression complète** : tous les tests existants de `test_physique_personnage.cpp` et
  `test_swept_collision.cpp` restent verts, **sans modification** de leur code.

## Points d'attention
- **Isoler le code de suivi de pente de `sweepAabb`** (voir décision de cadrage de l'épic et
  discussion ci-dessus) : la fonction historique reste la référence testée pour les murs/sols
  plats ; le suivi de pente est une couche additive, jamais une modification en place.
- **Écart constaté par rapport au plan initial** : `sweepAabb` n'est en fait **pas** restée
  totalement inchangée. Les tests d'intégration ont révélé qu'une pente suivie d'un bloc plein de
  même hauteur (le raccord le plus courant) bloquait le personnage à mi-montée : `sweepX` traite
  comme un mur toute case pleine partageant une ligne que la boîte chevauche déjà, or suivre une
  pente fait légitimement chevaucher la boîte à l'intérieur de sa case (contrairement au sol plat,
  où elle n'en effleure jamais que la frontière). Corrigé par une exclusion minimale et ciblée
  (`rowIsSlopeGround`) plutôt qu'une réécriture : généralise le principe déjà en place de `kSkin`
  sans toucher `sweepY` ni la logique de blocage des murs/sols pleins. Voir @ref guide-physique,
  section « Suivi de pente », pour le détail du raisonnement et le test qui l'a mis au jour
  (`SuitUnePenteAscendanteEnMarchant`).
- **Tolérance de calage** : trop large, le personnage « collerait » à une pente même en sautant
  franchement au-dessus ; trop stricte, il risque de passer légèrement sous la surface à grande
  vitesse de chute avant que le calage ne s'applique. Choisir une valeur testée empiriquement
  (partir de `kSkin`/tolérances déjà en place dans `SweptCollision.cpp`, ajuster si les tests de
  chute rapide échouent).
- **Ne pas oublier le cas « pas de pente sous le personnage »** : le comportement doit rester
  strictement identique à aujourd'hui (aucun appel au suivi de pente, `grounded` décidé uniquement
  par `sweepAabb` comme actuellement) — c'est ce qui garantit la non-régression du critère
  d'acceptation 4 de l'épic.

## Définition de fait (DoD)
- Suivi de pente fonctionnel et testé ; **zéro régression** sur la suite de tests physique
  existante ; build `/W4 /WX` sans avertissement.

## Exigences
`EX-GP-003` (implémentation complète).
