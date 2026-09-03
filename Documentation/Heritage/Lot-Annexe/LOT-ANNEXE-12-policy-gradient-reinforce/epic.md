# LOT-ANNEXE-12 — Policy gradient maison (REINFORCE) {#lot-annexe-12}

> Statut : **fait**. Prérequis : [LOT-ANNEXE-02](@ref lot-annexe-02) (autodiff),
> [LOT-ANNEXE-04](@ref lot-annexe-04) (optimiseurs), [LOT-ANNEXE-05](@ref lot-annexe-05)
> (environnement headless), [LOT-ANNEXE-08](@ref lot-annexe-08) (récompense),
> [LOT-ANNEXE-09](@ref lot-annexe-09) (statistiques). Premier lot de la génération 3
> (apprentissage par gradient) et point d'entrée de l'exigence ferme d'un modèle **appris par
> rétropropagation**, par opposition à la recherche aveugle par population de la génération 2
> ([LOT-ANNEXE-10](@ref lot-annexe-10)).

## Objectif
La génération 2 a produit un agent fonctionnel — l'algorithme évolutionniste de LOT-ANNEXE-10 fait
progresser une population de politiques par sélection/mutation, sans jamais calculer le moindre
gradient. C'est une ligne de base valide, mais elle n'exploite **rien** de la génération 0
(bibliothèque tensorielle, moteur d'autodiff maison avec `backward()` et gradient checking) : le
gradient de la récompense par rapport aux paramètres de la politique n'est jamais calculé ni
utilisé pour orienter la recherche.

Ce lot introduit **REINFORCE**, l'algorithme de policy gradient le plus direct : la politique est
un réseau de neurones (bibliothèque de LOT-ANNEXE-03) qui produit une distribution sur l'espace
d'action ([LOT-ANNEXE-07](@ref lot-annexe-07)) ; on **échantillonne** une action selon cette
distribution, on joue un épisode complet sur l'environnement headless (LOT-ANNEXE-05), et on
**rétropropage** — via le moteur d'autodiff maison de LOT-ANNEXE-02, pas une approximation — le
gradient de `-log(probabilité de l'action prise) × retour` à travers le réseau, puis on met à jour
les poids avec un optimiseur de LOT-ANNEXE-04 (SGD ou Adam). C'est la **première fois** dans le
programme que la mise à jour des poids est **dirigée par un gradient calculé exactement**, plutôt
que par un tirage aléatoire de mutations retenu ou rejeté après coup. L'utilisateur a explicitement
écarté de s'arrêter à l'évolutionniste seul : ce lot est la réponse directe à cette exigence.

L'entraînement reste **niveau par niveau**, comme tous les lots des générations 2 et 3 (aucun
entraînement joint multi-niveaux, cf. [lots-annexe.md](@ref lots-annexe)).

## Périmètre

### Inclus
- **Collecte de trajectoires** sur un épisode complet (un niveau, du départ jusqu'à victoire, échec
  ou timeout au sens de LOT-ANNEXE-08) : à chaque pas, observation encodée (LOT-ANNEXE-06), action
  échantillonnée selon la distribution produite par le réseau de politique, log-probabilité de
  cette action, récompense immédiate — via `HeadlessLevelEnvironment` (LOT-ANNEXE-05).
- **Calcul du retour par pas** (`return`), somme des récompenses futures de l'épisode, avec un
  facteur d'actualisation `gamma` configurable (Monte-Carlo complet, sans bootstrap ni baseline —
  la réduction de variance par critique est le sujet du lot suivant, LOT-ANNEXE-13).
- **Perte REINFORCE** (`-log π(a|s) × retour`, sommée ou moyennée sur l'épisode) construite comme
  un graphe du moteur d'autodiff maison (LOT-ANNEXE-02) et rétropropagée par son `backward()` —
  **aucune** formule de gradient écrite à la main : le gradient traverse le réseau de politique
  (LOT-ANNEXE-03) exactement comme n'importe quel autre graphe de calcul du moteur.
- **Boucle d'entraînement par épisodes** : collecte → retours → perte → `backward()` → pas
  d'optimiseur (LOT-ANNEXE-04) → journalisation (`TrainingStatsRecorder`, LOT-ANNEXE-09) — répétée
  sur un budget d'épisodes configurable, sur un seul niveau à la fois.
- Tests de non-régression numérique : **gradient checking** spécifique à la perte REINFORCE
  (différences finies vs gradient rétropropagé, même infrastructure que LOT-ANNEXE-02), et suivi de
  progression (récompense moyenne croissante) sur un niveau de contrôle simple et déterministe.

### Exclus (hors périmètre de ce lot)
- **Réduction de variance (baseline/critique)** : REINFORCE brut a une variance de gradient connue
  pour être élevée (le retour complet sert de signal, sans rien pour le recentrer) — c'est traité
  spécifiquement par LOT-ANNEXE-13 (acteur-critique), qui réutilise directement la perte de ce lot
  en y substituant l'avantage au retour brut.
- **PPO, DQN et toute variante avancée** : LOT-ANNEXE-14, qui suppose REINFORCE et
  l'acteur-critique déjà en place pour choisir entre les deux sur la base de résultats mesurés.
- **Comparaison chiffrée rigoureuse contre l'évolutionniste** (nombre d'épisodes jusqu'au plafond,
  écart-type inter-essais) : ce lot montre qu'un signal de gradient fonctionne (tendance
  d'apprentissage dirigée, visible dans les CSV) ; le bake-off chiffré entre approches est le rôle
  de LOT-ANNEXE-13 (TACHE-04) et de la génération 4, pas de ce lot d'introduction.
- **Export en séquence d'actions déterministe rejouable en jeu** : déjà couvert par le format v1 de
  LOT-ANNEXE-07 et l'export de LOT-ANNEXE-11 ; ce lot produit un modèle entraîné, pas un rejeu.
- **Entraînement multi-niveaux ou généralisation** : hors régime niveau-par-niveau (décision
  transverse de tout le programme, mesurée en génération 4).

## Décisions de cadrage
- **Retour Monte-Carlo complet, sans baseline.** Un épisode se termine toujours (victoire, échec ou
  timeout, LOT-ANNEXE-08) : le retour de chaque pas peut donc être calculé exactement en fin
  d'épisode, sans bootstrap sur une estimation de valeur. Introduire une baseline maintenant
  anticiperait sur le rôle du critique de LOT-ANNEXE-13 ; ce lot reste volontairement l'algorithme
  le plus simple qui exploite réellement l'autodiff.
- **Échantillonnage stochastique de l'action pendant l'entraînement, jamais argmax.** REINFORCE a
  besoin d'explorer pour que le gradient de `log π(a|s)` ait un sens (une action toujours certaine
  a un gradient de log-probabilité nul) ; le mode déterministe (argmax) n'intervient qu'au moment
  de figer une politique pour l'export en rejeu (LOT-ANNEXE-11), pas ici.
- **La perte est un graphe d'autodiff, jamais une formule de gradient codée à la main.** Le point
  du lot est d'exploiter le moteur de LOT-ANNEXE-02 : `logProbability` est lui-même un nœud du
  graphe (sortie du réseau de politique), et `backward()` calcule le gradient de bout en bout. Coder
  `∂perte/∂poids` directement contournerait l'autodiff et viderait le lot de son objet.
- **`gamma` par défaut proche de 1** (ex. `0.99`), configurable par expérience : les niveaux visés
  sont courts (quelques centaines de pas au plus), l'actualisation sert surtout à atténuer le bruit
  des récompenses très tardives plutôt qu'à modéliser un horizon infini.
- **Un épisode = un niveau joué jusqu'à son critère de fin** (LOT-ANNEXE-08), pas un nombre de pas
  fixe : la longueur d'épisode varie selon que l'agent réussit vite, échoue vite ou va jusqu'au
  timeout, ce que la boucle d'entraînement doit accepter sans hypothèse de longueur constante.
- **Aucune normalisation de retour inter-épisodes dans ce lot** (ex. centrage/réduction sur un lot
  d'épisodes) : à un seul épisode par mise à jour, une telle normalisation n'a pas de population sur
  laquelle se calculer ; elle redeviendrait pertinente avec des mises à jour par lot, hors périmètre
  ici.

## Notions abordées
Voir @ref guide-annexe-reinforce (astuce du log-gradient, règle de mise à jour REINFORCE, retour
actualisé). Sources directes : Williams (1992, article d'origine de REINFORCE) ; Sutton, McAllester,
Singh, Mansour (1999/2000, théorème du gradient de politique) — bibliographie complète dans le
chapitre.

## Exigences couvertes
- Nouvelle, déclarée dans [la spécification IA](@ref spec-ia) : [`EX-IA-013`](@ref EX-IA-013).
- Réutilisées (inchangées) : les exigences des lots amont — bibliothèque tensorielle et RNG
  (LOT-ANNEXE-01), moteur d'autodiff et gradient checking (LOT-ANNEXE-02), bibliothèque de réseaux
  de neurones (LOT-ANNEXE-03), optimiseurs SGD/Adam (LOT-ANNEXE-04), environnement headless
  (LOT-ANNEXE-05), encodage d'observation (LOT-ANNEXE-06), espace d'action et format de rejeu v1
  (LOT-ANNEXE-07), récompense et critères d'épisode (LOT-ANNEXE-08), `TrainingStatsRecorder`
  (LOT-ANNEXE-09) — non renumérotées ici.

## Découpage

> État : ✅ fait · 🔄 en cours · ⬜ non commencé.

| Tâche | Intitulé | Emplacement | État |
|-------|----------|-------------|:----:|
| [TACHE-01](tache-01-collecte-trajectoires.md) | Collecte de trajectoires sur un épisode complet | `Source/AiSolver/Training/PolicyGradient` | ✅ |
| [TACHE-02](tache-02-calcul-retour.md) | Calcul du retour actualisé par pas | `Source/AiSolver/Training/PolicyGradient` | ✅ |
| [TACHE-03](tache-03-perte-reinforce.md) | Perte REINFORCE rétropropagée via l'autodiff maison | `Source/AiSolver/Training/PolicyGradient` | ✅ |
| [TACHE-04](tache-04-boucle-entrainement.md) | Boucle d'entraînement par épisodes et journalisation | `Source/AiSolver/Training/PolicyGradient` | ✅ |
| [TACHE-05](tache-05-tests-gradient-checking.md) | Tests : gradient checking et non-régression de progression | `Source/Test/Unit/AiSolver/Training` | ✅ |

## Critères d'acceptation du lot
1. Sur un niveau de contrôle simple et déterministe, la récompense moyenne par épisode **progresse**
   sur la durée de l'entraînement (tendance croissante mesurable dans le CSV produit par
   `TrainingStatsRecorder`), signe d'un apprentissage effectivement dirigé par le gradient.
2. Le gradient de la perte REINFORCE, calculé par `backward()` du moteur d'autodiff maison,
   correspond au gradient obtenu par différences finies à la tolérance retenue par LOT-ANNEXE-02
   (gradient checking vert).
3. L'action jouée à chaque pas de collecte est bien **échantillonnée** selon la distribution de la
   politique (stochastique), jamais choisie par argmax pendant l'entraînement.
4. Le retour par pas est calculé correctement pour des épisodes de longueur variable (victoire
   rapide, échec rapide, timeout), sans hypothèse de longueur fixe.
5. Aucune nouvelle dépendance tierce n'a été ajoutée (`External/CMakeLists.txt` inchangé).
6. Logique nouvelle **couverte par des tests** (`ctest` vert), déterministe, sans GPU. Build
   `/W4 /WX` sans avertissement, Doxygen et lint des exigences verts.

## Dépendances
Bâtit sur [LOT-ANNEXE-02](@ref lot-annexe-02) (moteur d'autodiff, `backward()`, gradient checking),
[LOT-ANNEXE-03](@ref lot-annexe-03) (réseaux de neurones), [LOT-ANNEXE-04](@ref lot-annexe-04)
(optimiseurs), [LOT-ANNEXE-05](@ref lot-annexe-05) (`HeadlessLevelEnvironment`),
[LOT-ANNEXE-06](@ref lot-annexe-06) (encodage d'observation), [LOT-ANNEXE-07](@ref lot-annexe-07)
(espace d'action), [LOT-ANNEXE-08](@ref lot-annexe-08) (récompense, critères d'épisode) et
[LOT-ANNEXE-09](@ref lot-annexe-09) (`TrainingStatsRecorder`). Sert de base commune à
[LOT-ANNEXE-13](@ref lot-annexe-13) (acteur-critique, qui réutilise sa perte) et, en aval,
à [LOT-ANNEXE-14](@ref lot-annexe-14). Comparé, sans en dépendre techniquement, à
[LOT-ANNEXE-10](@ref lot-annexe-10) (évolutionniste, ligne de base sans gradient).

## Navigation des tâches
- @subpage lot-annexe-12-tache-01-collecte-trajectoires
- @subpage lot-annexe-12-tache-02-calcul-retour
- @subpage lot-annexe-12-tache-03-perte-reinforce
- @subpage lot-annexe-12-tache-04-boucle-entrainement
- @subpage lot-annexe-12-tache-05-tests-gradient-checking
