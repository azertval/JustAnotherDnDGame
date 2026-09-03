# LOT-ANNEXE-14 — Algorithme avancé (PPO maison ou Q-learning à mémoire de rejeu) {#lot-annexe-14}

> Statut : **fait**. Prérequis : [LOT-ANNEXE-13](@ref lot-annexe-13) (acteur-critique, dont
> les résultats chiffrés orientent le choix d'algorithme de ce lot). Bénéficie de
> [LOT-ANNEXE-05](@ref lot-annexe-05) (environnement headless), [LOT-ANNEXE-07](@ref lot-annexe-07)
> (espace d'action) et [LOT-ANNEXE-09](@ref lot-annexe-09) (statistiques). Troisième et dernier lot
> de la génération 3 (apprentissage par gradient).

## Objectif
LOT-ANNEXE-12 a établi qu'un gradient de récompense calculé par autodiff maison peut orienter
l'apprentissage (REINFORCE) ; LOT-ANNEXE-13 a réduit la variance de ce gradient (acteur-critique) et
produit une **mesure chiffrée** de gain de stabilité/vitesse de convergence par rapport à REINFORCE
brut. Ce lot est la dernière marche de sophistication de la génération 3 : un algorithme plus avancé
que l'acteur-critique simple, choisi **sur la base de ces mesures** plutôt que par préférence a
priori.

Deux familles répondent à la même limitation générale de l'acteur-critique — des mises à jour
encore potentiellement trop grandes ou trop bruitées d'un épisode à l'autre — par des mécanismes
différents :
- **PPO** (Proximal Policy Optimization) borne explicitement l'amplitude de chaque mise à jour de
  politique (clipping du ratio de probabilité entre ancienne et nouvelle politique), ce qui cible
  directement un problème d'**instabilité** de la mise à jour elle-même.
- **DQN** (Deep Q-learning) change de paradigme : plutôt qu'une politique qui produit une
  distribution d'action, un réseau estime la valeur de chaque action possible (`Q(s, a)`), avec une
  **mémoire de rejeu** (`ReplayBuffer`) qui réutilise des transitions passées pour stabiliser
  l'apprentissage et une exploration `ε`-greedy — ce qui cible plutôt un problème d'**efficacité
  d'échantillonnage** (chaque transition vécue sert plusieurs fois) et convient particulièrement
  bien à un espace d'action **discret** comme celui déjà retenu en LOT-ANNEXE-07.

## Périmètre

### Inclus
- **Décision de cadrage propre à ce lot, tranchée à son ouverture** (voir TACHE-01) : lecture des
  résultats chiffrés de LOT-ANNEXE-13 (TACHE-04) pour choisir entre PPO et DQN, avec les critères de
  décision explicités ci-dessous. Ce choix n'est **volontairement pas fait** dans cet épic.
- **PPO**, si retenu : clipping du ratio de probabilité `π_nouvelle(a|s) / π_ancienne(a|s)` dans la
  perte de politique, plusieurs époques d'optimisation par batch de trajectoires collectées (par
  opposition à la seule passe par épisode de LOT-ANNEXE-12/13).
- **DQN**, si retenu : réseau `Q(s, a)` (une sortie par action de l'espace discret de
  LOT-ANNEXE-07), `ReplayBuffer` (mémoire des transitions passées, échantillonnage aléatoire pour la
  mise à jour), réseau cible (`target network`) mis à jour périodiquement pour stabiliser la cible de
  l'erreur quadratique de Bellman, exploration `ε`-greedy avec décroissance de `ε` au fil de
  l'entraînement.
- **Intégration au harnais d'entraînement existant** : `HeadlessLevelEnvironment` (LOT-ANNEXE-05),
  journalisation via `TrainingStatsRecorder` (LOT-ANNEXE-09), quel que soit l'algorithme retenu.
- **Comparaison chiffrée finale** de la génération 3 : l'algorithme avancé retenu contre la
  génération 2 (évolutionniste, LOT-ANNEXE-10) et contre le reste de la génération 3 (REINFORCE
  LOT-ANNEXE-12, acteur-critique LOT-ANNEXE-13), sur un même jeu de niveaux de contrôle, via les CSV
  de LOT-ANNEXE-09 — clôture de la démonstration que l'apprentissage par gradient dépasse la
  recherche aveugle, exigence ferme du programme.

### Exclus (hors périmètre de ce lot)
- **L'algorithme non retenu** : ce lot implémente **un seul** des deux (PPO ou DQN), pas les deux —
  documenter les deux options dans ce périmètre sert à informer la décision de TACHE-01, pas à les
  développer en parallèle.
- **Double DQN, Dueling DQN, Prioritized Experience Replay** ou toute variante avancée de DQN au-delà
  du DQN de base (réseau cible + mémoire de rejeu + `ε`-greedy) — raffinements possibles d'un lot
  futur, non nécessaires pour clore la génération 3.
- **GAE (avantage généralisé) pour PPO** : si PPO est retenu, il réutilise l'avantage à un pas déjà
  défini en LOT-ANNEXE-13, sans lissage supplémentaire — cohérent avec la même décision de cadrage
  déjà actée pour l'acteur-critique.
- **Généralisation multi-niveaux** : reste hors régime niveau-par-niveau, mesurée en génération 4.
- **Export en séquence de rejeu déterministe** : inchangé par rapport à LOT-ANNEXE-11, quel que soit
  l'algorithme retenu ici.

## Décisions de cadrage
- **Le choix entre PPO et DQN est explicitement différé à l'ouverture de ce lot**, tranché sur la
  base des résultats mesurés (CSV) de LOT-ANNEXE-13 : c'est une décision de cadrage assumée de ce
  lot, pas une lacune de rédaction. Deux critères guident le choix au moment de l'ouvrir :
  - Si les CSV de LOT-ANNEXE-13 montrent une **instabilité persistante** de l'acteur-critique
    (variance de récompense en fin de run encore élevée, courbes de convergence irrégulières), PPO
    cible directement cette instabilité en bornant l'amplitude des mises à jour de politique.
  - Si l'instabilité est déjà largement résorbée par l'acteur-critique mais que la **vitesse de
    convergence** (nombre d'épisodes) reste le facteur limitant, et si l'espace d'action discret de
    LOT-ANNEXE-07 s'y prête bien (nombre d'actions raisonnable, pas d'action continue), DQN cible
    plutôt l'efficacité d'échantillonnage via la réutilisation des transitions passées.
- **Un seul algorithme est implémenté, jamais les deux en parallèle** : diluer l'effort entre deux
  implémentations partielles serait moins utile qu'une seule implémentation complète, correctement
  testée et comparée — cohérent avec le principe déjà appliqué à chaque lot précédent (un mécanisme
  à la fois, complet).
- **Le harnais d'entraînement (environnement, statistiques) est commun aux deux options** : quel que
  soit le choix, `HeadlessLevelEnvironment` et `TrainingStatsRecorder` sont réutilisés sans
  modification de leur contrat — seule la boucle d'entraînement et la perte diffèrent, pas
  l'infrastructure amont.
- **La comparaison finale de génération inclut systématiquement l'évolutionniste** (LOT-ANNEXE-10),
  pas seulement les algorithmes de gradient entre eux : c'est le point de clôture de l'exigence ferme
  de l'utilisateur — démontrer, chiffres à l'appui, qu'un apprentissage dirigé par gradient dépasse
  la recherche aveugle par population.

## Notions abordées
Voir @ref guide-annexe-ppo-dqn (les deux options complètes : clipping du ratio pour PPO ;
équation de Bellman, mémoire de rejeu, réseau cible, `ε`-greedy pour DQN). Sources directes : PPO —
Schulman et al. (2017) ; DQN — Mnih et al. (2013, 2015) et Watkins (1989, Q-learning d'origine) —
bibliographie complète dans le chapitre, à relire avant de trancher entre les deux options à
l'ouverture de ce lot.

## Exigences couvertes
- Nouvelle, déclarée dans [la spécification IA](@ref spec-ia) : [`EX-IA-015`](@ref EX-IA-015).
- Réutilisées (inchangées) : `EX-IA-013` (perte de policy gradient, base commune), `EX-IA-014`
  (avantage, si PPO est retenu) et les exigences des lots amont — environnement headless
  (LOT-ANNEXE-05), espace d'action et format de rejeu v1 (LOT-ANNEXE-07),
  `TrainingStatsRecorder` (LOT-ANNEXE-09), évolutionniste (LOT-ANNEXE-10) — non renumérotées ici.

## Découpage

> État : ✅ fait · 🔄 en cours · ⬜ non commencé.

| Tâche | Intitulé | Emplacement | État |
|-------|----------|-------------|:----:|
| [TACHE-01](tache-01-decision-ppo-dqn.md) | Décision de cadrage PPO vs DQN et implémentation de l'algorithme retenu | `Source/AiSolver/Training/Advanced` | ✅ |
| [TACHE-02](tache-02-integration-harnais.md) | Intégration au harnais d'entraînement existant | `Source/AiSolver/Training/Advanced` | ✅ |
| [TACHE-03](tache-03-comparaison-chiffree.md) | Comparaison chiffrée contre la génération 2 et le reste de la génération 3 | `Source/AiSolver/Training/Advanced` | ✅ |

## Décision retenue et résultat de la comparaison

**Décision (TACHE-01) : DQN**, plutôt que PPO. LOT-ANNEXE-13 (acteur-critique) a produit un résultat
non concluant sur l'instabilité : sur le niveau de contrôle trivial (résolu en moins d'un épisode),
REINFORCE montrait déjà une variance de fin de run plus faible que l'acteur-critique — le niveau était
trop simple pour révéler un bénéfice de clipping PPO, qui cible précisément une instabilité non
démontrée ici. L'espace d'action discret à 48 actions (LOT-ANNEXE-07) se prête en revanche bien à une
approche par valeur d'action : DQN cible l'efficacité d'échantillonnage (réutilisation des transitions
via `ReplayBuffer`), un facteur pertinent indépendamment du constat de stabilité.

**Comparaison chiffrée (TACHE-03)** — même niveau de contrôle (trivial, utilisé par LOT-ANNEXE-12/13),
même budget équivalent de 120 épisodes de jeu pour les quatre approches (conversion générations ×
individus → épisodes pour l'évolutionniste), 3 essais par approche, seuil de récompense = 5.0 :

| Approche | Essais atteignant le seuil | Épisodes moyens jusqu'au seuil | Écart-type de récompense en fin de run |
|---|---|---|---|
| Évolutionniste (LOT-ANNEXE-10) | 3/3 | 0.000 | 0.00475 |
| REINFORCE (LOT-ANNEXE-12) | 3/3 | 0.667 | 0.08844 |
| Acteur-critique (LOT-ANNEXE-13) | 3/3 | 0.333 | 47.60 |
| **DQN (LOT-ANNEXE-14)** | 3/3 | 0.333 | **0.02409** |

Les quatre approches atteignent systématiquement le seuil sur ce niveau trivial. DQN produit la
variance de fin de run la plus faible des quatre — y compris plus faible que l'acteur-critique, dont
la variance élevée ici confirme, a posteriori, que ce niveau de contrôle ne sollicite pas fortement
la stabilité de politique (cohérent avec le constat déjà fait en LOT-ANNEXE-13). Mesure honnête et non
orientée : ce résultat porte sur les niveaux de contrôle retenus par les générations 2 et 3, pas sur
une généralisation — celle-ci est explicitement le rôle mesuré séparément par la génération 4
(LOT-ANNEXE-15/16). `EX-IA-015` est couverte par cette mesure comparative.

## Critères d'acceptation du lot
1. La décision PPO vs DQN est **documentée explicitement** (dans TACHE-01, une fois tranchée), avec
   les résultats de LOT-ANNEXE-13 cités à l'appui du choix.
2. L'algorithme retenu est entraîné avec succès sur le niveau de contrôle utilisé par
   LOT-ANNEXE-12/13 (progression de récompense observable dans les CSV).
3. Si PPO est retenu : le ratio de probabilité clippé reste dans les bornes configurées à chaque
   époque d'optimisation (vérifié par test), et plusieurs époques d'optimisation s'exécutent
   effectivement sur un même batch de trajectoires. Si DQN est retenu : le `ReplayBuffer` est
   effectivement échantillonné pour les mises à jour (pas uniquement la transition la plus récente),
   et le réseau cible est mis à jour à la période configurée, pas à chaque pas.
4. Une comparaison chiffrée, sur un même jeu de niveaux de contrôle, place l'algorithme retenu, la
   génération 2 (évolutionniste) et le reste de la génération 3 côte à côte (vitesse de convergence
   et/ou robustesse), consignée dans la documentation du lot.
5. Aucune nouvelle dépendance tierce n'a été ajoutée (`External/CMakeLists.txt` inchangé).
6. Logique nouvelle **couverte par des tests** (`ctest` vert), déterministe, sans GPU. Build
   `/W4 /WX` sans avertissement, Doxygen et lint des exigences verts.

## Dépendances
S'appuie directement sur les résultats mesurés de [LOT-ANNEXE-13](@ref lot-annexe-13) pour trancher
son choix d'algorithme. Bâtit sur [LOT-ANNEXE-05](@ref lot-annexe-05) (environnement headless),
[LOT-ANNEXE-07](@ref lot-annexe-07) (espace d'action discret, en particulier si DQN est retenu) et
[LOT-ANNEXE-09](@ref lot-annexe-09) (`TrainingStatsRecorder`). Compare ses résultats à ceux de
[LOT-ANNEXE-10](@ref lot-annexe-10) (évolutionniste), [LOT-ANNEXE-12](@ref lot-annexe-12)
(REINFORCE) et [LOT-ANNEXE-13](@ref lot-annexe-13) (acteur-critique). Dernier lot de la génération 3 ;
la génération 4 ([LOT-ANNEXE-15](@ref lot-annexe-15)/[LOT-ANNEXE-16](@ref lot-annexe-16)) évalue la
robustesse des modèles produits par l'ensemble de la génération, y compris celui-ci.

## Navigation des tâches
- @subpage lot-annexe-14-tache-01-decision-ppo-dqn
- @subpage lot-annexe-14-tache-02-integration-harnais
- @subpage lot-annexe-14-tache-03-comparaison-chiffree
