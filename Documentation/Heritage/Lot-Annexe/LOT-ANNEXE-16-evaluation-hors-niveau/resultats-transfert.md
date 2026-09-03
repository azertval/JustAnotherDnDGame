# Résultats — Évaluation hors-niveau (transfert) {#lot-annexe-16-resultats-transfert}

**Lot :** [LOT-ANNEXE-16](epic.md) · Mesures produites par
`Source/Test/Unit/AiSolver/Eval/test_cross_level_benchmark.cpp`,
`CrossLevelTransferTest.CampagneReelleDeuxPairesDeNiveaux`.

## Attentes déclarées avant mesure

L'entraînement du programme Lot-Annexe reste strictement **niveau par niveau** (décision transverse,
réaffirmée par chaque génération concernée) : aucun algorithme entraîné par ce programme n'a jamais
reçu de pression d'entraînement à généraliser au-delà du niveau qui l'a produit. Avant toute mesure,
l'attente déclarée est donc un **transfert faible, voire nul** — un résultat faible ci-dessous n'est
pas un échec du programme, c'est la confirmation attendue de son régime d'entraînement assumé. Ce
document rapporte des chiffres, il n'en tire aucune conclusion sur la qualité d'un algorithme.

## Protocole

- Modèle : réseau évolutionniste (`LOT-ANNEXE-10`/`11`) entraîné **uniquement** sur le niveau
  trivial (`TrivialLevelDirectory`, corridor à deux cases, un seul pas vers la droite), jusqu'à
  résolution stable (3 générations consécutives invaincues et résolvantes).
- Exécution croisée : `aisolver::eval::BenchmarkRunner::run` (`LOT-ANNEXE-15`, inchangé), décodage
  `Argmax`, 20 répétitions par paire, budget de 100 pas par épisode.
- Paires exécutées, choisies pour leur lisibilité pédagogique (décision de cadrage de l'épic) :

| Paire | Niveau d'entraînement (A) | Niveau d'exécution (B) | Mécaniques communes | Mécaniques différentes |
|---|---|---|---|---|
| 1 | `Trivial` (corridor 1 pas) | `TrivialLong` (corridor 3 pas) | Déplacement latéral pur, aucun saut | Largeur du corridor, position de la sortie |
| 2 | `Trivial` (corridor 1 pas) | `Gap` (brèche à sauter) | Déplacement latéral initial | Saut requis pour franchir la brèche |

## Résultats mesurés

| Paire | successRate | meanStepsAll |
|---|---|---|
| `Trivial` → `TrivialLong` | 0,000 | 100,00 (plafond, timeout systématique) |
| `Trivial` → `Gap` | 0,000 | 47,00 (épisodes clos avant le plafond, agent **bloqué**) |

> **Remesuré lors de l'audit de release v0.1.2.** La conclusion — transfert **nul** dans les
> deux cas — est inchangée. Seul le nombre moyen de pas de la paire `Gap` a bougé : il valait
> le plafond de 100 pas lors de la campagne d'origine, il vaut 47 aujourd'hui. Les épisodes ne
> vont plus au bout du budget, ils se closent plus tôt sur un état **bloqué** — l'agent
> s'immobilise devant la brèche au lieu de s'agiter jusqu'à l'épuisement du budget. Un test
> rendu falsifiable a révélé l'écart ; il ne fige plus cette valeur, seulement le taux de
> réussite nul et la borne du budget, pour que la mesure reste vérifiable sans être fragile.

À titre de repère (mesure non croisée, même modèle, niveau d'origine, `LOT-ANNEXE-15`) : ce même
modèle résout systématiquement `Trivial` par construction (critère d'arrêt de l'entraînement,
`LevelTrainingSession`).

## Lecture des résultats

Le transfert mesuré est **nul dans les deux cas** — y compris sur la paire de mécanique partagée
(`TrivialLong`), ce qui n'était pas garanti a priori : on aurait pu s'attendre à un transfert au
moins partiel pour une géométrie ne demandant que la même mécanique de déplacement, sur un corridor
simplement plus long. Ce n'est pas ce qui est observé ici. Une explication plausible, non vérifiée
par ce lot (hors périmètre — ce lot mesure, il n'investigue pas la cause) : `ObservationEncoder`
(`LOT-ANNEXE-06`) encode une fenêtre/des repères propres à la géométrie du niveau d'entraînement, et
la politique, sans pression de généralisation, a pu s'ancrer sur des repères spécifiques à cette
géométrie précise plutôt que sur une règle de déplacement transférable — cohérent avec l'attente
déclarée d'un transfert faible, ici confirmée de façon particulièrement nette (nul plutôt que
partiel). Aucune conclusion n'est tirée sur la qualité de l'algorithme évolutionniste en particulier :
la génération 4 ne mesure qu'un modèle par cette campagne, pas l'ensemble des algorithmes de
générations 2/3 sur ce point précis.
