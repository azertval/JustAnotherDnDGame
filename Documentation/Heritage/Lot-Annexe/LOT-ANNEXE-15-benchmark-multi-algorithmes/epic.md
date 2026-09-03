# LOT-ANNEXE-15 — Harnais de benchmark multi-algorithmes {#lot-annexe-15}

> Statut : **fait**. Prérequis : [LOT-ANNEXE-05](@ref lot-annexe-05) (`HeadlessLevelEnvironment`),
> [LOT-ANNEXE-06](@ref lot-annexe-06) (encodage d'observation), [LOT-ANNEXE-09](@ref lot-annexe-09)
> (`TrainingStatsRecorder`) et **au moins un** algorithme entraîné sur un niveau
> ([LOT-ANNEXE-10](@ref lot-annexe-10)/[LOT-ANNEXE-11](@ref lot-annexe-11) — évolutionniste — ou
> [LOT-ANNEXE-12](@ref lot-annexe-12)/[LOT-ANNEXE-13](@ref lot-annexe-13)/[LOT-ANNEXE-14](@ref
> lot-annexe-14) — apprentissage par gradient). Premier lot de la génération 4 (évaluation et
> robustesse).

## Objectif
L'entraînement reste **niveau par niveau** (décision transverse du programme Lot-Annexe) : chaque
lot des générations 2 et 3 produit un **modèle** au sens strict d'une paire (algorithme, niveau).
Jusqu'ici, la seule preuve qu'un modèle « fonctionne » est le rejeu final déterministe
(`LOT-ANNEXE-07`, format v1) — une **unique** trajectoire, jouée une seule fois, qui prouve qu'un
niveau peut être terminé mais ne dit rien sur la **fiabilité** de la politique sous-jacente ni sur
sa **qualité relative** face à un autre algorithme. Or `LOT-ANNEXE-13` a déjà introduit une
comparaison chiffrée REINFORCE/acteur-critique **locale** à la génération 3 ; il manque un outillage
**générique**, indépendant de la famille d'algorithme, capable de répondre objectivement à « l'agent
avancé de `LOT-ANNEXE-14` termine-t-il réellement plus souvent, plus vite ou plus stablement que la
ligne de base évolutionniste de `LOT-ANNEXE-10` ? » — question qui traverse toute la génération 2 et
3, pas seulement REINFORCE contre acteur-critique.

Ce lot livre ce harnais : exécution automatisée et répétée d'un modèle entraîné sur son niveau
**d'origine**, un rapport comparatif inter-algorithmes au format CSV déjà en usage
(`TrainingStatsRecorder`), et un test de robustesse au bruit d'observation qui distingue une
politique réellement robuste d'une politique en surapprentissage fragile (haute performance
uniquement sur la trajectoire exacte vue à l'entraînement).

## Périmètre

### Inclus
- **Exécution automatisée répétée** (`TACHE-01`) : pour un modèle (algorithme, niveau) donné, `N`
  répétitions de la politique **entraînée** — pas du rejeu déterministe figé — sur
  `HeadlessLevelEnvironment` (`LOT-ANNEXE-05`), avec mesure du taux de réussite, du nombre de pas
  moyen et de la variance.
- **Rapport comparatif CSV** (`TACHE-02`) : agrégation de plusieurs campagnes d'exécution en un
  tableau (niveau × algorithme), au format de colonnes de `TrainingStatsRecorder` (`LOT-ANNEXE-09`)
  adapté à une exécution **post-entraînement** plutôt qu'à une génération/un épisode d'entraînement.
- **Test de robustesse au bruit d'observation** (`TACHE-03`) : perturbation légère et contrôlée de
  l'observation encodée (`LOT-ANNEXE-06`) transmise à la politique, sans jamais toucher l'état réel
  simulé — révèle un écart de performance entre observation propre et observation bruitée.
- Une abstraction minimale (`TrainedPolicy`) permettant au harnais de traiter n'importe quel modèle
  entraîné (évolutionniste, REINFORCE, acteur-critique, algorithme avancé) de façon uniforme, sans
  connaître les détails internes de l'algorithme qui l'a produit.

### Exclus (hors périmètre de ce lot)
- **Exécution croisée entre niveaux** (modèle entraîné sur A, exécuté sur B) : c'est tout le
  périmètre de `LOT-ANNEXE-16`, qui réutilise ce harnais sans le dupliquer.
- **Toute modification d'un algorithme d'entraînement ou d'un modèle déjà produit** : ce lot ne fait
  que **lire** des fichiers de modèle déjà entraînés, en lecture seule ; aucun réentraînement, aucun
  réglage d'hyperparamètre déclenché depuis `Source/AiSolver/Eval`.
- **Décision automatique du « meilleur » algorithme** : le harnais produit des chiffres (taux de
  réussite, pas moyen, variance, écart au bruit) ; l'arbitrage sur quel modèle est retenu reste une
  lecture humaine des rapports, hors périmètre technique de ce lot.
- **Test adversarial ciblé** (perturbation optimisée pour faire échouer délibérément la politique) :
  `TACHE-03` reste un bruit **léger** et non dirigé, pas une recherche d'exemples adversariaux —
  amplitude documentée et bornée, calibrée pour rester exploitable, pas pour maximiser l'échec.
- **Parallélisation de l'exécution des répétitions** : séquentiel, un épisode après l'autre, comme
  le reste du programme Lot-Annexe (`LOT-ANNEXE-10`) — optimisation potentielle future, non requise
  ici.

## Décisions de cadrage
- **Le harnais évalue la politique entraînée elle-même, pas le rejeu déterministe final.** Le format
  de rejeu v1 (`LOT-ANNEXE-07`) est une trace unique, figée, destinée à être rejouée en jeu ; ce lot
  réexécute la politique en environnement headless pour mesurer une **fiabilité statistique** que le
  rejeu seul ne peut pas donner. `BenchmarkConfig` expose un `ActionDecodingMode` (`Argmax` /
  `Stochastic`) : pour un modèle évolutionniste, toujours décodé en `Argmax` (`LOT-ANNEXE-10` :
  aucune autre décision de décodage n'existe pour cette famille — les répétitions sont alors
  strictement identiques, ce qui est un résultat attendu et sert de garde de cohérence croisée) ;
  pour un modèle de gradient de politique, `Stochastic` par défaut (échantillonnage selon la
  distribution produite, comme à l'entraînement), avec un passage en `Argmax` disponible pour
  vérifier que la mesure « sans hasard » du harnais coïncide avec le rejeu exporté par
  `LOT-ANNEXE-11`.
- **Toute la stochasticité du harnais passe par une seule instance d'`aisolver::Rng`**, dérivée de
  façon déterministe d'une graine de base et de l'indice de répétition — jamais `std::rand`, jamais
  l'horloge (même contrainte que `LOT-ANNEXE-01`/`LOT-ANNEXE-10`) : condition nécessaire à la
  reproductibilité exacte d'une campagne de benchmark.
- **Chaque famille d'algorithme est exposée au harnais via un adaptateur `TrainedPolicy` fin**, pas
  via un couplage direct au code d'entraînement : `Source/AiSolver/Eval` **dépend** des fichiers de
  modèle produits par `Source/AiSolver/Training/*`, jamais l'inverse — un module de mesure ne doit
  pas alourdir les modules d'entraînement de préoccupations qui leur sont étrangères.
- **Le bruit d'observation (`TACHE-03`) ne perturbe que ce que la politique perçoit, jamais l'état
  réel simulé par `HeadlessLevelEnvironment`.** L'issue (`core::LevelOutcome`, via
  `core::evaluateOutcome`) reste jugée sur l'état réel non perturbé : seule l'entrée de la politique
  change, ce qui isole la robustesse de la **perception** de la politique de la difficulté propre du
  niveau.
- **Aucune nouvelle dépendance tierce, aucun calcul GPU** : le harnais reste du C++ pur, aligné sur
  la contrainte transverse « from scratch » de tout le programme Lot-Annexe (`LOT-ANNEXE-01`).
- **La comparaison inter-algorithmes reste descriptive, jamais normative.** Le harnais rapporte des
  chiffres ; il ne déclenche aucune action (sélection automatique d'un modèle, arrêt d'un
  entraînement) — cohérent avec le principe général de la génération 4 : mesurer, pas décider à la
  place d'un humain.

## Notions abordées
Voir @ref guide-annexe-evaluation-rl (pourquoi une seule réussite ne prouve rien, taux de réussite
répété, robustesse au bruit). Source directe : Henderson, Islam, Bachman, Pineau, Precup, Meger
(2018, *Deep Reinforcement Learning that Matters*) — justification empirique directe de la
nécessité de répéter les exécutions plutôt que de se fier à un seul essai.

## Exigences couvertes
- Nouvelle, déclarée dans [la spécification IA](@ref spec-ia) : [`EX-IA-016`](@ref EX-IA-016).
- Réutilisées (inchangées) : `EX-NFR-002` (déterminisme au pas fixe), `EX-NFR-010` (testable sans
  fenêtre ni GPU), `EX-NFR-013` (`/W4 /WX`), `EX-NFR-020` (couverture par tests), `EX-NFR-040`
  (erreur récupérable signalée, pas de plantage) et les exigences des lots amont —
  `HeadlessLevelEnvironment` (`LOT-ANNEXE-05`), encodage d'observation (`LOT-ANNEXE-06`), espace
  d'action et décodage (`LOT-ANNEXE-07`), `TrainingStatsRecorder` (`LOT-ANNEXE-09`) et l'algorithme
  entraîné évalué (`LOT-ANNEXE-10`/`11` ou `12`–`14`) — identifiants propres non repris ici, hors
  périmètre de rédaction de ce lot.

## Découpage

> État : ✅ fait · 🔄 en cours · ⬜ non commencé.

| Tâche | Intitulé | Emplacement | État |
|-------|----------|-------------|:----:|
| [TACHE-01](tache-01-execution-automatisee.md) | Exécution automatisée répétée d'un modèle sur son niveau d'origine | `Source/AiSolver/Eval` | ✅ |
| [TACHE-02](tache-02-rapport-comparatif-csv.md) | Rapport comparatif CSV par niveau × algorithme | `Source/AiSolver/Eval` | ✅ |
| [TACHE-03](tache-03-robustesse-bruit.md) | Test de robustesse au bruit d'observation | `Source/AiSolver/Eval` | ✅ |

## Critères d'acceptation du lot
1. Pour un modèle (algorithme, niveau) donné, `N` répétitions produisent un taux de réussite, un
   nombre de pas moyen (distingué entre « tous épisodes » et « épisodes réussis ») et une variance,
   reproductibles à graine de base fixée.
2. Un rapport CSV agrège ces mesures pour toutes les combinaisons niveau × algorithme exécutées dans
   une même campagne, avec les mêmes conventions de colonnes/échappement que `TrainingStatsRecorder`.
3. Le test de bruit compare, pour chaque modèle testé, le taux de réussite avec et sans perturbation
   de l'observation, sans qu'aucune perturbation n'atteigne l'état réel simulé.
4. Un modèle évolutionniste (décodage `Argmax` uniquement) produit des répétitions strictement
   identiques (variance nulle), vérifié explicitement comme garde de cohérence, pas ignoré comme cas
   dégénéré.
5. Aucune ligne de `Source/Core`, `Source/HMI` ni d'aucun module `Source/AiSolver/Training/*` n'est
   modifiée par ce lot (vérifiable par diff) ; aucune nouvelle dépendance tierce.
6. Logique nouvelle **couverte par des tests** (`ctest` vert), déterministe, sans GPU. Build
   `/W4 /WX` sans avertissement, Doxygen et lint des exigences verts.

## Dépendances
Consomme, en lecture seule, `HeadlessLevelEnvironment` (`LOT-ANNEXE-05`), l'encodage d'observation
(`LOT-ANNEXE-06`), l'espace d'action et son décodage (`LOT-ANNEXE-07`) et `TrainingStatsRecorder`
(`LOT-ANNEXE-09`). Requiert au moins un modèle entraîné produit par `LOT-ANNEXE-10`/`LOT-ANNEXE-11`
(évolutionniste) ou `LOT-ANNEXE-12`/`LOT-ANNEXE-13`/`LOT-ANNEXE-14` (apprentissage par gradient).
`LOT-ANNEXE-16` dépend de celui-ci (réutilise intégralement le harnais pour l'exécution croisée).

## Navigation des tâches
- @subpage lot-annexe-15-tache-01-execution-automatisee
- @subpage lot-annexe-15-tache-02-rapport-comparatif-csv
- @subpage lot-annexe-15-tache-03-robustesse-bruit
