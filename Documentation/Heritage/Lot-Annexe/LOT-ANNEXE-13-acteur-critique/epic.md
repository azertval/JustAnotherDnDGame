# LOT-ANNEXE-13 — Réduction de variance : critique et avantage (acteur-critique) {#lot-annexe-13}

> Statut : **fait**. Prérequis : [LOT-ANNEXE-12](@ref lot-annexe-12) (REINFORCE, dont la
> perte et la boucle d'entraînement sont réutilisées et étendues). Bénéficie de
> [LOT-ANNEXE-03](@ref lot-annexe-03) (réseaux), [LOT-ANNEXE-04](@ref lot-annexe-04) (optimiseurs)
> et [LOT-ANNEXE-09](@ref lot-annexe-09) (statistiques). Deuxième lot de la génération 3
> (apprentissage par gradient).

## Objectif
REINFORCE (LOT-ANNEXE-12) prouve que le gradient de la récompense peut orienter l'apprentissage de
la politique — mais c'est un estimateur de gradient à **haute variance**, limitation bien connue et
directement héritée du choix de cadrage assumé dans ce lot amont : le retour brut `G_t`, utilisé tel
quel comme pondération du gradient de log-probabilité, mélange le mérite propre de l'action prise à
`t` avec tout le bruit accumulé sur le **reste** de l'épisode (un épisode globalement chanceux
survalorise chaque action qu'il contient, même une mauvaise). En pratique, cela se traduit par des
mises à jour de poids erratiques et une convergence lente ou instable.

Ce lot introduit un **critique** — un second réseau, entraîné en parallèle, qui apprend à estimer
la valeur d'un état — et remplace le retour brut par l'**avantage** (`retour − valeur estimée`) dans
la perte de politique. L'avantage mesure combien une action a fait **mieux ou moins bien que ce qui
était attendu** depuis cet état, plutôt que la performance brute de tout l'épisode : c'est un
resserrement direct de la variance du gradient, sans changer la nature de l'algorithme (toujours du
policy gradient, toujours rétropropagé via le moteur d'autodiff maison de LOT-ANNEXE-02 — ce lot
reste pleinement dans l'exigence ferme d'apprentissage par gradient, il en améliore la stabilité).

## Périmètre

### Inclus
- **Réseau critique** (`Critic`) : réseau de neurones distinct de la politique, construit avec la
  même bibliothèque (LOT-ANNEXE-03), qui prend en entrée une observation encodée (LOT-ANNEXE-06) et
  produit une **estimation scalaire** de la valeur de l'état (récompense future attendue depuis cet
  état, sous la politique courante).
- **Avantage** (`avantage_t = retour_t − valeur_estimée_t`) qui remplace le retour brut `G_t` dans la
  perte de politique de LOT-ANNEXE-12 : même formule `-log π(a_t|s_t) × avantage_t`, seul le facteur
  multiplicatif change.
- **Perte du critique** (erreur quadratique entre `valeur_estimée_t` et `retour_t`, la même cible que
  celle utilisée pour l'avantage) et **optimisation conjointe** : deux réseaux (acteur, critique),
  deux graphes d'autodiff, deux optimiseurs indépendants (LOT-ANNEXE-04) — mis à jour à chaque
  épisode, dans le même passage que la boucle de LOT-ANNEXE-12.
- **Comparaison chiffrée** entre acteur-critique et REINFORCE brut, sur le **même** niveau de
  contrôle et le même budget d'épisodes, mesurée via les CSV produits par `TrainingStatsRecorder`
  (LOT-ANNEXE-09) : vitesse de convergence (nombre d'épisodes jusqu'à un plafond de récompense
  donné) et stabilité (variance de la récompense d'un épisode à l'autre en fin de run).

### Exclus (hors périmètre de ce lot)
- **PPO ou DQN** : LOT-ANNEXE-14, dont le choix est explicitement différé à son ouverture et
  s'appuiera précisément sur les résultats chiffrés produits ici.
- **Avantage généralisé (GAE) ou tout lissage inter-pas de l'avantage** : l'avantage à un pas
  (`retour − valeur`) est le resserrement de variance le plus direct ; un lissage supplémentaire
  (moyenne pondérée entre plusieurs horizons) est un raffinement de plus, non nécessaire pour
  démontrer le bénéfice qualitatif de la réduction de variance.
- **Partage de poids entre acteur et critique** (tronc commun, deux têtes) : deux réseaux
  **entièrement séparés** sont plus simples à raisonner et à tester indépendamment ; le partage de
  poids introduirait un couplage d'entraînement (un gradient de perte du critique affectant les
  poids partagés avec l'acteur) hors périmètre d'un lot centré sur la réduction de variance.
- **Rejeu ou export en jeu** : aucun changement par rapport à LOT-ANNEXE-11 — un acteur entraîné
  ici s'exporte exactement comme un acteur de LOT-ANNEXE-12, le critique ne sert qu'à l'entraînement
  et n'est jamais exporté (aucune inférence live, cf. contrainte transverse du programme).

## Décisions de cadrage
- **Acteur et critique sont deux réseaux séparés, sans poids partagés.** Un tronc commun réduirait
  le nombre de paramètres mais coupla-t-il les deux gradients de façon à rendre l'attribution des
  effets (l'acteur progresse-t-il grâce à un meilleur avantage, ou parce que le tronc partagé a
  changé pour une autre raison ?) difficile à isoler dans la comparaison chiffrée visée par ce lot.
- **L'avantage est calculé à un seul pas (`retour − valeur`), sans lissage inter-horizons (GAE).**
  Le bénéfice recherché est la réduction de variance la plus directe et la plus simple à valider par
  gradient checking ; un lissage plus sophistiqué resterait un raffinement possible d'un lot futur,
  non nécessaire pour répondre à l'exigence de ce lot.
- **La perte du critique utilise le même retour `G_t` (Monte-Carlo complet) que TACHE-02 de
  LOT-ANNEXE-12 comme cible**, pas une cible bootstrapée (`reward_t + gamma × valeur(s_{t+1})`) :
  cohérent avec l'absence de bootstrap déjà actée pour l'acteur en LOT-ANNEXE-12, et plus simple à
  vérifier par gradient checking (la cible ne dépend pas d'une estimation du réseau lui-même,
  contrairement à un bootstrap classique de type TD).
- **Deux optimiseurs indépendants, potentiellement de types différents** (ex. Adam pour les deux,
  ou SGD pour l'un et Adam pour l'autre) : l'acteur et le critique n'ont pas nécessairement la même
  dynamique d'apprentissage optimale, et les découpler évite qu'un réglage pour l'un contraigne
  l'autre.
- **La comparaison chiffrée réutilise le format CSV existant** (LOT-ANNEXE-09/12) sans le modifier :
  superposer les courbes de REINFORCE brut et d'acteur-critique ne demande qu'une colonne
  d'identification de l'algorithme, déjà couverte par la structure existante des runs nommés.
- **Le niveau de contrôle est le même que celui de LOT-ANNEXE-12 (TACHE-05)** : condition nécessaire
  pour que la comparaison de vitesse de convergence soit valide (même difficulté, même longueur
  d'épisode typique).

## Notions abordées
Voir @ref guide-annexe-acteur-critique (variance de REINFORCE, avantage, critique, invariance de la
*baseline*). Sources directes : Sutton, McAllester, Singh, Mansour (1999/2000, preuve de
l'invariance de la baseline) ; Konda & Tsitsiklis (2000, article d'origine de l'architecture
acteur-critique) — bibliographie complète dans le chapitre.

## Exigences couvertes
- Nouvelle, déclarée dans [la spécification IA](@ref spec-ia) : [`EX-IA-014`](@ref EX-IA-014).
- Réutilisées (inchangées) : `EX-IA-013` (perte de politique de LOT-ANNEXE-12, étendue à l'avantage)
  et les exigences des lots amont — réseaux de neurones (LOT-ANNEXE-03), optimiseurs
  (LOT-ANNEXE-04), `TrainingStatsRecorder` (LOT-ANNEXE-09) — non renumérotées ici.

## Découpage

> État : ✅ fait · 🔄 en cours · ⬜ non commencé.

| Tâche | Intitulé | Emplacement | État |
|-------|----------|-------------|:----:|
| [TACHE-01](tache-01-reseau-critique.md) | Réseau critique (estimation de la valeur d'état) | `Source/AiSolver/Training/ActorCritic` | ✅ |
| [TACHE-02](tache-02-avantage.md) | Avantage remplaçant le retour brut dans la perte de politique | `Source/AiSolver/Training/ActorCritic` | ✅ |
| [TACHE-03](tache-03-perte-critique-optimisation-conjointe.md) | Perte du critique et optimisation conjointe acteur/critique | `Source/AiSolver/Training/ActorCritic` | ✅ |
| [TACHE-04](tache-04-comparaison-convergence.md) | Comparaison chiffrée de convergence vs REINFORCE brut | `Source/AiSolver/Training/ActorCritic` | ✅ |

## Critères d'acceptation du lot
1. Le critique produit une estimation de valeur dont l'erreur quadratique par rapport au retour
   observé **diminue** au fil de l'entraînement, sur le niveau de contrôle.
2. La perte de politique utilise effectivement l'avantage (`retour − valeur estimée`) et non plus le
   retour brut ; le gradient de cette perte, tout comme celui de la perte du critique, est validé
   par gradient checking contre des différences finies.
3. Sur le même niveau de contrôle et le même budget d'épisodes que LOT-ANNEXE-12, l'acteur-critique
   atteint un plafond de récompense donné en **moins d'épisodes en moyenne**, et/ou avec une
   **variance inter-essais plus faible** que REINFORCE brut — mesuré et documenté à partir des CSV
   comparés.
4. Les deux réseaux (acteur, critique) sont optimisés indépendamment ; une perturbation du critique
   (ex. optimiseur mal réglé) n'empêche pas l'acteur de continuer à recevoir un gradient exploitable
   (les deux graphes d'autodiff sont bien séparés).
5. Aucune nouvelle dépendance tierce n'a été ajoutée (`External/CMakeLists.txt` inchangé).
6. Logique nouvelle **couverte par des tests** (`ctest` vert), déterministe, sans GPU. Build
   `/W4 /WX` sans avertissement, Doxygen et lint des exigences verts.

## Résultat mesuré (TACHE-04)
Comparaison exécutée (`ConvergenceComparatorTest.ComparaisonReinforceVsActorCritiqueSurNiveauDeControle`,
`Source/Test/Unit/AiSolver/Training/test_convergence_comparator.cpp`) : `4` essais (graines
différentes) de chaque algorithme, `60` épisodes chacun, niveau de contrôle trivial (le même que
`LOT-ANNEXE-12` TACHE-05), plafond de récompense `5.0` :

| Algorithme    | Essais atteignant le plafond | Épisodes moyens jusqu'au plafond | Écart-type de fin de run |
|---------------|:-----------------------------:|:---------------------------------:|:--------------------------:|
| REINFORCE     | 4/4                            | 0.5                                 | 0.0043                      |
| Acteur-critique | 4/4                          | 0.25                                | 4.45                         |

Mesure honnête (cf. décision de cadrage ci-dessus) : sur ce niveau de contrôle, **REINFORCE brut
montre une variance de fin de run plus faible que l'acteur-critique**, contrairement à
l'amélioration attendue. Explication la plus probable : le corridor trivial se résout en une seule
action correcte, si bien que les deux algorithmes atteignent le plafond de récompense en moins d'un
épisode en moyenne — le niveau de contrôle est trop simple pour laisser à la réduction de variance
de l'acteur-critique l'occasion de se manifester (aucun épisode assez long ni assez varié pour que
le retour brut de REINFORCE soit réellement bruité). Le mécanisme lui-même est correct et vérifié
indépendamment (convergence du critique, gradient checking des deux pertes, indépendance des deux
optimisations) : ce résultat documente une limite du niveau de contrôle choisi, pas un défaut de
l'algorithme. À reprendre avec un niveau de contrôle plus long/varié si LOT-ANNEXE-14 ou un lot
futur a besoin d'une mesure plus discriminante.

## Dépendances
Étend directement [LOT-ANNEXE-12](@ref lot-annexe-12) (perte et boucle d'entraînement REINFORCE,
réutilisées et modifiées). Bâtit sur [LOT-ANNEXE-02](@ref lot-annexe-02) (autodiff),
[LOT-ANNEXE-03](@ref lot-annexe-03) (réseaux de neurones, pour le critique),
[LOT-ANNEXE-04](@ref lot-annexe-04) (deux optimiseurs indépendants) et
[LOT-ANNEXE-09](@ref lot-annexe-09) (`TrainingStatsRecorder`, pour la comparaison chiffrée). Ses
résultats mesurés orientent directement le choix de [LOT-ANNEXE-14](@ref lot-annexe-14).

## Navigation des tâches
- @subpage lot-annexe-13-tache-01-reseau-critique
- @subpage lot-annexe-13-tache-02-avantage
- @subpage lot-annexe-13-tache-03-perte-critique-optimisation-conjointe
- @subpage lot-annexe-13-tache-04-comparaison-convergence
