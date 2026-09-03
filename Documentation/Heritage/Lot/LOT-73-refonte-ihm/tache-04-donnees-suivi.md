# TACHE-04 — Les données que le moteur produit déjà {#lot-73-tache-04-donnees-suivi}

**Lot :** [LOT-73](epic.md) · **Emplacement :** `Source/AiSolver/Stats`, `Source/HMI/Ai` ·
**Statut :** fait

## Contexte
`TrainingStatsRecorder::record()` calcule, à chaque ligne, la **moyenne mobile** de la meilleure
récompense et sa **variation** depuis la ligne précédente. Les deux sont écrites dans le `stats.csv`
du run — puis perdues : l'observateur branché par `setOnRecord` ne recevait que la ligne brute.

Un appelant qui voulait tracer une courbe lissée n'avait donc que deux recours, tous deux mauvais :
relire le fichier que l'enregistreur venait d'écrire, ou refaire le calcul. L'écran ne traçait par
conséquent que des courbes brutes, sur lesquelles une progression lente est indiscernable du bruit
d'une génération à l'autre — précisément ce que la moyenne mobile existe pour montrer.

Même motif côté DQN : `DqnTrainer::totalSteps()` est le compteur dont dépend la décroissance
d'`epsilon`, et donc la seule façon de lire cette décroissance autrement que comme un nombre qui
baisse tout seul. Le trainer l'exposait, aucun appelant ne le lisait.

## Travail réalisé
- **`aisolver::TrainingStatsDerived`** (`Stats/TrainingStatsRecorder.h`) : structure des grandeurs
  que l'enregistreur **calcule**, distincte de `TrainingStatsRow` que l'appelant **fournit**. La
  distinction n'est pas cosmétique : ces valeurs dépendent de l'historique du run, que seul
  l'enregistreur tient.
- **`setOnRecord` élargi** à `void(const TrainingStatsRow&, const TrainingStatsDerived&)`, et
  `LevelTrainingSession::setOnStatsRow` avec lui — les grandeurs sont **transmises**, jamais
  recalculées par l'observateur.
- **`hmi::TrainingProgress`** gagne `movingAverageReward`, `rewardDelta` et `totalSteps` (chemin
  DQN), renseignés par les quatre branches d'algorithme.
- **Le graphique trace la moyenne mobile** comme quatrième courbe, et l'inclut dans le calcul de son
  échelle — tracée hors bornes, elle sortirait du cadre.

## Vérification
- `TrainingStatsRecorderTest.ObservateurRecoitLesGrandeursDerivees` (nouveau) : sur trois lignes de
  récompenses croissantes, la variation est nulle à la première ligne (aucune précédente à comparer)
  puis strictement positive, et la moyenne mobile monte.
- `TrainingStatsRecorderTest.SetOnRecordObserveSansEffetDeBord` : adapté à la nouvelle signature,
  son invariant inchangé.

## Ce que cette tâche n'a pas fait
Quatre autres grandeurs restent produites sans être affichées, et sont laissées à un cadrage
ultérieur plutôt qu'ajoutées à la hâte : la **perte du critique** (`ActorCriticTrainer` sait
l'écrire, aucun appelant ne passe `criticLossCsvPath` — ni l'IHM, ni la ligne de commande), le
**remplissage de la mémoire de rejeu** DQN, la répartition détaillée des `EpisodeStatus`
(`Won`/`Lost`/`TimedOut`/`Stuck`, l'information de diagnostic qui distingue « meurt » de « tourne en
rond » de « manque de budget ») et le détail par répétition de `BenchmarkResult::episodes`. Les
exposer suppose de décider **où** dans l'écran, ce qui relève d'un travail d'architecture de
l'information et non du câblage mené ici.
