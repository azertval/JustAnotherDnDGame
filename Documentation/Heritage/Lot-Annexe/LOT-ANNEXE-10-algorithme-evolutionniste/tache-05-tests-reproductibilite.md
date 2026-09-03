# TACHE-05 — Tests de reproductibilité et non-régression {#lot-annexe-10-tache-05-tests-reproductibilite}

**Lot :** [LOT-ANNEXE-10](epic.md) · **Emplacement :** `Source/Test/Unit/AiSolver/Training` ·
**Statut :** fait

## Contexte
Chaque tâche précédente teste sa brique isolément. Cette tâche ajoute les tests d'**intégration**
de bout en bout explicitement exigés par le cadrage du programme Lot-Annexe : la reproductibilité
stricte à seed fixée (condition indispensable pour pouvoir déboguer un entraînement en le rejouant
à l'identique) et la non-régression d'une croissance effective du fitness sur un niveau trivial.

## Travail à réaliser
- **Test de reproductibilité** : deux instances complètes d'`EvolutionaryTrainer`, mêmes
  `EvolutionaryConfig`, même seed `Rng`, même niveau, exécutées sur un nombre fixe de générations —
  comparer les poids finaux du meilleur individu (égalité bit-à-bit des `Tensor` de poids) et
  l'historique de fitness par génération. Doivent être strictement identiques.
- **Test de sensibilité à la seed** : les deux mêmes instances mais avec des seeds différentes
  produisent (sur plusieurs répétitions si nécessaire pour écarter une coïncidence) des résultats
  différents — évite un test de reproductibilité qui passerait trivialement si l'algorithme
  ignorait la seed par erreur.
- **Test de non-régression sur niveau trivial** : sur `demo-deplacement.json`
  (`Source/Elements/Levels/`, mécanique unique — déplacement horizontal, sans obstacle ni danger,
  réutilisé tel quel plutôt qu'un niveau de test dédié pour rester proche d'un contenu réel du
  jeu), faire tourner l'algorithme sur un nombre de générations fixé et vérifier que le meilleur
  fitness final dépasse un seuil minimal documenté, et que le meilleur fitness ne régresse jamais
  d'une génération à l'autre sur toute la fenêtre testée.
- Réduction de la configuration (taille de population, nombre de générations) pour tenir dans un
  budget de temps CI raisonnable (quelques secondes) — valeurs choisies expérimentalement et
  commentées dans le test (pas de valeur magique non expliquée).

## Fichiers impactés
- `Source/Test/Unit/AiSolver/Training/test_evolutionary_reproducibility.cpp` (nouveau).
- `Source/Test/Unit/AiSolver/Training/test_evolutionary_non_regression.cpp` (nouveau).
- Cible `UnitTests` (`CMakeLists.txt` de `Source/Test`) : ajout des deux nouveaux fichiers.

## Tests (obligatoires)
- **Reproductibilité stricte à seed fixée** : voir « Travail à réaliser » — comparaison bit-à-bit.
- **Sensibilité effective à la seed** : deux seeds différentes produisent des résultats différents.
- **Croissance du fitness sur niveau trivial** : seuil minimal franchi sur `demo-deplacement.json`
  dans la fenêtre de générations testée.
- **Non-régression de l'élite sur la fenêtre complète** : aucune diminution du meilleur fitness
  d'une génération à l'autre, tout au long du test.
- **Budget CI respecté** : le test complet s'exécute en un temps borné documenté.

## Points d'attention
- **Égalité bit-à-bit, pas de tolérance** : toute la chaîne (`Rng`, `Tensor`, propagation avant,
  environnement) est déterministe par construction (LOT-ANNEXE-01/03/05) — une différence, même
  infime, signale une vraie source de non-déterminisme à corriger (ex. itération sur une structure
  non ordonnée), pas un bruit numérique à masquer par une tolérance.
- **« Niveau trivial » ne veut pas dire « trivial pour l'algorithme »** : même sans obstacle, il
  faut apprendre à produire l'action « avancer » depuis une politique initialement aléatoire — le
  seuil de fitness et le nombre de générations doivent être calibrés empiriquement et documentés,
  pas fixés arbitrairement.
- **Premier filet de détection pour toute la chaîne amont** : ces tests s'appuient sur
  LOT-ANNEXE-01/03/05/06/07/08/09 en plus de LOT-ANNEXE-10 lui-même — une régression sur l'un
  d'eux qui échapperait à ses propres tests unitaires a de bonnes chances d'être détectée ici.

## Définition de fait (DoD)
- Les tests listés sont verts en CI, avec un temps d'exécution documenté et raisonnable ; build
  `/W4 /WX` sans avertissement ; Doxygen à jour.

## Notions abordées
@ref guide-annexe-algorithmes-evolutionnistes — population, fitness, sélection, croisement,
mutation.

## Exigences
Aucune exigence propre — valide `EX-IA-011` (déclarée en TACHE-04).
