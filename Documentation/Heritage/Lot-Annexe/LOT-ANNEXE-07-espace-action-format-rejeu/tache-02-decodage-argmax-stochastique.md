# TACHE-02 — Décodage déterministe et stochastique {#lot-annexe-07-tache-02-decodage-argmax-stochastique}

**Lot :** [LOT-ANNEXE-07](epic.md) · **Emplacement :** `Source/AiSolver/Env` · **Statut :** à faire

## Contexte
`ActionSpace` (TACHE-01) définit *quelles* actions existent ; cette tâche définit *comment en
choisir une* à partir d'une distribution de probabilité produite par un réseau (sortie `softmax`,
`LOT-ANNEXE-03`). Deux besoins distincts et complémentaires : un choix reproductible pour le rejeu
final (`LOT-ANNEXE-11`), un choix explorant la distribution pendant l'entraînement par gradient
(génération 3).

## Travail à réaliser
- **`aisolver::decodeArgmax`** (`Source/AiSolver/Env/ActionDecoding.h/.cpp`) :
  `Action decodeArgmax(const Tensor<float>& distribution)` — retourne `actionAt(indice du maximum)`.
  Règle de départage explicite en cas d'égalité stricte entre plusieurs indices : le **premier**
  indice au sens de l'ordre d'énumération de `ActionSpace` (déterministe, ne dépend d'aucun état
  externe).
- **`aisolver::decodeStochastic`** : `Action decodeStochastic(const Tensor<float>& distribution,
  float temperature, Rng& rng)` — applique la température à la distribution (`p_i^(1/temperature) /
  somme`, `temperature = 1.0` laisse la distribution inchangée), puis échantillonne un indice selon
  la distribution résultante en tirant `rng.nextFloat(0, 1)` et en parcourant la somme cumulée
  (méthode de la roulette).
- Documentation explicite du contrat : `distribution` est supposée déjà normalisée (somme à `1`,
  produite par une couche `softmax`, `LOT-ANNEXE-03`) — ni `decodeArgmax` ni `decodeStochastic` ne
  renormalisent en entrée.

## Fichiers impactés
- `Source/AiSolver/Env/ActionDecoding.h/.cpp` — nouveau.
- `Source/AiSolver/CMakeLists.txt` — ajout des nouveaux fichiers.

## Tests (obligatoires)
- **`decodeArgmax` sur un maximum unique** : retourne l'action attendue.
- **`decodeArgmax` sur une égalité stricte** : retourne systématiquement l'action du premier indice
  en cas d'égalité (testé avec une distribution construite à la main pour ce cas).
- **`decodeStochastic`, sur un grand nombre de tirages (ex. 100 000) à graine fixée puis variable** :
  la fréquence empirique de chaque action converge vers la probabilité attendue de la distribution
  d'entrée, à une tolérance statistique documentée (test de type khi-deux ou simple écart relatif
  borné, pas une preuve statistique rigoureuse mais un garde-fou de non-régression).
- **Effet de la température** : une température basse (proche de `0`) rend `decodeStochastic`
  quasi-équivalent à `decodeArgmax` sur une majorité de tirages ; une température haute aplatit la
  distribution vers un tirage quasi uniforme — les deux comportements vérifiés explicitement.
- **Reproductibilité** : deux appels à `decodeStochastic` avec la **même** instance de `Rng` dans le
  même état produisent la même action ; avec deux instances de graines différentes, potentiellement
  des actions différentes (non déterminisme inter-graines attendu, déterminisme intra-graine
  vérifié).

## Points d'attention
- **`decodeStochastic` ne doit jamais recevoir une instance de `Rng` initialisée sur l'horloge** :
  cohérent avec la décision transverse de `LOT-ANNEXE-01` — c'est l'appelant (boucle d'entraînement,
  génération 3) qui contrôle la graine, jamais cette fonction.
- **La méthode de la roulette (somme cumulée) est en `O(taille de l'espace d'action)`** : avec 24
  actions (TACHE-01), le coût est négligeable ; pas d'optimisation par recherche binaire nécessaire
  à cette échelle.

## Définition de fait (DoD)
- `decodeArgmax`/`decodeStochastic` disponibles et testés (`ctest` vert) ; build `/W4 /WX` sans
  avertissement ; Doxygen à jour.

## Notions abordées
@ref guide-annexe-apprentissage-renforcement — action, espace d'action, politique déterministe ou
stochastique, exploration.

## Exigences
`EX-IA-007` (nouvelle, partagée avec TACHE-01/03/04 du même lot).
